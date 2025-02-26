#ifndef _Slic3r_GUI_FFDownloadTool_hpp_
#define _Slic3r_GUI_FFDownloadTool_hpp_

#include <atomic>
#include <wx/event.h>
#include <vector>
#include "slic3r/GUI/FlashForge/ComThreadPool.hpp"

namespace Slic3r { namespace GUI {

struct FFDownloadFinishedEvent : public wxCommandEvent {
    int taskId;
    bool succeed;
    std::vector<char> data; // when downloading to the disk, it always empty
};

wxDECLARE_EVENT(EVT_FF_DOWNLOAD_FINISHED, FFDownloadFinishedEvent);

class FFDownloadTool : public wxEvtHandler
{
public:
    FFDownloadTool(size_t maxThreadCnt, int expiryTimeout);

    ~FFDownloadTool();

    int downloadMem(const std::string &url, int msConnectTimeout, int msTimeout);

    int downloadDisk(const std::string &url, const wxString &saveName, int msConnectTimeout, int msTimeout);

    void wait(bool abortTasks);

private:
    static int callback(long long now, long long total, void *callbackData);

private:
    int m_baseTaskId;
    std::atomic_bool m_abortTasks;
    ComThreadPool m_threadPool;
};

}} // namespace Slic3r::GUI

#endif
