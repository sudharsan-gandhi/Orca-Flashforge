#include "FFDownloadTool.hpp"
#include "slic3r/GUI/FlashForge/MultiComUtils.hpp"

namespace Slic3r { namespace GUI {

wxDEFINE_EVENT(EVT_FF_DOWNLOAD_FINISHED, FFDownloadFinishedEvent);

FFDownloadTool::FFDownloadTool(size_t maxThreadCnt, int expiryTimeout)
    : m_baseTaskId(0)
    , m_abortTasks(false)
    , m_threadPool(maxThreadCnt, expiryTimeout)
{
}

FFDownloadTool::~FFDownloadTool()
{
    wait(false);
}

int FFDownloadTool::downloadMem(const std::string &url, int msConnectTimeout, int msTimeout)
{
    m_abortTasks = false;
    int taskId = m_baseTaskId++;
    m_threadPool.post([this, taskId, url, msConnectTimeout, msTimeout]() {
        std::vector<char> bytes;
        ComErrno ret = MultiComUtils::downloadFileMem(
            url, bytes, callback, this, msConnectTimeout, msTimeout);
        FFDownloadFinishedEvent *event = new FFDownloadFinishedEvent;
        event->SetEventType(EVT_FF_DOWNLOAD_FINISHED);
        event->taskId = taskId;
        event->succeed = ret == COM_OK;
        event->data = std::move(bytes);
        QueueEvent(event);
    });
    return taskId;
}

int FFDownloadTool::downloadDisk(const std::string &url, const wxString &saveName,
    int msConnectTimeout, int msTimeout)
{
    m_abortTasks = false;
    int taskId = m_baseTaskId++;
    m_threadPool.post([this, taskId, url, saveName, msConnectTimeout, msTimeout]() {
        ComErrno ret = MultiComUtils::downloadFileDisk(
            url, saveName, callback, this, msConnectTimeout, msTimeout);
        FFDownloadFinishedEvent *event = new FFDownloadFinishedEvent;
        event->SetEventType(EVT_FF_DOWNLOAD_FINISHED);
        event->taskId = taskId;
        event->succeed = ret == COM_OK;
        QueueEvent(event);
    });
    return taskId;
}

void FFDownloadTool::wait(bool abortTasks)
{
    if (abortTasks) {
        m_abortTasks = true;
        m_threadPool.clear();
    }
    m_threadPool.wait();
}

int FFDownloadTool::callback(long long now, long long total, void *callbackData)
{
    FFDownloadTool *self = (FFDownloadTool *)callbackData;
    return self->m_abortTasks;
}

}} // namespace Slic3r::GUI
