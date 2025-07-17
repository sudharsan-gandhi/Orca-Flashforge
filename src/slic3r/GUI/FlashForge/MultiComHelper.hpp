#ifndef slic3r_GUI_MultiComHelper_hpp_
#define slic3r_GUI_MultiComHelper_hpp_

#include "ComThreadPool.hpp"
#include "MultiComDef.hpp"
#include "Singleton.hpp"

namespace Slic3r { namespace GUI {

class MultiComHelper : public wxEvtHandler, public Singleton<MultiComHelper>
{
public:
    MultiComHelper();

    void setUid(const std::string &uid) { m_uid = uid; }

    void aiModelClickCount(int msTimeout);

    void doBusGetRequest(const std::string &requestId, const std::string &target, int msTimeout);

    ComErrno singOut(int msTimeout);

    ComErrno getUserAiPointsInfo(com_user_ai_points_info_t &userAiPointsInfo, int msTimeout);

    ComErrno uploadAiImageClound(const std::string &filePath, const std::string &saveName,
        std::string &storeUrl, fnet_progress_callback_t callback, void *callbackData, int msTimeout);

    ComErrno startAiModelJob(const std::string &imageUrl, const std::string &resultFormat,
        com_ai_model_job_result_t &jobResult, int msTimeout);

    ComErrno getAiModelJobState(const std::string &jobId, com_ai_model_job_state_t &jobState,
        int msTimeout);

    ComErrno abortAiModelJob(const std::string &jobId, int msTimeout);

    ComErrno getPromoShareData(const std::string &language, std::string &responseData,
        int msTimeout);

private:
    std::string m_uid;
    ComThreadPool m_threadPool;
};

}} // namespace Slic3r::GUI

#endif
