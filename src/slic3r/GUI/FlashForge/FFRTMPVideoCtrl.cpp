#include "FFRTMPVideoCtrl.h"

#include <wx/dcbuffer.h>

#include <boost/log/trivial.hpp>
#include <thread>
#include <chrono>
#include <cctype>
#include <algorithm>
#include <cstring>

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
#include "slic3r/GUI/FlashForge/MultiComMgr.hpp"
#include "slic3r/GUI/FlashForge/ComCommand.hpp"

namespace Slic3r { namespace GUI {

FFRTMPVideoCtrl::FFRTMPVideoCtrl(wxWindow *parent)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE)
{
    SetBackgroundColour(*wxBLACK);
    SetMinSize(wxSize(FromDIP(621), FromDIP(466)));

    // 记录内联时的父窗口，弹窗（全屏预览）关闭后需归还，否则控件会被孤立、内联区域永久空白
    m_inline_parent = parent;

    m_offline_bitmap = ScalableBitmap(this, "camera_offline", 1500).bmp();
    m_playIconMap["offline"] = ScalableBitmap(this, "play_offline", 29);
    m_playIconMap["play"] = ScalableBitmap(this, "play_normal", 29);
    m_playIconMap["pause"] = ScalableBitmap(this, "play_pause", 29);

    Bind(wxEVT_PAINT, &FFRTMPVideoCtrl::OnPaint, this);
    Bind(wxEVT_SIZE,  &FFRTMPVideoCtrl::OnSize,  this);
    Bind(wxEVT_LEFT_DOWN, &FFRTMPVideoCtrl::OnLeftDown, this);  // 左下角播放/暂停按钮

    // 保活定时器：播放期间周期性重发 camera "open"，维持打印机推流。
    m_keepalive_timer.Bind(wxEVT_TIMER, [this](wxTimerEvent &) { sendCameraOpen(); });
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

    if (m_paused && m_url == url) {
        ffrtmp_log("paused, ignoring same-url StartStream");
        return;
    }

    // 暂停只是“显示冻结”，与拉流解耦：即使处于暂停显示态，解码/拉流仍照常进行/切换，
    // 因此这里不拦截，正常起流（切设备时会切到新流；用户点播放即恢复显示最新帧）。
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
    m_offline_shown = false;
    m_paused = false;                        // 新开流清除暂停态
    m_skip_initial_delay = same_url;
    setPlayState(PlayState::Initializing);   // 右下角状态：正在初始化

    ffrtmp_log("Starting stream: " + m_url);

    // 内联显示：摄像头控件常驻设备状态面板。起流时确保控件可见（此前可能被
    // setOffline/StopStream 隐藏过），并触发一次布局。弹窗模式下由弹窗自行管理显示。
    CallAfter([this]() {
        if (!m_popup_dlg && !IsShown()) {
            Show();
            if (wxSizer *s = GetContainingSizer()) s->Layout();
        }
        Refresh();
    });

    m_thread = std::make_unique<std::thread>(&FFRTMPVideoCtrl::DecoderThreadFunc, this);

    // 立即通知设备开流，并启动定时保活 —— 让打印机（经云端）开始并持续推流。
    // 这条 open 指令走 WAN 云链路，是画面真正出现的前提；仅拉流而不开流拿到的是空地址。
    sendCameraOpen();
    m_keepalive_timer.Start(m_keepalive_interval_ms);
#else
    (void)url;
    ffrtmp_err("FFmpeg not available (FFRTMP_USE_FFMPEG not defined)");
#endif
}

void FFRTMPVideoCtrl::StopStream()
{
    ffrtmp_log("StopStream called");
    m_keepalive_timer.Stop();  // 停止推流保活（wxTimer 仅可在主线程操作）
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
    // 内联控件常驻显示：停止拉流后不隐藏，保持可见并露出黑底占位图 + 状态条
    // （例如“打印机断开连接”）。弹窗模式同样保持显示。
    CallAfter([this]() { Refresh(); });
}

bool FFRTMPVideoCtrl::IsStreaming() const { return m_running; }

void FFRTMPVideoCtrl::ShowFullScreenPopup()
{
    if (m_popup_dlg) return;

    // 打开摄像头窗口：正常显示实时画面（与 Orca 一致）。若之前处于显示暂停态，则恢复显示。
    if (m_paused) {
        resumeStream();
    }

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
            m_offline_shown = false;
            setPlayState(PlayState::Loading);   // 已连接，等待首帧：视频加载中
            int frame_count = 0;
            // 最近一次“有数据”的时间点，用于 HLS 到达直播边缘时的宽限判定。
            auto last_progress = std::chrono::steady_clock::now();
            // HLS 直播边缘 EOF 宽限期：期间在同一连接上原地重试读取，
            // 等待设备（经保活）产出新分片，避免为了一次边缘 EOF 就整路重连。
            const auto hls_eof_grace = std::chrono::seconds(15);

            while (m_running) {
                // 不做人工帧率限制：av_read_frame 会阻塞等待网络数据，天然按源帧率节流。
                // 若加 30fps 睡眠上限，连接时服务端一次性下发的 GOP 缓存无法被快速消费，
                // 会造成延迟不断累积且永不恢复，这是直播画面高延迟的主因。
                ReadStatus st = ReadAndDecodeOneFrame();
                if (st == ReadStatus::Frame) {
                    ++frame_count;
                    last_progress = std::chrono::steady_clock::now();
                    // 暂停是“显示暂停”，解码始终在跑；这里状态置为 Playing，
                    // 若处于暂停显示态，OnFrameReady 会跳过刷新（画面冻结），
                    // 但状态文字由 pauseStream 保持为“视频已暂停”。
                    if (!m_paused) {
                        setPlayState(PlayState::Playing);   // 视频播放中（内部去重，不会频繁刷新）
                    }
                    if (frame_count == 1) {
                        ffrtmp_log("First frame decoded successfully!");
                    }
                    continue;
                }
                if (st == ReadStatus::Eof && m_is_hls) {
                    // 直播到达边缘：不断流。短暂等待后在同一连接上继续读，
                    // 等待新分片到来；仅当超过宽限期仍无数据才重开连接。
                    if (std::chrono::steady_clock::now() - last_progress > hls_eof_grace) {
                        ffrtmp_log("HLS EOF grace exceeded, reopening stream (frames="
                                   + std::to_string(frame_count) + ")");
                        break;
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(300));
                    continue;
                }
                // 非 HLS 的 EOF 视为真正断流；或读取错误 —— 退出内层循环走重连。
                ffrtmp_log("Read ended (status="
                           + std::string(st == ReadStatus::Eof ? "eof" : "error")
                           + "), frames decoded=" + std::to_string(frame_count));
                break;
            }

            CloseStream();
        } else {
            ffrtmp_err("OpenStream failed for " + m_url);
        }

