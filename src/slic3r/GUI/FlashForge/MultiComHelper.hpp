#ifndef slic3r_GUI_MultiComHelper_hpp_
#define slic3r_GUI_MultiComHelper_hpp_

#include "MultiComDef.hpp"
#include "Singleton.hpp"

namespace Slic3r { namespace GUI {

class MultiComHelper : public Singleton<MultiComHelper>
{
public:
    void setUid(const std::string &uid) { m_uid = uid; }

    ComErrno getUserAiPointsInfo(com_user_ai_points_info_t &userAiPointsInfo, int msTimeout);

    ComErrno uploadAiImageClound(const std::string &filePath, const std::string &saveName,
        std::string &storeUrl, fnet_progress_callback_t callback, void *callbackData, int msTimeout);

    ComErrno startAiModelJob(const std::string &imageUrl, const std::string &resultFormat,
        com_ai_model_job_result_t &jobResult, int msTimeout);

    ComErrno getAiModelJobState(const std::string &jobId, com_ai_model_job_state_t &jobState, int msTimeout);

    ComErrno abortAiModelJob(const std::string &jobId, int msTimeout);

private:
    std::string m_uid;
};

}} // namespace Slic3r::GUI

#endif
