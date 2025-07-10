#include "MultiComHelper.hpp"
#include "MultiComMgr.hpp"
#include "MultiComUtils.hpp"
#include "WanDevTokenMgr.hpp"

namespace Slic3r { namespace GUI {

ComErrno MultiComHelper::getUserAiPointsInfo(com_user_ai_points_info_t &userAiPointsInfo, int msTimeout)
{
    fnet::FlashNetworkIntfc *intfc = MultiComMgr::inst()->networkIntfc();
    if (intfc == nullptr) {
        return COM_ERROR;
    }
    ScopedWanDevToken token = WanDevTokenMgr::inst()->getScopedToken();
    fnet_user_ai_points_info_t *fnetUserAiPointsInfo;
    int ret = intfc->getUserAiPointsInfo(m_uid.c_str(), token.accessToken().c_str(), &fnetUserAiPointsInfo, msTimeout);
    if (ret != COM_OK) {
        return MultiComUtils::fnetRet2ComErrno(ret);
    }
    fnet::FreeInDestructor freeDevInfos(fnetUserAiPointsInfo, intfc->freeUserAiPointsInfo);
    userAiPointsInfo.totalPoints = fnetUserAiPointsInfo->totalPoints;
    userAiPointsInfo.currAiGeneratePoints = fnetUserAiPointsInfo->currAiGeneratePoints;
    return COM_OK;
}

}} // namespace Slic3r::GUI