        if (!m_running) break;

        m_reconnect_attempts++;
        setPlayState(PlayState::Loading);   // 重连/缓冲中：视频加载中

        // 直播源不永久放弃：达到阈值时切到离线占位图（黑底，绝不留白），
        // 之后仍以退避间隔持续重连，直到 StopStream/setOffline 主动结束。
        if (m_reconnect_attempts >= m_max_reconnect_attempts && !m_offline_shown) {
            m_offline_shown = true;
            ffrtmp_log("Reconnect threshold reached, showing offline placeholder but keep retrying");
            CallAfter([this]() {
                wxCriticalSectionLocker lock(m_frame_cs);
                m_frame_ready = false;
                m_rgb_buffer.clear();
                m_rgb_bitmap = wxBitmap();
                Refresh();  // OnPaint 会绘制离线黑底占位图，而非露出弹窗白底
            });
        }

        // 指数退避，封顶 m_reconnect_delay_max_ms，避免网络长时间不可用时高频重连。
        int delay = std::min(m_reconnect_delay_ms * m_reconnect_attempts, m_reconnect_delay_max_ms);
        ffrtmp_log("Reconnecting in " + std::to_string(delay) + "ms (attempt "
                   + std::to_string(m_reconnect_attempts) + ")");

        for (int i = 0; i < delay / 100 && m_running; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    ffrtmp_log("Decode thread exiting");

    CallAfter([this]() {
        // 用户暂停时保留最后一帧冻结显示，不清空画面。
        if (m_paused) { Refresh(); return; }
        wxCriticalSectionLocker lock(m_frame_cs);
        m_frame_ready = false;
        m_rgb_buffer.clear();
        m_rgb_bitmap = wxBitmap();
        Refresh();
    });
}

int FFRTMPVideoCtrl::interruptCb(void *opaque)
{
    FFRTMPVideoCtrl *self = static_cast<FFRTMPVideoCtrl *>(opaque);
    // 返回非 0 会让 FFmpeg 中止当前阻塞的网络调用。
    return (self && !self->m_running) ? 1 : 0;
}

bool FFRTMPVideoCtrl::OpenStream(const std::string &url)
{
    AVFormatContext *fmt_ctx = avformat_alloc_context();
    AVDictionary    *opts    = nullptr;
    int              ret;

    if (!fmt_ctx) {
        ffrtmp_err("avformat_alloc_context failed");
        return false;
    }
    // 停止/暂停时（m_running=false）立即中止阻塞的网络 I/O，避免 join 卡住 UI。
    // 该回调对 avformat_open_input 与后续 av_read_frame 均生效。
    fmt_ctx->interrupt_callback.callback = &FFRTMPVideoCtrl::interruptCb;
    fmt_ctx->interrupt_callback.opaque   = this;

    // 按 URL 协议/扩展名选择解复用参数，实现多协议支持：
    //   rtmp(e/s/t):// —— RTMP 直播流
    //   *.m3u8         —— HLS 直播/点播（Apple HTTP Live Streaming）
    //   其它 http(s)   —— HTTP-FLV 等
    // 之前无条件设置 RTMP 专属选项会让 HLS(.m3u8) 无法正常播放，这里分协议配置。
    std::string lurl = url;
    std::transform(lurl.begin(), lurl.end(), lurl.begin(),
                   [](unsigned char c) { return (char)std::tolower(c); });
    const bool is_rtmp = lurl.rfind("rtmp://", 0) == 0 || lurl.rfind("rtmpe://", 0) == 0
                      || lurl.rfind("rtmps://", 0) == 0 || lurl.rfind("rtmpt://", 0) == 0;
    const bool is_hls  = lurl.find(".m3u8") != std::string::npos;
    m_is_hls = is_hls;

    // 通用低延迟解复用选项（对所有协议生效）
    av_dict_set(&opts, "max_delay",       "100000",   0);  // max demux-decode delay (us)
    av_dict_set(&opts, "probesize",       "32768",    0);  // smaller probe for faster start
    av_dict_set(&opts, "fflags",          "nobuffer", 0);  // disable format-level buffering
    av_dict_set(&opts, "analyzeduration", "1000000",  0);  // 1s analysis (default 5s)
    av_dict_set(&opts, "flush_packets",   "1",        0);  // flush packets immediately

    if (is_rtmp) {
        // RTMP live stream options — tuned for low latency
        av_dict_set(&opts, "rtmp_live",   "live", 0);
        av_dict_set(&opts, "rtmp_listen", "0",    0);  // 显式关闭 listen 模式（timeout 选项会导致 listen 被强制开启，故移除 timeout）
        av_dict_set(&opts, "rtmp_buffer", "100",  0);  // 客户端向 RTMP 服务端请求的缓冲时长，FFmpeg 默认 3000ms 是直播高延迟的主因，改小以降低延迟
    } else {
        // HLS(.m3u8) / HTTP(-FLV) options
        // 允许 HLS 播放列表内嵌 https/tls 加密分片，否则会因协议白名单拒绝加载子分片而失败。
        av_dict_set(&opts, "protocol_whitelist",
                    "file,crypto,data,http,https,tcp,tls,hls,applehttp", 0);
        // HTTP 断流自动重连：分片偶发 404/超时时不至于整路失败。
        av_dict_set(&opts, "reconnect",           "1", 0);
        av_dict_set(&opts, "reconnect_streamed",  "1", 0);
        av_dict_set(&opts, "reconnect_delay_max", "2", 0);
        if (is_hls) {
            // 直播 HLS 从最新分片开始，贴近直播边缘、降低起播延迟。
            av_dict_set(&opts, "live_start_index", "-1", 0);
        }
    }

    ffrtmp_log(std::string("avformat_open_input... (") + (is_rtmp ? "rtmp" : is_hls ? "hls" : "http") + ")");

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

FFRTMPVideoCtrl::ReadStatus FFRTMPVideoCtrl::ReadAndDecodeOneFrame()
{
    AVFormatContext *fmt_ctx  = FMT_CTX(m_format_ctx);
    AVCodecContext  *codec_ctx = CDC_CTX(m_codec_ctx);
    AVFrame         *frame    = FRM(m_frame);
    AVPacket        *packet   = PCKT(m_packet);

    int ret = av_read_frame(fmt_ctx, packet);
    if (ret < 0) {
        if (ret == AVERROR_EOF) {
            // 到达流末尾：HLS 直播这是“到达直播边缘”的常态，交由上层做宽限处理。
            return ReadStatus::Eof;
        }
        ffrtmp_err("av_read_frame failed: " + av_err_str(ret));
        return ReadStatus::Error;
    }

    // 读到了一个包（连接存活即算“有进展”），无论它是否属于视频流。
    if (packet->stream_index != m_video_stream_idx) {
        av_packet_unref(packet); return ReadStatus::Frame;
    }

    ret = avcodec_send_packet(codec_ctx, packet);
    av_packet_unref(packet);
    if (ret < 0) { return ReadStatus::Frame; }

    while (ret >= 0) {
        ret = avcodec_receive_frame(codec_ctx, frame);
        if (ret == AVERROR(EAGAIN)) break;
        if (ret < 0) return ReadStatus::Frame;

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
            if (!sws) { av_frame_unref(frame); return ReadStatus::Frame; }
            ffrtmp_log("Scaler (re)created: " + std::to_string(m_video_width) + "x" + std::to_string(m_video_height));
        }

        // Scale into raw RGB buffer (thread-safe, no GDI objects in decoder thread)
        const int fw = m_video_width;
        const int fh = m_video_height;
        int      dst_stride = fw * 3;
        // swscale 输出 RGB24 时，末行可能因 SIMD 一次写满寄存器而多写若干字节；
        // 若目标缓冲按 w*h*3 精确分配，末行溢出会踩坏堆相邻内存，导致随后在
        // CRT(ucrtbase) 中随机崩溃(0xC0000005)。这里额外预留一段安全边距。
        std::vector<uint8_t> buffer((size_t)dst_stride * fh + 64);
        uint8_t *dst[1]     = { buffer.data() };

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
    return ReadStatus::Frame;
}

#else  // !FFRTMP_USE_FFMPEG

// Stubs when FFmpeg is not available
void   FFRTMPVideoCtrl::DecoderThreadFunc()                           {}
bool   FFRTMPVideoCtrl::OpenStream(const std::string &)              { return false; }
void   FFRTMPVideoCtrl::CloseStream()                                {}
FFRTMPVideoCtrl::ReadStatus FFRTMPVideoCtrl::ReadAndDecodeOneFrame() { return ReadStatus::Error; }

#endif // FFRTMP_USE_FFMPEG

// ============================================================================
//  PrinterCameraPanel compatibility API (always compiled)
// ============================================================================

void FFRTMPVideoCtrl::setSize(wxSize size)
{
    SetSize(size);
    SetMinSize(size);
}

void FFRTMPVideoCtrl::setDisplayMode(DisplayMode mode)
{
    if (m_display_mode == mode) return;
    m_display_mode = mode;
    CallAfter([this]() { Refresh(); });
}

void FFRTMPVideoCtrl::setCurComId(com_id_t comId)
{
    // 切换设备：摄像头画面跟随切换（新地址由后续 setStreamUrl 触发 StartStream 正常播放）。
    // 暂停只是显示层面的冻结，不在这里干预。
    m_curComId = comId;
}

void FFRTMPVideoCtrl::sendCameraOpen()
{
    if (m_curComId == ComInvalidId) {
        return;
    }
    // 无条件下发，与 PrinterCameraPanel::onScriptMessage 一致：
    //   · WAN → ComCameraStreamCtrl::exec 经 ComWanConn 把 "open" 发到云端，
    //           云端通知打印机启动摄像头并推流；
    //   · LAN → exec 返回 COM_UNSUPPORTED，自动忽略，无副作用。
    // putCommand 内部对 WAN 未在线的情况会拒绝并记录日志，无需在此判断。
    bool ok = MultiComMgr::inst()->putCommand(m_curComId, new ComCameraStreamCtrl("open"));
    if (!ok) {
        // 仅在失败时记录（通常意味着 WAN 未在线）；成功的保活不打日志，避免刷屏。
        ffrtmp_err("sendCameraOpen putCommand rejected, comId=" + std::to_string(m_curComId));
    }
}

void FFRTMPVideoCtrl::setStreamUrl(const std::string &streamUrl)
{
    StartStream(streamUrl);
}

void FFRTMPVideoCtrl::setOffline()
{
    ffrtmp_log("setOffline called");
    m_paused = false;                          // 断连清除显示暂停态
    StopStream();
    setPlayState(PlayState::Disconnected);      // 右下角状态：打印机断开连接
    CallAfter([this]() {
        wxCriticalSectionLocker lock(m_frame_cs);
        m_frame_ready = false;
        m_rgb_buffer.clear();
        m_rgb_bitmap = wxBitmap();
        // 内联控件常驻显示：离线时不隐藏，保持可见并露出黑底占位图 + “打印机断开连接”状态条。
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
    // 显示暂停：后台解码仍在把最新帧写入 m_rgb_buffer，但这里不更新显示位图，
    // 画面冻结在暂停时刻（暂停与拉流解耦）。
    if (m_paused) {
        return;
    }
    wxCriticalSectionLocker lock(m_frame_cs);
    // 缓冲尾部含安全边距，可能比有效像素大，故用 >= 判断，并只拷贝有效像素 w*h*3。
    const size_t need = (size_t)m_buf_width * m_buf_height * 3;
    if (m_frame_ready && m_buf_width > 0 && m_buf_height > 0 && m_rgb_buffer.size() >= need) {
        wxImage img(m_buf_width, m_buf_height, false);
        memcpy(img.GetData(), m_rgb_buffer.data(), need);
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

    // 底部状态条（播放/暂停 + 状态文字），覆盖在画面之上。
    drawOverlayBar(dc);
}

void FFRTMPVideoCtrl::OnSize(wxSizeEvent & /*event*/)
{
    Refresh();
}

// ============================================================================
//  播放器覆盖层：底部状态条（左下角播放/暂停，右下角状态文字）
// ============================================================================

void FFRTMPVideoCtrl::setPlayState(PlayState s)
{
    // 仅在状态变化时刷新，避免解码线程每帧 setPlayState(Playing) 造成刷新风暴。
    if (m_play_state.exchange(s) == s) {
        return;
    }
    CallAfter([this]() { Refresh(false); });
}

wxString FFRTMPVideoCtrl::statusText() const
{
    // 多语言：msgid 用英文，各语言译文见 localization/flashforge/{lang}/flashforge_{lang}.po。
    switch (m_play_state.load()) {
    case PlayState::Initializing: return _L("Initializing camera...");
    case PlayState::Loading:      return _L("Loading video...");
    case PlayState::Playing:      return _L("Video is playing");
    case PlayState::Paused:       return _L("Video is paused");
    case PlayState::Disconnected: return _L("Printer disconnected");
    case PlayState::Not:
    default:                      return _L("No camera detected");
    }
}

wxRect FFRTMPVideoCtrl::playButtonRect()
{
    // 左下角一块点击区域（比图标略大，便于点击）。
    wxSize client = GetClientSize();
    const int barH = FromDIP(30);
    return wxRect(0, client.y - barH, FromDIP(44), barH);
}

void FFRTMPVideoCtrl::drawOverlayBar(wxDC &dc)
{
    wxSize client = GetClientSize();
    if (client.x <= 0 || client.y <= 0) {
        return;
    }

    const int barH = FromDIP(45);
    const int barY = client.y - barH;

    // 底部状态条背景（深色）
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.SetBrush(wxBrush(*wxWHITE));
    dc.DrawRectangle(0, barY, client.x, barH);

    // 左下角 播放/暂停 图标：暂停/断连时显示“播放三角”，播放中显示“暂停双竖条”。
    auto state = m_play_state.load();
    const int  icon = FromDIP(29);
    const int  ix   = FromDIP(20);
    const int  iy   = barY + (barH - icon) / 2;
    std::string selectIcon = "";
    if (state == PlayState::Not || state == PlayState::Disconnected || state == PlayState::Initializing ||
        state == PlayState::Loading) {
        selectIcon = "offline";
    } else if (m_paused){
        selectIcon = "play";
    } else {
        selectIcon = "pause";
    }
    dc.DrawBitmap(m_playIconMap[selectIcon].bmp(), ix, iy);
    // 右下角 状态文字
    wxString txt = statusText();
    dc.SetFont(Label::Body_13);
    dc.SetTextForeground(wxColour("#666666"));
    wxSize ts = dc.GetTextExtent(txt);
    dc.DrawText(txt, client.x - ts.x - FromDIP(12), barY + (barH - ts.y) / 2);
}

void FFRTMPVideoCtrl::pauseStream()
{
    if (m_paused) {
        return;
    }
    // 暂停与拉流解耦：后台解码/拉流/保活继续运行，只是“冻结显示”——
    // OnFrameReady 不再把最新帧刷到显示位图，画面停在当前帧。
    ffrtmp_log("pauseStream (display-only, decode keeps running)");
    m_paused = true;
    setPlayState(PlayState::Paused);
    Refresh();
}

void FFRTMPVideoCtrl::resumeStream()
{
    if (!m_paused) {
        return;
    }
    // 恢复显示：解码一直在跑，这里只是重新允许把最新帧刷到显示。
    ffrtmp_log("resumeStream (display-only)");
    m_paused = false;
    setPlayState(m_running ? PlayState::Playing : PlayState::Loading);
    Refresh();
}

void FFRTMPVideoCtrl::togglePause()
{
    // 断连状态下按钮不响应。
    if (m_play_state.load() == PlayState::Disconnected || m_play_state.load() == PlayState::Not) {
        return;
    }
    if (!m_paused) {
        pauseStream();
    } else {
        resumeStream();
    }
}

void FFRTMPVideoCtrl::OnLeftDown(wxMouseEvent &event)
{
    if (playButtonRect().Contains(event.GetPosition())) {
        togglePause();
    }
    event.Skip();
}

}} // namespace Slic3r::GUI
