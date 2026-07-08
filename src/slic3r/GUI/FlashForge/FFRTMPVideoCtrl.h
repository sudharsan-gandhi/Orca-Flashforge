#ifndef slic3r_GUI_FFRTMPVideoCtrl_hpp_
#define slic3r_GUI_FFRTMPVideoCtrl_hpp_

#include <wx/panel.h>
#include <wx/bitmap.h>
#include <wx/timer.h>
#include <wx/dialog.h>
#include <atomic>
#include <chrono>
#include <string>
#include <memory>
#include <thread>
#include <vector>

#include "MultiComDef.hpp"

namespace Slic3r { namespace GUI {

class FFRTMPVideoCtrl : public wxPanel
{
public:
    // 画面填充方式
    //   Fit   : 等比缩放完整显示，留黑边（letterbox / contain）—— 默认，行为对齐 VLC 等播放器
    //   Cover : 等比缩放铺满控件，超出部分裁掉（铺满/裁边）
    enum class DisplayMode { Fit, Cover };

    FFRTMPVideoCtrl(wxWindow *parent);
    ~FFRTMPVideoCtrl();

    // 设置画面填充方式（默认 Fit）
    void setDisplayMode(DisplayMode mode);

    // Start RTMP stream playback on background thread. Non-blocking.
    void StartStream(const std::string &url);

    // Stop playback and release resources.
    void StopStream();

    // Returns true if a decode thread is currently active.
    bool IsStreaming() const;

    // Show the video in a popup dialog (full-screen).
    void ShowFullScreenPopup();

    // ---- PrinterCameraPanel compatibility API ----

    // Sets panel size and min/max constraints
    void setSize(wxSize size);

    // Records current communication identifier
    void setCurComId(com_id_t comId);

    // Start streaming from the given URL
    void setStreamUrl(const std::string &streamUrl);

    // Stop streaming and show offline placeholder
    void setOffline();

    // Show popup (full-screen) dialog
    void showPopup();

protected:
    void OnPaint(wxPaintEvent &event);
    void OnSize(wxSizeEvent &event);

private:
    // Threading
    void DecoderThreadFunc();
    void OnFrameReady();

    // 向设备发送 camera "open" 指令：WAN 下经云端让打印机开始/持续推流
    // （LAN 下命令层会自行忽略）。对齐 PrinterCameraPanel 的 rtsp_player_continue 机制。
    void sendCameraOpen();

    // ---- 播放器覆盖层（底部状态条：左下角播放/暂停按钮，右下角状态文字）----
    enum class PlayState { Initializing, Loading, Playing, Paused, Disconnected };
    void     setPlayState(PlayState s);  // 线程安全，可从解码线程调用
    wxString statusText() const;         // 右下角状态文字
    wxRect   playButtonRect();           // 左下角播放/暂停按钮的点击区域
    void     drawOverlayBar(wxDC &dc);   // 绘制底部状态条
    void     togglePause();              // 切换 播放/暂停
    void     OnLeftDown(wxMouseEvent &event);

    // FFmpeg helpers (only called from decode thread)
    bool OpenStream(const std::string &url);
    void CloseStream();

    // FFmpeg 阻塞 I/O 中断回调：m_running 变 false 时立即中止 open/read，
    // 让 StopStream/暂停 秒级停止，避免主线程 join 阻塞导致 UI 卡顿。
    static int interruptCb(void *opaque);

    // 单次读取结果：区分“读到数据/直播到达边缘(EOF)/真正错误”，
    // 以便对 HLS 等分段协议的 EOF 做容忍处理，而不是简单断流重连。
    enum class ReadStatus { Frame, Eof, Error };
    ReadStatus ReadAndDecodeOneFrame();

    // Synchronisation
    std::atomic<bool>              m_running{false};
    std::atomic<bool>              m_frame_ready{false};
    std::atomic<bool>              m_skip_initial_delay{false}; // 同一路流重启时跳过预热等待
    wxCriticalSection              m_frame_cs;
    std::unique_ptr<std::thread> m_thread;

    // Latest decoded RGB frame
    std::vector<uint8_t> m_rgb_buffer;  // raw RGB data from decoder thread
    int                  m_buf_width{0};   // m_rgb_buffer 对应的帧宽（与 buffer 一起在锁内更新，避免与解码线程竞争）
    int                  m_buf_height{0};  // m_rgb_buffer 对应的帧高
    wxBitmap             m_rgb_bitmap;  // created on main thread in OnFrameReady
    int                  m_video_width{1280};  // 解码线程内部使用（scaler 尺寸），非渲染依据
    int                  m_video_height{720};

    // 画面填充方式（默认完整显示，不裁剪）
    DisplayMode          m_display_mode{DisplayMode::Fit};

    // FFmpeg objects (opaque via void*, only valid while m_running)
    void *m_format_ctx{nullptr};
    void *m_codec_ctx{nullptr};
    void *m_sws_ctx{nullptr};
    void *m_frame{nullptr};
    void *m_packet{nullptr};
    int   m_video_stream_idx{-1};

    // 当前流是否为 HLS(.m3u8)：仅解码线程读写，用于对直播边缘 EOF 做容忍。
    bool  m_is_hls{false};

    // Offline / placeholder image
    wxBitmap m_offline_bitmap;

    // Com ID for this camera (from PrinterCameraPanel API)
    com_id_t m_curComId{ComInvalidId};

    // 推流保活：播放期间周期性重发 camera "open"，维持打印机推流会话，
    // 避免云端会话超时（约 10s）后停止产出分片。
    wxTimer m_keepalive_timer;
    int     m_keepalive_interval_ms{5000};

    // Popup dialog for full-screen view
    wxDialog *m_popup_dlg{nullptr};
    wxWindow *m_inline_parent{nullptr}; // 内联时的父窗口，弹窗关闭后需归还，避免控件被孤立到 mainframe
    wxSize    m_inline_size;            // 内联时的固定尺寸，弹窗关闭后恢复

    // Current stream URL (for reconnect)
    std::string m_url;

    // Reconnect
    // 首次连接前的等待时间。设为 0 以最小化首帧延迟；若摄像头尚未开始推流，
    // OpenStream 会失败并由重连逻辑自动重试，无需在此白等。
    int m_initial_delay_ms{0};
    int m_reconnect_delay_ms{1000};  // 首连失败时快速重试，缩短首帧出现前的等待
    int m_reconnect_delay_max_ms{5000}; // 退避上限：直播源持续重连，不永久放弃
    int m_reconnect_attempts{0};
    // 达到该次数后显示离线占位图（黑底，非白屏），但仍以退避间隔持续重连，
    // 直到 StopStream/setOffline 主动结束 —— 保证“不会简单断流”。
    int m_max_reconnect_attempts{10};

    // 离线占位图是否已显示，避免在持续重连期间反复 CallAfter 刷新。
    std::atomic<bool> m_offline_shown{false};

    // 播放器状态（右下角状态文字），可从解码线程原子更新。
    std::atomic<PlayState> m_play_state{PlayState::Disconnected};
    // 用户是否手动暂停：暂停时停止解码但保留最后一帧冻结显示。
    bool                   m_paused{false};
};

}} // namespace Slic3r::GUI

#endif
