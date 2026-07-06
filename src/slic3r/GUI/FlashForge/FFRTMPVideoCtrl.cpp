#include "FFRTMPVideoCtrl.h"

#include <wx/dcbuffer.h>

#include <boost/log/trivial.hpp>
#include <thread>
#include <chrono>
#include <cstring>
#include <algorithm>

// Debug output helper — writes to both Boost log and Visual Studio / DebugView on Windows
static void ffrtmp_log(const std::string &msg)
{
    BOOST_LOG_TRIVIAL(info) << "[FFRTMP] " << msg;
#ifdef _WIN32
    OutputDebugStringA(("[FFRTMP] " + msg + "\n").c_str());
#endif
}
static void ffrtmp_err(const std::string &msg)
{
    BOOST_LOG_TRIVIAL(error) << "[FFRTMP] ERROR: " << msg;
#ifdef _WIN32
    OutputDebugStringA(("[FFRTMP] ERROR: " + msg + "\n").c_str());
#endif
}

#ifdef FFRTMP_USE_FFMPEG
extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}
#endif

#include "slic3r/GUI/GUI_App.hpp"
#include "slic3r/GUI/MainFrame.hpp"
#include "slic3r/GUI/I18N.hpp"

namespace Slic3r { namespace GUI {

FFRTMPVideoCtrl::FFRTMPVideoCtrl(wxWindow *parent)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE)
{
    SetBackgroundColour(*wxBLACK);
    SetMinSize(wxSize(FromDIP(320), FromDIP(180)));

    // 记录内联时的父窗口，弹窗（全屏预览）关闭后需归还，否则控件会被孤立、内联区域永久空白
    m_inline_parent = parent;

    m_offline_bitmap.Create(FromDIP(640), FromDIP(360), 24);
    {
        wxMemoryDC dc(m_offline_bitmap);
        dc.SetBackground(*wxBLACK_BRUSH);
        dc.Clear();
    }

    Bind(wxEVT_PAINT, &FFRTMPVideoCtrl::OnPaint, this);
    Bind(wxEVT_SIZE,  &FFRTMPVideoCtrl::OnSize,  this);
}

FFRTMPVideoCtrl::~FFRTMPVideoCtrl()
{
    StopStream();

    if (m_popup_dlg) {
        m_popup_dlg->Destroy();
        m_popup_dlg = nullptr;
    }
}

void FFRTMPVideoCtrl::StartStream(const std::string &url)
{
    // Unconditional debug output — always fires regardless of FFRTMP_USE_FFMPEG
    ffrtmp_log("StartStream called, url='" + url + "'");

#ifdef FFRTMP_USE_FFMPEG
    if (url.empty()) {
        ffrtmp_log("url is empty, stopping stream");
        StopStream();
        return;
    }

    if (m_running && m_url == url) {
        ffrtmp_log("Already streaming this url, ignoring duplicate StartStream");
        return;
    }

    // 是否为同一路流的重启（用于跳过 2s 预热等待，实现快速恢复）
    const bool same_url = (!m_url.empty() && m_url == url);

    StopStream();

    m_url = url;
    m_running = true;
    m_reconnect_attempts = 0;
    m_skip_initial_delay = same_url;

    ffrtmp_log("Starting stream: " + m_url);

    // 视频为“弹窗专属”：解码在后台持续进行，但内联控件保持隐藏，
    // 不在设备状态面板上显示画面。只有 ShowFullScreenPopup 才会显示控件。
    // 后台保持解码可让再次打开弹窗时立即出画面，避免重连等待。

    m_thread = std::make_unique<std::thread>(&FFRTMPVideoCtrl::DecoderThreadFunc, this);
#else
    (void)url;
    ffrtmp_err("FFmpeg not available (FFRTMP_USE_FFMPEG not defined)");
#endif
}

void FFRTMPVideoCtrl::StopStream()
{
    ffrtmp_log("StopStream called");
    m_running = false;

    if (m_thread && m_thread->joinable()) {
        m_thread->join();
    }
    m_thread.reset();

#ifdef FFRTMP_USE_FFMPEG
    CloseStream();
#endif

    {
        wxCriticalSectionLocker lock(m_frame_cs);
        m_frame_ready = false;
        m_rgb_buffer.clear();
        m_rgb_bitmap = wxBitmap();
    }
    CallAfter([this]() { Hide(); Refresh(); });
}

bool FFRTMPVideoCtrl::IsStreaming() const { return m_running; }

void FFRTMPVideoCtrl::ShowFullScreenPopup()
{
    if (m_popup_dlg) return;

    // 保存内联时的固定尺寸，弹窗关闭后需恢复（否则会残留 640x480 的约束把内联区域撑大）
    m_inline_size = GetSize();

    // 关键：把本控件从内联 sizer 中分离。
    // 否则设备页每次遥测刷新触发的 SingleDeviceState::Layout() 会级联到 m_monitoring_sizer，
    // 而该 sizer 仍持有指向本控件的 item，会对已经 Reparent 到弹窗的控件调用 SetSize(内联尺寸)，
    // 把弹窗里 640x480 的画面强行缩回内联大小 —— 这就是“画面时大时小”的根因。
    if (wxSizer *s = GetContainingSizer()) s->Detach(this);

    wxSize videoSize(FromDIP(640), FromDIP(480));
    m_popup_dlg = new wxDialog(wxGetApp().mainframe, wxID_ANY, "");
    m_popup_dlg->SetClientSize(videoSize);
    m_popup_dlg->CenterOnParent();

    Reparent(m_popup_dlg);
    setSize(videoSize);  // min==max==640x480，弹窗内尺寸固定
    Show();              // 控件此前可能处于 Hidden 状态，必须显式显示，否则弹窗内一片黑
    Refresh();

    m_popup_dlg->Bind(wxEVT_CLOSE_WINDOW, [this](wxCloseEvent &event) {
        // 弹窗专属：关闭后控件必须先从即将销毁的弹窗中 Reparent 出去（否则会随弹窗一起销毁），
        // 归还给内联父窗口后保持 Hidden —— 设备状态面板上不显示任何视频画面。
        // 解码线程不停止，保证再次打开弹窗时能立即出画面。
        Reparent(m_inline_parent ? m_inline_parent : wxGetApp().mainframe);
        setSize(m_inline_size);  // 复位为内联占位尺寸（隐藏状态下不占布局空间）
        Hide();
        if (m_inline_parent && m_inline_parent->GetSizer()) {
            m_inline_parent->GetSizer()->Add(this, 0, wxALL, 0);
            m_inline_parent->Layout();
        }
        m_popup_dlg->Destroy();
        m_popup_dlg = nullptr;
    });

    m_popup_dlg->Show();
}

#ifdef FFRTMP_USE_FFMPEG

// ============================================================================
//  FFmpeg-specific helpers
// ============================================================================

#define FMT_CTX(p)   ((AVFormatContext *)(p))
#define CDC_CTX(p)   ((AVCodecContext *)(p))
#define SWS_CTX(p)   ((SwsContext *)(p))
#define FRM(p)       ((AVFrame *)(p))
#define PCKT(p)      ((AVPacket *)(p))

// Helper to convert FFmpeg error code to string
static std::string av_err_str(int err)
{
    char buf[256];
    av_strerror(err, buf, sizeof(buf));
    return std::string(buf);
}

void FFRTMPVideoCtrl::DecoderThreadFunc()
{
    ffrtmp_log("Decode thread started");

    // 首次连接前等待，给摄像头时间启动推流（RTMP 直播流需要几秒准备）。
    // 同一路流重启（m_skip_initial_delay）时跳过，避免每次重连都白等 2 秒。
    if (m_reconnect_attempts == 0 && m_initial_delay_ms > 0 && !m_skip_initial_delay) {
        ffrtmp_log("Waiting " + std::to_string(m_initial_delay_ms) + "ms for camera stream to become ready...");
        std::this_thread::sleep_for(std::chrono::milliseconds(m_initial_delay_ms));
    }
    m_skip_initial_delay = false;

    while (m_running) {
        ffrtmp_log("Attempting OpenStream (attempt " + std::to_string(m_reconnect_attempts + 1)
                   + ") for " + m_url);

        if (OpenStream(m_url)) {
            m_reconnect_attempts = 0;
            int frame_count = 0;

            while (m_running) {
                // 不做人工帧率限制：av_read_frame 会阻塞等待网络数据，天然按源帧率节流。
                // 若加 30fps 睡眠上限，连接时服务端一次性下发的 GOP 缓存无法被快速消费，
                // 会造成延迟不断累积且永不恢复，这是直播画面高延迟的主因。
                if (!ReadAndDecodeOneFrame()) {
                    ffrtmp_log("ReadAndDecodeOneFrame returned false, frames decoded="
                               + std::to_string(frame_count));
                    break;
                }
                ++frame_count;
                if (frame_count == 1) {
                    ffrtmp_log("First frame decoded successfully!");
                }
            }

            CloseStream();
        } else {
            ffrtmp_err("OpenStream failed for " + m_url);
        }

        if (!m_running) break;

        m_reconnect_attempts++;
        if (m_reconnect_attempts > m_max_reconnect_attempts) {
            ffrtmp_err("Max reconnect reached (" + std::to_string(m_max_reconnect_attempts) + ")");
            m_running = false;
            break;
        }

        ffrtmp_log("Reconnecting in " + std::to_string(m_reconnect_delay_ms) + "ms (attempt "
                   + std::to_string(m_reconnect_attempts) + "/"
                   + std::to_string(m_max_reconnect_attempts) + ")");

        for (int i = 0; i < m_reconnect_delay_ms / 100 && m_running; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    ffrtmp_log("Decode thread exiting");

    CallAfter([this]() {
        wxCriticalSectionLocker lock(m_frame_cs);
        m_frame_ready = false;
        m_rgb_buffer.clear();
        m_rgb_bitmap = wxBitmap();
        Refresh();
    });
}

bool FFRTMPVideoCtrl::OpenStream(const std::string &url)
{
    AVFormatContext *fmt_ctx = nullptr;
    AVDictionary    *opts    = nullptr;
    int              ret;

    // RTMP live stream options — tuned for low latency
    av_dict_set(&opts, "rtmp_live",       "live",   0);
    av_dict_set(&opts, "rtmp_listen",     "0",      0);  // 显式关闭 listen 模式（timeout 选项会导致 listen 被强制开启，故移除 timeout）
    av_dict_set(&opts, "rtmp_buffer",     "100",    0);  // 客户端向 RTMP 服务端请求的缓冲时长，FFmpeg 默认 3000ms 是直播高延迟的主因，改小以降低延迟
    av_dict_set(&opts, "max_delay",       "100000", 0);  // max demux-decode delay (us)
    av_dict_set(&opts, "probesize",       "32768",  0);  // smaller probe for faster start
    av_dict_set(&opts, "fflags",          "nobuffer", 0); // disable format-level buffering
    av_dict_set(&opts, "analyzeduration", "1000000", 0);  // 1s analysis (default 5s)
    av_dict_set(&opts, "flush_packets",   "1",       0);  // flush packets immediately

    ffrtmp_log("avformat_open_input...");

    ret = avformat_open_input(&fmt_ctx, url.c_str(), nullptr, &opts);
    av_dict_free(&opts);

    if (ret < 0) {
        ffrtmp_err("avformat_open_input failed: " + av_err_str(ret)
                   + " (code=" + std::to_string(ret) + ")");
        return false;
    }

    ffrtmp_log("avformat_find_stream_info...");

    ret = avformat_find_stream_info(fmt_ctx, nullptr);
    if (ret < 0) {
        ffrtmp_err("avformat_find_stream_info failed: " + av_err_str(ret));
        avformat_close_input(&fmt_ctx);
        return false;
    }

    ffrtmp_log("Stream info: nb_streams=" + std::to_string(fmt_ctx->nb_streams)
               + " format=" + (fmt_ctx->iformat ? fmt_ctx->iformat->name : "unknown")
               + " duration=" + std::to_string(fmt_ctx->duration));

    int video_idx = -1;
    AVCodecParameters *codecpar = nullptr;
    for (unsigned i = 0; i < fmt_ctx->nb_streams; ++i) {
        AVCodecParameters *cp = fmt_ctx->streams[i]->codecpar;
        const char *type_str =
            cp->codec_type == AVMEDIA_TYPE_VIDEO   ? "VIDEO"  :
            cp->codec_type == AVMEDIA_TYPE_AUDIO   ? "AUDIO"  :
            cp->codec_type == AVMEDIA_TYPE_SUBTITLE ? "SUBTITLE" : "OTHER";
        ffrtmp_log("stream[" + std::to_string(i) + "] type=" + type_str
                   + " codec_id=" + std::to_string(cp->codec_id));
        if (cp->codec_type == AVMEDIA_TYPE_VIDEO && video_idx < 0) {
            video_idx = i; codecpar = cp;
        }
    }

    if (video_idx < 0 || !codecpar) {
        ffrtmp_err("No video stream found in " + std::to_string(fmt_ctx->nb_streams) + " streams");
        avformat_close_input(&fmt_ctx);
        return false;
    }

    const AVCodec *codec = avcodec_find_decoder(codecpar->codec_id);
    if (!codec) {
        ffrtmp_err("avcodec_find_decoder failed for codec_id=" + std::to_string(codecpar->codec_id));
        avformat_close_input(&fmt_ctx);
        return false;
    }

    AVCodecContext *codec_ctx = avcodec_alloc_context3(codec);
    if (!codec_ctx) {
        ffrtmp_err("avcodec_alloc_context3 failed");
        avformat_close_input(&fmt_ctx);
        return false;
    }

    avcodec_parameters_to_context(codec_ctx, codecpar);
    codec_ctx->thread_count = 2;

    ret = avcodec_open2(codec_ctx, codec, nullptr);
    if (ret < 0) {
        ffrtmp_err("avcodec_open2 failed: " + av_err_str(ret) + " codec=" + codec->name);
        avcodec_free_context(&codec_ctx);
        avformat_close_input(&fmt_ctx);
        return false;
    }

    AVFrame *frame   = av_frame_alloc();
    AVPacket *packet = av_packet_alloc();
    if (!frame || !packet) {
        ffrtmp_err("av_frame/packet_alloc failed");
        av_frame_free(&frame); av_packet_free(&packet);
        avcodec_free_context(&codec_ctx); avformat_close_input(&fmt_ctx);
        return false;
    }

    m_video_width  = codec_ctx->width;
    m_video_height = codec_ctx->height;
    m_format_ctx   = fmt_ctx;
    m_codec_ctx    = codec_ctx;
    m_sws_ctx      = nullptr;
    m_frame        = frame;
    m_packet       = packet;
    m_video_stream_idx = video_idx;

    ffrtmp_log("Stream opened successfully — "
               + std::to_string(m_video_width) + "x" + std::to_string(m_video_height)
               + " codec=" + codec->name);
    return true;
}

void FFRTMPVideoCtrl::CloseStream()
{
    AVFormatContext *fmt_ctx  = FMT_CTX(m_format_ctx);
    AVCodecContext  *codec_ctx = CDC_CTX(m_codec_ctx);
    SwsContext      *sws       = SWS_CTX(m_sws_ctx);
    AVFrame         *frame     = FRM(m_frame);
    AVPacket        *packet    = PCKT(m_packet);

    if (sws)       { sws_freeContext(sws);         m_sws_ctx = nullptr; }
    if (frame)     { av_frame_free(&frame);        m_frame   = nullptr; }
    if (packet)    { av_packet_free(&packet);      m_packet  = nullptr; }
    if (codec_ctx) { avcodec_free_context(&codec_ctx); m_codec_ctx = nullptr; }
    if (fmt_ctx)   { avformat_close_input(&fmt_ctx);   m_format_ctx = nullptr; }

    m_video_stream_idx = -1;
}

bool FFRTMPVideoCtrl::ReadAndDecodeOneFrame()
{
    AVFormatContext *fmt_ctx  = FMT_CTX(m_format_ctx);
    AVCodecContext  *codec_ctx = CDC_CTX(m_codec_ctx);
    AVFrame         *frame    = FRM(m_frame);
    AVPacket        *packet   = PCKT(m_packet);

    int ret = av_read_frame(fmt_ctx, packet);
    if (ret < 0) {
        if (ret != AVERROR_EOF) {
            ffrtmp_err("av_read_frame failed: " + av_err_str(ret));
        }
        return false;
    }

    if (packet->stream_index != m_video_stream_idx) {
        av_packet_unref(packet); return true;
    }

    ret = avcodec_send_packet(codec_ctx, packet);
    av_packet_unref(packet);
    if (ret < 0) { return true; }

    while (ret >= 0) {
        ret = avcodec_receive_frame(codec_ctx, frame);
        if (ret == AVERROR(EAGAIN)) break;
        if (ret < 0) return true;

        // Lazy-init or re-create scaler when resolution changes
        SwsContext *sws = SWS_CTX(m_sws_ctx);
        if (!sws || codec_ctx->width != m_video_width || codec_ctx->height != m_video_height) {
            if (sws) { sws_freeContext(sws); m_sws_ctx = nullptr; }
            sws = sws_getContext(
                codec_ctx->width, codec_ctx->height, codec_ctx->pix_fmt,
                codec_ctx->width, codec_ctx->height, AV_PIX_FMT_RGB24,
                SWS_BILINEAR, nullptr, nullptr, nullptr);
            m_sws_ctx = sws;
            m_video_width  = codec_ctx->width;
            m_video_height = codec_ctx->height;
            if (!sws) { av_frame_unref(frame); return true; }
            ffrtmp_log("Scaler (re)created: " + std::to_string(m_video_width) + "x" + std::to_string(m_video_height));
        }

        // Scale into raw RGB buffer (thread-safe, no GDI objects in decoder thread)
        const int fw = m_video_width;
        const int fh = m_video_height;
        std::vector<uint8_t> buffer((size_t)fw * fh * 3);
        uint8_t *dst[1]     = { buffer.data() };
        int      dst_stride = fw * 3;

        sws_scale(sws, frame->data, frame->linesize, 0, codec_ctx->height,
                  dst, &dst_stride);

        {
            // buffer 与其尺寸必须一起在锁内更新，主线程读取时才能保证一致，
            // 否则渲染时用到不匹配的宽高会导致画面尺寸错乱（突然变小）甚至内存越界。
            wxCriticalSectionLocker lock(m_frame_cs);
            m_rgb_buffer  = std::move(buffer);
            m_buf_width   = fw;
            m_buf_height  = fh;
            m_frame_ready = true;
        }

        CallAfter(&FFRTMPVideoCtrl::OnFrameReady);
        av_frame_unref(frame);
    }
    return true;
}

#else  // !FFRTMP_USE_FFMPEG

// Stubs when FFmpeg is not available
void   FFRTMPVideoCtrl::DecoderThreadFunc()                           {}
bool   FFRTMPVideoCtrl::OpenStream(const std::string &)              { return false; }
void   FFRTMPVideoCtrl::CloseStream()                                {}
bool   FFRTMPVideoCtrl::ReadAndDecodeOneFrame()                      { return false; }

#endif // FFRTMP_USE_FFMPEG

// ============================================================================
//  PrinterCameraPanel compatibility API (always compiled)
// ============================================================================

void FFRTMPVideoCtrl::setSize(wxSize size)
{
    SetSize(size);
    SetMinSize(size);
    SetMaxSize(size);
}

void FFRTMPVideoCtrl::setDisplayMode(DisplayMode mode)
{
    if (m_display_mode == mode) return;
    m_display_mode = mode;
    CallAfter([this]() { Refresh(); });
}

void FFRTMPVideoCtrl::setCurComId(com_id_t comId)
{
    m_curComId = comId;
}

void FFRTMPVideoCtrl::setStreamUrl(const std::string &streamUrl)
{
    StartStream(streamUrl);
}

void FFRTMPVideoCtrl::setOffline()
{
    ffrtmp_log("setOffline called");
    StopStream();
    CallAfter([this]() {
        wxCriticalSectionLocker lock(m_frame_cs);
        m_frame_ready = false;
        m_rgb_buffer.clear();
        m_rgb_bitmap = wxBitmap();
        Hide();   // 弹窗专属：离线时不在设备面板上显示任何占位画面
        Refresh();
    });
}

void FFRTMPVideoCtrl::showPopup()
{
    ShowFullScreenPopup();
}

// ============================================================================
//  Main-Thread Callbacks (always compiled)
// ============================================================================

void FFRTMPVideoCtrl::OnFrameReady()
{
    // Called on main thread via CallAfter — safe to create GDI objects here
    wxCriticalSectionLocker lock(m_frame_cs);
    if (m_frame_ready && !m_rgb_buffer.empty() && m_buf_width > 0 && m_buf_height > 0
        && m_rgb_buffer.size() == (size_t)m_buf_width * m_buf_height * 3) {
        wxImage img(m_buf_width, m_buf_height, false);
        memcpy(img.GetData(), m_rgb_buffer.data(), m_rgb_buffer.size());
        m_rgb_bitmap = wxBitmap(img);
    }
    Refresh(false);
}

void FFRTMPVideoCtrl::OnPaint(wxPaintEvent & /*event*/)
{
    wxAutoBufferedPaintDC dc(this);
    dc.SetBackground(*wxBLACK_BRUSH);
    dc.Clear();

    wxCriticalSectionLocker lock(m_frame_cs);
    wxSize client = GetClientSize();

    if (m_frame_ready && m_rgb_bitmap.IsOk() && client.x > 0 && client.y > 0) {
        wxMemoryDC memDC;
        memDC.SelectObject(m_rgb_bitmap);

        // 以位图自身尺寸为准，而不是解码线程异步写入的 m_video_width/height，
        // 避免两者不一致导致画面尺寸计算错误（突然变小）。
        const int vw = m_rgb_bitmap.GetWidth();
        const int vh = m_rgb_bitmap.GetHeight();

        if (m_display_mode == DisplayMode::Cover) {
            // ---- 铺满(裁边) / cover ----
            // 等比缩放铺满整个控件，通过裁剪源图保持画面比例，超出部分被裁掉。
            const double client_aspect = (double)client.x / (double)client.y;
            const double video_aspect  = (double)vw / (double)vh;

            int src_x, src_y, src_w, src_h;
            if (video_aspect > client_aspect) {
                // 视频比控件更宽 —— 裁掉左右
                src_h = vh;
                src_w = std::max(1, (int)(vh * client_aspect + 0.5));
                src_x = (vw - src_w) / 2;
                src_y = 0;
            } else {
                // 视频比控件更高（或等宽）—— 裁掉上下
                src_w = vw;
                src_h = std::max(1, (int)(vw / client_aspect + 0.5));
                src_x = 0;
                src_y = (vh - src_h) / 2;
            }

            dc.StretchBlit(0, 0, client.x, client.y,
                           &memDC, src_x, src_y, src_w, src_h);
        } else {
            // ---- 完整显示(留黑边) / fit ----
            double scale = std::min((double)client.x / vw,
                                    (double)client.y / vh);
            int w = std::max(1, (int)(vw * scale));
            int h = std::max(1, (int)(vh * scale));
            int x = (client.x - w) / 2;
            int y = (client.y - h) / 2;
            dc.StretchBlit(x, y, w, h, &memDC, 0, 0, vw, vh);
        }

        memDC.SelectObject(wxNullBitmap);
    } else if (m_offline_bitmap.IsOk() && client.x > 0 && client.y > 0) {
        int w = m_offline_bitmap.GetWidth();
        int h = m_offline_bitmap.GetHeight();
        double scale = std::min((double)client.x / w, (double)client.y / h);
        int sw = std::max(1, (int)(w * scale));
        int sh = std::max(1, (int)(h * scale));
        wxMemoryDC memDC;
        memDC.SelectObject(m_offline_bitmap);
        dc.StretchBlit((client.x - sw) / 2, (client.y - sh) / 2, sw, sh,
                       &memDC, 0, 0, w, h);
        memDC.SelectObject(wxNullBitmap);
    }
}

void FFRTMPVideoCtrl::OnSize(wxSizeEvent & /*event*/)
{
    Refresh();
}

}} // namespace Slic3r::GUI
