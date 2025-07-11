#include "MultiComHelper.hpp"
#include "MultiComMgr.hpp"
#include "MultiComUtils.hpp"
#include "WanDevTokenMgr.hpp"

namespace Slic3r { namespace GUI {

ComErrno MultiComHelper::singOut(int msTimeout)
{
    fnet::FlashNetworkIntfc *intfc = MultiComMgr::inst()->networkIntfc();
    if (intfc == nullptr) {
        return COM_ERROR;
    }
    ScopedWanDevToken token = WanDevTokenMgr::inst()->getScopedToken();
    return MultiComUtils::fnetRet2ComErrno(intfc->signOut(token.accessToken().c_str(), msTimeout));
}

ComErrno MultiComHelper::getUserAiPointsInfo(com_user_ai_points_info_t &userAiPointsInfo, int msTimeout)
{
    fnet::FlashNetworkIntfc *intfc = MultiComMgr::inst()->networkIntfc();
    if (intfc == nullptr) {
        return COM_ERROR;
    }
    ScopedWanDevToken token = WanDevTokenMgr::inst()->getScopedToken();
    fnet_user_ai_points_info_t *fnetUserAiPointsInfo;
    ComErrno ret = MultiComUtils::fnetRet2ComErrno(intfc->getUserAiPointsInfo(
        m_uid.c_str(), token.accessToken().c_str(), &fnetUserAiPointsInfo, msTimeout));
    if (ret != FNET_OK) {
        return ret;
    }
    fnet::FreeInDestructor freeDevInfos(fnetUserAiPointsInfo, intfc->freeUserAiPointsInfo);
    userAiPointsInfo.totalPoints = fnetUserAiPointsInfo->totalPoints;
    userAiPointsInfo.currAiGeneratePoints = fnetUserAiPointsInfo->currAiGeneratePoints;
    return ret;
}

ComErrno MultiComHelper::uploadAiImageClound(const std::string &filePath, const std::string &saveName,
    std::string &storeUrl, fnet_progress_callback_t callback, void *callbackData, int msTimeout)
{
    fnet::FlashNetworkIntfc *intfc = MultiComMgr::inst()->networkIntfc();
    if (intfc == nullptr) {
        return COM_ERROR;
    }
    ScopedWanDevToken token = WanDevTokenMgr::inst()->getScopedToken();
    fnet_upload_file_data_t uploadFileData;
    uploadFileData.filePath = filePath.c_str();
    uploadFileData.saveName = saveName.c_str();
    uploadFileData.callback = callback;
    uploadFileData.callbackData = callbackData;
    fnet_clound_file_data_t *cloundFileData;
    ComErrno ret = MultiComUtils::fnetRet2ComErrno(intfc->uploadAiImageClound(
        m_uid.c_str(), token.accessToken().c_str(), &uploadFileData, &cloundFileData, msTimeout));
    if (ret != FNET_OK) {
        return ret;
    }
    fnet::FreeInDestructor freeCloundFileData(cloundFileData, intfc->freeCloundFileData);
    storeUrl = cloundFileData->storageUrl;
    return ret;
}

ComErrno MultiComHelper::startAiModelJob(const std::string &imageUrl, const std::string &resultFormat,
    com_ai_model_job_result_t &jobResult, int msTimeout)
{
    fnet::FlashNetworkIntfc *intfc = MultiComMgr::inst()->networkIntfc();
    if (intfc == nullptr) {
        return COM_ERROR;
    }
    ScopedWanDevToken token = WanDevTokenMgr::inst()->getScopedToken();
    fnet_start_ai_model_job_data_t jobData;
    jobData.imageUrl = imageUrl.c_str();
    jobData.resultFormat = resultFormat.c_str();
    fnet_start_ai_model_job_result *fnetJobResult;
    ComErrno ret = MultiComUtils::fnetRet2ComErrno(intfc->startAiModelJob(
        m_uid.c_str(), token.accessToken().c_str(), &jobData, &fnetJobResult, msTimeout));
    if (ret != FNET_OK) {
        return ret;
    }
    fnet::FreeInDestructor freeJobResult(fnetJobResult, intfc->freeStartAiModelJobResult);
    jobResult.status = fnetJobResult->status;
    jobResult.jobId = fnetJobResult->jobId;
    jobResult.posInQueue = fnetJobResult->posInQueue;
    jobResult.queueLength = fnetJobResult->queueLength;
    return ret;
}

ComErrno MultiComHelper::getAiModelJobState(const std::string &jobId, com_ai_model_job_state_t &jobState,
    int msTimeout)
{
    fnet::FlashNetworkIntfc *intfc = MultiComMgr::inst()->networkIntfc();
    if (intfc == nullptr) {
        return COM_ERROR;
    }
    ScopedWanDevToken token = WanDevTokenMgr::inst()->getScopedToken();
    fnet_ai_model_job_state_t *fnetJobState;
    ComErrno ret = MultiComUtils::fnetRet2ComErrno(intfc->getAiModelJobState(
        m_uid.c_str(), token.accessToken().c_str(), jobId.c_str(), &fnetJobState, msTimeout));
    if (ret != FNET_OK) {
        return ret;
    }
    fnet::FreeInDestructor freeJobState(fnetJobState, intfc->freeAiModelJobState);
    jobState.status = fnetJobState->status;
    jobState.jobId = fnetJobState->jobId;
    jobState.posInQueue = fnetJobState->posInQueue;
    jobState.queueLength = fnetJobState->queueLength;
    jobState.externalJobId = fnetJobState->externalJobId;
    jobState.models.resize(fnetJobState->modelCnt);
    for (int i = 0; i < fnetJobState->modelCnt; ++i) {
        jobState.models[i].modelType = fnetJobState->models[i].modelType;
        jobState.models[i].modelUrl = fnetJobState->models[i].modelUrl;
    }
    return ret;
}

ComErrno MultiComHelper::abortAiModelJob(const std::string &jobId, int msTimeout)
{
    fnet::FlashNetworkIntfc *intfc = MultiComMgr::inst()->networkIntfc();
    if (intfc == nullptr) {
        return COM_ERROR;
    }
    ScopedWanDevToken token = WanDevTokenMgr::inst()->getScopedToken();
    ComErrno ret = MultiComUtils::fnetRet2ComErrno(intfc->abortAiModelJob(
        m_uid.c_str(), token.accessToken().c_str(), jobId.c_str(), msTimeout));
    if (ret != FNET_OK) {
        return ret;
    }
    return ret;
}

}} // namespace Slic3r::GUI
