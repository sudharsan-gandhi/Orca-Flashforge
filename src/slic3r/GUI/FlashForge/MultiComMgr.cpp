#include "MultiComMgr.hpp"
#include <boost/filesystem.hpp>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include "FreeInDestructor.h"
#include "WanDevTokenMgr.hpp"

namespace Slic3r { namespace GUI {

MultiComMgr::MultiComMgr()
    : m_idNum(ComInvalidId + 1)
    , m_login(false)
    , m_httpOnline(false)
    , m_nimOnline(false)
    , m_procPendingWanDevTimer(this)
{
    com_dev_data_t devData;
    devData.connectMode = COM_CONNECT_LAN;
    devData.devProduct = nullptr;
    devData.devDetail = nullptr;
    devData.lanGcodeList.gcodeDatas = nullptr;
    devData.lanGcodeList.gcodeCnt = 0;
    devData.wanGcodeList.gcodeDatas = nullptr;
    devData.wanGcodeList.gcodeCnt = 0;
    memset(&devData.lanDevInfo, 0, sizeof(devData.lanDevInfo));
    m_datMap.emplace(ComInvalidId, devData);
    Bind(wxEVT_TIMER, &MultiComMgr::onTimer, this);
}

bool MultiComMgr::initalize(const std::string &dllPath, const std::string &dataDir)
{
    if (networkIntfc() != nullptr) {
        return false;
    }
    wxFileName appFileName(wxStandardPaths::Get().GetExecutablePath());
    wxString appPathWithSep = appFileName.GetPathWithSep();
    std::string logFileDir = dataDir + "/FlashNetwork";
    bool debug = wxFileName::FileExists(appPathWithSep + "FLASHNETWORK_DEBUG");

    fnet_log_settings_t logSettings;
    logSettings.fileDir = logFileDir.c_str();
    logSettings.expireHours = 72;
    logSettings.level = debug ? FNET_LOG_LEVEL_DEBUG : FNET_LOG_LEVEL_INFO;

    std::string serverSettingsPath = (appPathWithSep + "FLASHNETWORK2.DAT").ToUTF8().data();
    m_networkIntfc.reset(new fnet::FlashNetworkIntfc(
        dllPath.c_str(), serverSettingsPath.c_str(), logSettings));
    if (!m_networkIntfc->isOk()) {
        BOOST_LOG_TRIVIAL(error) << "initalize FlashNetwork failed: " << dllPath;
        m_networkIntfc.reset();
        return false;
    }
    m_wanDevMaintainThd.reset(new WanDevMaintainThd(m_networkIntfc.get()));
    m_wanDevMaintainThd->Bind(RELOGIN_HTTP_EVENT, &MultiComMgr::onReloginHttp, this);
    m_wanDevMaintainThd->Bind(GET_WAN_DEV_EVENT, &MultiComMgr::onUpdateWanDev, this);
    m_wanDevMaintainThd->Bind(COM_GET_USER_PROFILE_EVENT, &MultiComMgr::onUpdateUserProfile, this);

    auto queueEvent = [this](auto &event) { QueueEvent(event.Clone()); };
    m_sendGcodeThd.reset(new WanDevSendGcodeThd(m_networkIntfc.get()));
    m_sendGcodeThd->Bind(COM_SEND_GCODE_PROGRESS_EVENT, queueEvent);
    m_sendGcodeThd->Bind(COM_SEND_GCODE_FINISH_EVENT, queueEvent);

    m_threadPool.reset(new boost::asio::thread_pool(4));
    m_threadExitEvent.set(false);

    std::string nimAppDir = dataDir + "/nimData";
    ComWanNimConn::inst()->initalize(networkIntfc(), nimAppDir.c_str());
    ComWanNimConn::inst()->Bind(WAN_CONN_STATUS_EVENT, &MultiComMgr::onWanConnStatus, this);
    ComWanNimConn::inst()->Bind(WAN_CONN_READ_EVENT, &MultiComMgr::onWanConnRead, this);
    ComWanNimConn::inst()->Bind(WAN_CONN_SUBSCRIBE_EVENT, &MultiComMgr::onWanConnSubscribe, this);
    WanDevTokenMgr::inst()->Bind(COM_REFRESH_TOKEN_EVENT, &MultiComMgr::onRefreshToken, this);
    return true;
}

void MultiComMgr::uninitalize()
{
    if (networkIntfc() == nullptr) {
        return;
    }
    WanDevTokenMgr::inst()->Unbind(COM_REFRESH_TOKEN_EVENT, &MultiComMgr::onRefreshToken, this);
    ComWanNimConn::inst()->Unbind(WAN_CONN_STATUS_EVENT, &MultiComMgr::onWanConnStatus, this);
    ComWanNimConn::inst()->Unbind(WAN_CONN_READ_EVENT, &MultiComMgr::onWanConnRead, this);
    ComWanNimConn::inst()->Unbind(WAN_CONN_SUBSCRIBE_EVENT, &MultiComMgr::onWanConnSubscribe, this);
    ComWanNimConn::inst()->uninitalize();
    m_threadExitEvent.set(true);
    m_threadPool.reset();
    m_sendGcodeThd->exit();
    m_sendGcodeThd.reset();
    m_wanDevMaintainThd->exit();
    m_wanDevMaintainThd.reset();
    m_networkIntfc.reset();
}

fnet::FlashNetworkIntfc *MultiComMgr::networkIntfc()
{
    return m_networkIntfc.get();
}

com_id_t MultiComMgr::addLanDev(const fnet_lan_dev_info_t &devInfo, const std::string &checkCode)
{
    if (networkIntfc() == nullptr) {
        return ComInvalidId;
    }
    com_ptr_t comPtr = std::make_shared<ComConnection>(m_idNum, checkCode, devInfo, networkIntfc());
    com_dev_data_t devData = { COM_CONNECT_LAN, devInfo, com_wan_dev_info_t(), nullptr };
    initConnection(comPtr, devData);
    return m_idNum++;
}

void MultiComMgr::removeLanDev(com_id_t id)
{
    auto it = m_ptrMap.left.find(id);
    if (it == m_ptrMap.left.end()) {
        return;
    }
    it->second->disconnect(0);
}

ComErrno MultiComMgr::addWanDev(const com_token_data_t &tokenData, int tryCnt, int tryMsInterval)
{
    auto tryDo = [tryCnt, tryMsInterval](const std::function<ComErrno()> &func) {
        ComErrno ret = COM_ERROR;
        for (int i = 0; i < tryCnt; ++i) {
            ret = func();
            if (ret == COM_OK) {
                return ret;
            } else if (i + 1 < tryCnt) {
                boost::this_thread::sleep_for(boost::chrono::milliseconds(tryMsInterval));
            }
        }
        return ret;
    };
    BOOST_LOG_TRIVIAL(info) << "MultiComMgr::addWanDev";
    if (networkIntfc() == nullptr || m_login) {
        return COM_ERROR;
    }
    com_user_profile_t userProfile;
    ComErrno ret = tryDo([&]() {
        return MultiComUtils::getUserProfile(tokenData.accessToken, userProfile);
    });
    if (ret != COM_OK) {
        return ret;
    }
    com_nim_data_t nimData;
    ret = tryDo([&]() {
        return MultiComUtils::getNimData(userProfile.uid, tokenData.accessToken, nimData);
    });
    if (ret != COM_OK) {
        return ret;
    }
    m_login = true;
    m_httpOnline = true;
    m_nimOnline = true;
    m_uid = userProfile.uid;
    m_nimAppAccoutId = nimData.appNimAccountId;
    m_commandFailedUpdating = false;
    m_commandFailedUpdateTime = std_precise_clock::time_point::min();
    m_wanDevMaintainThd->setUid(userProfile.uid);
    WanDevTokenMgr::inst()->start(tokenData, networkIntfc()); // initialize global token
    //
    ret = ComWanNimConn::inst()->createConn(nimData.nimAppKey.c_str(), nimData.nimAccountId.c_str(),
        nimData.nimToken.c_str());
    if (ret != COM_OK) {
        m_login = false;
        m_httpOnline = false;
        m_nimOnline = false;
        return ret;
    }
    m_procPendingWanDevTimer.Start(3000);
    return ret;
}

void MultiComMgr::removeWanDev()
{
    BOOST_LOG_TRIVIAL(info) << "MultiComMgr::removeWanDev";
    if (!m_login) {
        return;
    }
    for (auto &comPtr : m_comPtrs) {
        if (comPtr->connectMode() == COM_CONNECT_WAN) {
            comPtr.get()->disconnect(0);
        }
    }
    m_login = false;
    m_httpOnline = false;
    m_nimOnline = false;
    m_procPendingWanDevTimer.Stop();
    m_subscribeDevStatusTimer.Stop();
    m_wanDevMaintainThd->stop();
    WanDevTokenMgr::inst()->exit();
    ComWanNimConn::inst()->freeConn();
}

ComErrno MultiComMgr::bindWanDev(const std::string &ip, unsigned short port,
    const std::string &serialNumber, unsigned short pid, const std::string &name)
{
    if (!m_httpOnline || !m_nimOnline) {
        return COM_ERROR;
    }
    ScopedWanDevToken token = WanDevTokenMgr::inst()->getScopedToken();
    fnet_wan_dev_bind_data_t *bindData;
    int ret = m_networkIntfc->bindWanDev(m_uid.c_str(), token.accessToken().c_str(),
        serialNumber.c_str(), pid, name.c_str(), &bindData, ComTimeoutWan);
    fnet::FreeInDestructor freeBinData(bindData, m_networkIntfc->freeBindData);
    if (ret == FNET_OK) {
        boost::asio::post(*m_threadPool, [this, ip, port, serialNumber]() {
            for (int i = 0; i < 3 && !m_threadExitEvent.get(); ++i) {
                int ret = m_networkIntfc->notifyLanDevWanBind(
                    ip.c_str(), port, serialNumber.c_str(), ComTimeoutLan);
                if (ret == FNET_OK) {
                    break;
                }
                m_threadExitEvent.waitTrue(3000);
            }
        });
        ComWanNimConn::inst()->syncBindDev(m_nimAppAccoutId);
        m_wanDevMaintainThd->setUpdateWanDev();
    }
    return MultiComUtils::fnetRet2ComErrno(ret);
}

ComErrno MultiComMgr::unbindWanDev(const std::string &serialNumber, const std::string &devId)
{
    if (!m_httpOnline || !m_nimOnline) {
        return COM_ERROR;
    }
    ScopedWanDevToken token = WanDevTokenMgr::inst()->getScopedToken();
    int ret = m_networkIntfc->unbindWanDev(
        m_uid.c_str(), token.accessToken().c_str(), devId.c_str(), ComTimeoutWan);
    if (ret == FNET_OK) {
        ComWanNimConn::inst()->syncUnbindDev(m_nimAppAccoutId);
        for (auto &comPtr : m_comPtrs) {
            if (comPtr->deviceId() == devId) {
                if (m_readyIdSet.find(comPtr->id()) != m_readyIdSet.end()) {
                    const char *name = m_datMap.at(comPtr->id()).devDetail->name;
                    BOOST_LOG_TRIVIAL(info) << name << ", " << serialNumber << ", unbind_disconnect";
                }
                comPtr->disconnect(0);
                break;
            }
        }
    }
    return MultiComUtils::fnetRet2ComErrno(ret);
}

com_id_list_t MultiComMgr::getReadyDevList()
{
    com_id_list_t idList;
    for (auto &id : m_readyIdSet) {
        idList.push_back(id);
    }
    return idList;
}

const com_dev_data_t &MultiComMgr::devData(com_id_t id, bool *valid /* = nullptr */)
{
    auto it = m_ptrMap.left.find(id);
    if (valid != nullptr) {
        *valid = (it != m_ptrMap.left.end());
    }
    if (it == m_ptrMap.left.end()) {
        return m_datMap.at(ComInvalidId);
    } else {
        return m_datMap.at(it->get_left());
    }
}

bool MultiComMgr::putCommand(com_id_t id, ComCommand *command)
{
    ComCommandPtr commandPtr(command);
    auto it = m_ptrMap.left.find(id);
    if (it == m_ptrMap.left.end()) {
        return false;
    }
    if (it->second->connectMode() == COM_CONNECT_WAN && (!m_httpOnline || !m_nimOnline)) {
        return false;
    }
    m_ptrMap.left.at(id)->putCommand(commandPtr);
    return true;
}

bool MultiComMgr::abortSendGcode(com_id_t id, int commandId)
{
    auto it = m_ptrMap.left.find(id);
    if (it == m_ptrMap.left.end()) {
        return false;
    }
    m_ptrMap.left.at(id)->abortSendGcode(commandId);
    return true;
}

bool MultiComMgr::wanSendGcode(const std::vector<std::string> &devIds,
    const std::vector<std::string> &nimAccountIds, const com_send_gcode_data_t &sendGocdeData)
{
    if (!m_httpOnline || !m_nimOnline) {
        return false;
    }
    return m_sendGcodeThd->startSendGcode(m_uid, devIds, nimAccountIds, sendGocdeData);
}

bool MultiComMgr::abortWanSendGcode()
{
    return m_sendGcodeThd->abortSendGcode();
}

void MultiComMgr::initConnection(const com_ptr_t &comPtr, const com_dev_data_t &devData)
{
    m_comPtrs.push_back(comPtr);
    m_ptrMap.insert(com_ptr_map_val_t(comPtr->id(), comPtr.get()));
    m_datMap.emplace(comPtr->id(), devData);
    if (devData.connectMode == COM_CONNECT_WAN) {
        m_devNimAccountIdMap.emplace(devData.wanDevInfo.nimAccountId, comPtr->id());
    }
    auto queueEvent = [this](auto &event) { QueueEvent(event.Clone()); };
    comPtr->Bind(COM_CONNECTION_READY_EVENT, &MultiComMgr::onConnectionReady, this);
    comPtr->Bind(COM_CONNECTION_EXIT_EVENT, &MultiComMgr::onConnectionExit, this);
    comPtr->Bind(COM_DEV_DETAIL_UPDATE_EVENT, &MultiComMgr::onDevDetailUpdate, this);
    comPtr->Bind(COM_SEND_UPDATE_DETAIL_FAILED_EVENT, &MultiComMgr::onSendUpdateDetailFailed, this);
    comPtr->Bind(COM_GET_DEV_GCODE_LIST_EVENT, &MultiComMgr::onGetDevGcodeList, this);
    comPtr->Bind(COM_START_JOB_EVENT, queueEvent);
    comPtr->Bind(COM_GET_GCODE_THUMB_EVENT, [this](auto &event){ QueueEvent(event.MoveClone()); });
    comPtr->Bind(COM_SEND_GCODE_PROGRESS_EVENT, queueEvent);
    comPtr->Bind(COM_SEND_GCODE_FINISH_EVENT, queueEvent);
    comPtr->Bind(COMMAND_FAILED_EVENT, &MultiComMgr::onCommandFailed, this);
    comPtr->connect();
}

void MultiComMgr::onTimer(const wxTimerEvent &event)
{
    if (event.GetId() == m_procPendingWanDevTimer.GetId()) {
        if (!m_pendingWanDevDatas.empty()) {
            if (!m_httpOnline || !m_nimOnline) {
                m_pendingWanDevDatas.clear();
                return;
            }
            std::vector<std::string> nimAccountIds;
            for (auto it = m_pendingWanDevDatas.begin(); it != m_pendingWanDevDatas.end();) {
                const com_wan_dev_info_t &wanDevInfo = it->wanDevInfo;
                if (m_devNimAccountIdMap.find(wanDevInfo.nimAccountId) == m_devNimAccountIdMap.end()) {
                    com_ptr_t comPtr = std::make_shared<ComConnection>(m_idNum++, m_uid,
                        wanDevInfo.serialNumber, wanDevInfo.devId, wanDevInfo.nimAccountId, networkIntfc());
                    initConnection(comPtr, *it);
                    nimAccountIds.push_back(wanDevInfo.nimAccountId);
                    it = m_pendingWanDevDatas.erase(it);
                } else {
                    ++it;
                }
            }
            ComWanNimConn::inst()->subscribeDevStatus(nimAccountIds, SubscribeDevStatusDuration);
        }
    } else if (event.GetId() == m_subscribeDevStatusTimer.GetId()) {
        if (!m_nimOnline) {
            return;
        }
        subscribeWanDevNimStatus();
    }
}

void MultiComMgr::onReloginHttp(ReloginHttpEvent &event)
{
    if (!m_login || event.ret != COM_OK && event.ret != COM_UNAUTHORIZED) {
        m_networkIntfc->freeWanDevList(event.devInfos, event.devCnt);
        return;
    }
    if (event.ret == COM_UNAUTHORIZED && !WanDevTokenMgr::inst()->tokenExpired(event.accessToken)) {
        m_networkIntfc->freeWanDevList(event.devInfos, event.devCnt);
        removeWanDev();
        QueueEvent(new ComWanDevMaintainEvent(COM_WAN_DEV_MAINTAIN_EVENT, false, false, event.ret));
        return;
    }
    m_httpOnline = true;
    m_wanDevMaintainThd->setUpdateUserProfile();
    updateWanDevDetail();

    GetWanDevEvent updateWanDevEvent;
    updateWanDevEvent.SetEventType(GET_WAN_DEV_EVENT);
    updateWanDevEvent.ret = event.ret;
    updateWanDevEvent.uid = event.uid;
    updateWanDevEvent.devInfos = event.devInfos;
    updateWanDevEvent.devCnt = event.devCnt;
    onUpdateWanDev(updateWanDevEvent);
    QueueEvent(new ComWanDevMaintainEvent(COM_WAN_DEV_MAINTAIN_EVENT, true, m_nimOnline, COM_OK));
}

void MultiComMgr::onUpdateWanDev(const GetWanDevEvent &event)
{
    fnet::FreeInDestructorArg freeDevInfos(event.devInfos, m_networkIntfc->freeWanDevList, event.devCnt);
    if (m_uid != event.uid || !m_httpOnline || !m_nimOnline) {
        return;
    }
    if (event.ret != COM_OK) {
        maintianWanDev(event.ret);
        return;
    }
    std::map<std::string, fnet_wan_dev_info_t *> devInfoMap;
    for (int i = 0; i < event.devCnt; ++i) {
        const char *devId = event.devInfos[i].devId;
        if (devInfoMap.find(devId) != devInfoMap.end()) {
            BOOST_LOG_TRIVIAL(fatal) << devId << ", duplicated_devId";
        }
        devInfoMap.emplace(event.devInfos[i].devId, &event.devInfos[i]);
    }
    for (auto &comPtr : m_comPtrs) {
        if (comPtr->connectMode() == COM_CONNECT_WAN) {
            auto it = devInfoMap.find(comPtr->deviceId());
            if (it == devInfoMap.end()) {
                comPtr.get()->disconnect(0);
            } else {
                updateWanDevInfo(comPtr->id(), it->second->name, it->second->status,
                    it->second->location);
            }
        }
    }
    m_pendingWanDevDatas.clear();
    std::vector<std::string> nimAccountIds;
    for (int i = 0; i < event.devCnt; ++i) {
        const fnet_wan_dev_info_t &wanDevInfo = event.devInfos[i];
        auto it = m_devNimAccountIdMap.find(wanDevInfo.nimAccountId);
        if (it == m_devNimAccountIdMap.end()) {
            com_ptr_t comPtr = std::make_shared<ComConnection>(m_idNum++, m_uid,
                wanDevInfo.serialNumber, wanDevInfo.devId, wanDevInfo.nimAccountId, networkIntfc());
            initConnection(comPtr, makeDevData(&wanDevInfo));
            nimAccountIds.push_back(wanDevInfo.nimAccountId);
        } else if (m_ptrMap.left.at(it->second)->isDisconnect()) {
            m_pendingWanDevDatas.push_back(makeDevData(&wanDevInfo));
        }
    }
    ComWanNimConn::inst()->subscribeDevStatus(nimAccountIds, SubscribeDevStatusDuration);
}

void MultiComMgr::onUpdateUserProfile(const ComGetUserProfileEvent &event)
{
    if (!m_httpOnline || !m_nimOnline) {
        return;
    }
    if (event.ret == COM_UNAUTHORIZED) {
        maintianWanDev(event.ret);
    } else if (event.ret != COM_OK) {
        m_wanDevMaintainThd->setUpdateUserProfile();
    } else {
        QueueEvent(event.Clone());
    }
}

void MultiComMgr::onConnectionReady(const ComConnectionReadyEvent &event)
{
    com_dev_data_t &devData = m_datMap.at(event.id);
    devData.devProduct = event.devProduct;
    devData.devDetail = event.devDetail;
    devData.wanDevInfo.status = "offline";
    m_readyIdSet.insert(event.id);
    if (devData.connectMode == COM_CONNECT_WAN && m_httpOnline && m_nimOnline) {
        ComCommandPtr commandPtr(new ComSendUpdateDetail);
        m_ptrMap.left.at(event.id)->putCommand(commandPtr, 1, true);
    }
    QueueEvent(event.Clone());

    const char *name = m_datMap.at(event.id).devDetail->name;
    const std::string &serialNumber = m_ptrMap.left.at(event.id)->serialNumber();
    BOOST_LOG_TRIVIAL(info) << name << ", " << serialNumber << ", connection_ready";
    BOOST_LOG_TRIVIAL(info) << "devices count: " << m_readyIdSet.size();
}

void MultiComMgr::onConnectionExit(const ComConnectionExitEvent &event)
{
    if (m_readyIdSet.find(event.id) != m_readyIdSet.end()) {
        const char *name = m_datMap.at(event.id).devDetail->name;
        const std::string &serialNumber = m_ptrMap.left.at(event.id)->serialNumber();
        BOOST_LOG_TRIVIAL(info) << "devices count: " << m_readyIdSet.size();
        BOOST_LOG_TRIVIAL(info) << name << ", " << serialNumber << ", connection_exit";
    }
    ComConnection *comConnection = m_ptrMap.left.at(event.id);
    comConnection->joinThread();
    com_dev_data_t &devData = m_datMap.at(event.id);
    m_networkIntfc->freeDevProduct(devData.devProduct);
    m_networkIntfc->freeDevDetail(devData.devDetail);
    m_networkIntfc->freeGcodeList(devData.lanGcodeList.gcodeDatas, devData.lanGcodeList.gcodeCnt);
    m_networkIntfc->freeGcodeList(devData.wanGcodeList.gcodeDatas, devData.wanGcodeList.gcodeCnt);
    m_readyIdSet.erase(event.id);
    if (comConnection->connectMode() == COM_CONNECT_WAN) {
        m_devNimAccountIdMap.erase(devData.wanDevInfo.nimAccountId);
    }
    m_datMap.erase(event.id);
    m_ptrMap.left.erase(event.id);
    m_comPtrs.remove_if([comConnection](auto &ptr) { return ptr.get() == comConnection; });
    QueueEvent(event.Clone());
}

void MultiComMgr::onDevDetailUpdate(const ComDevDetailUpdateEvent &event)
{
    fnet_dev_detail_t *&devDetail = m_datMap.at(event.id).devDetail;
    m_networkIntfc->freeDevDetail(devDetail);
    devDetail = event.devDetail;
    if (m_readyIdSet.find(event.id) != m_readyIdSet.end()) {
        QueueEvent(event.Clone());
    }
    updateWanDevInfo(event.id, devDetail->name, devDetail->status, devDetail->location);
}

void MultiComMgr::onSendUpdateDetailFailed(const ComSendUpdateDetailFailedEvent &event)
{
    if (event.ret != COM_NIM_SEND_ERROR || !m_httpOnline || !m_nimOnline) {
        return;
    }
    boost::asio::post(*m_threadPool, [this, id = event.id]() {
        if (!m_threadExitEvent.waitTrue(5000)) {
            ComCommandPtr commandPtr(new ComSendUpdateDetail);
            m_ptrMap.left.at(id)->putCommand(commandPtr, 1, true);
        }
    });
}

void MultiComMgr::onGetDevGcodeList(const ComGetDevGcodeListEvent &event)
{
    com_dev_data_t &devData = m_datMap.at(event.id);
    if (event.lanGcodeList.gcodeCnt != 0) {
        m_networkIntfc->freeGcodeList(devData.lanGcodeList.gcodeDatas, devData.lanGcodeList.gcodeCnt);
        devData.lanGcodeList = event.lanGcodeList;
    }
    if (event.wanGcodeList.gcodeCnt != 0) {
        m_networkIntfc->freeGcodeList(devData.wanGcodeList.gcodeDatas, devData.wanGcodeList.gcodeCnt);
        devData.wanGcodeList = event.wanGcodeList;
    }
    QueueEvent(event.Clone());
}

void MultiComMgr::onCommandFailed(const CommandFailedEvent &event)
{
    if (!m_httpOnline || !m_nimOnline) {
        return;
    }
    if (event.fatalError || event.ret == COM_UNAUTHORIZED) {
        maintianWanDev(event.ret);
    } else if (!m_commandFailedUpdating) {
        m_commandFailedUpdating = true;
        boost::asio::post(*m_threadPool, [this]() {
            std::chrono::duration<double> duration = std_precise_clock::now() - m_commandFailedUpdateTime;
            int waitTime = 180000 - duration.count() * 1000;
            if (waitTime > 0) {
                m_threadExitEvent.waitTrue(waitTime);
            }
            if (!m_threadExitEvent.get()) {
                m_wanDevMaintainThd->setUpdateWanDev();
                m_commandFailedUpdateTime = std_precise_clock::now();
            }
            m_commandFailedUpdating = false;
        });
    }
}

void MultiComMgr::onWanConnStatus(const WanConnStatusEvent &event)
{
    if (!m_login) {
        return;
    }
    switch (event.status) {
    case FNET_CONN_STATUS_LOGINED:
        m_nimOnline = true;
        m_subscribeDevStatusTimer.Start(SubscribeDevStatusDuration - 30);
        QueueEvent(new ComWanDevMaintainEvent(COM_WAN_DEV_MAINTAIN_EVENT, true, m_httpOnline, COM_OK));
        m_wanDevMaintainThd->setUpdateUserProfile();
        m_wanDevMaintainThd->setUpdateWanDev();
        subscribeWanDevNimStatus();
        updateWanDevDetail();
        break;
    case FNET_CONN_STATUS_LOGOUT:
        maintianWanDev(COM_REPEAT_LOGIN);
        break;
    case FNET_CONN_STATUS_UNLOGIN:
        m_nimOnline = false;
        m_subscribeDevStatusTimer.Stop();
        setWanDevOffline();
        QueueEvent(new ComWanDevMaintainEvent(COM_WAN_DEV_MAINTAIN_EVENT, true, false, COM_ERROR));
        break;
    }
}

void MultiComMgr::onWanConnRead(const WanConnReadEvent &event)
{
    if (!m_httpOnline || !m_nimOnline) {
        m_networkIntfc->freeString(event.readData.nimAccountId);
        return;
    }
    auto procDevDetailUpdate = [this](const fnet_conn_read_data_t &readData) {
        auto it = m_devNimAccountIdMap.find(readData.nimAccountId);
        if (it != m_devNimAccountIdMap.end()) {
            ComDevDetailUpdateEvent devDetailUpdateEvent(COM_DEV_DETAIL_UPDATE_EVENT,
                it->second, ComInvalidCommandId, (fnet_dev_detail_t *)readData.data);
            onDevDetailUpdate(devDetailUpdateEvent);
        }
    };
    switch (event.readData.type) {
    case FNET_CONN_READ_SYNC_USER_PROFILE:
        m_wanDevMaintainThd->setUpdateUserProfile();
        break;
    case FNET_CONN_READ_SYNC_BIND_DEVICE:
    case FNET_CONN_READ_SYNC_UNBIND_DEVICE:
        m_wanDevMaintainThd->setUpdateWanDev();
        break;
    case FNET_CONN_READ_UNREGISTER_USER:
        maintianWanDev(COM_UNREGISTER_USER);
        break;
    case FNET_CONN_READ_DEVICE_DETAIL:
        procDevDetailUpdate(event.readData);
        break;
    }
    m_networkIntfc->freeString(event.readData.nimAccountId);
}

void MultiComMgr::onWanConnSubscribe(const WanConnSubscribeEvent &event)
{
    if (!m_httpOnline || !m_nimOnline) {
        return;
    }
    if (event.status == 2 || event.status == 3) {
        auto it = m_devNimAccountIdMap.find(event.nimAccountId);
        if (it != m_devNimAccountIdMap.end()) {
            m_datMap.at(it->second).wanDevInfo.status = "offline";
            if (m_readyIdSet.find(it->second) != m_readyIdSet.end()) {
                QueueEvent(new ComWanDevInfoUpdateEvent(COM_WAN_DEV_INFO_UPDATE_EVENT, it->second));
            }
        }
    }
}

void MultiComMgr::onRefreshToken(const ComRefreshTokenEvent &event)
{
    if (!m_login || event.ret != COM_OK) {
        return;
    }
    QueueEvent(event.Clone());
}

com_dev_data_t MultiComMgr::makeDevData(const fnet_wan_dev_info_t *wanDevInfo)
{
    com_dev_data_t devData;
    devData.connectMode = COM_CONNECT_WAN;
    devData.wanDevInfo.devId = wanDevInfo->devId;
    devData.wanDevInfo.name = wanDevInfo->name;
    devData.wanDevInfo.model = wanDevInfo->model;
    devData.wanDevInfo.imageUrl = wanDevInfo->imageUrl;
    devData.wanDevInfo.status = wanDevInfo->status;
    devData.wanDevInfo.location = wanDevInfo->location;
    devData.wanDevInfo.serialNumber = wanDevInfo->serialNumber;
    devData.wanDevInfo.nimAccountId = wanDevInfo->nimAccountId;
    devData.devProduct = nullptr;
    devData.devDetail = nullptr;
    devData.lanGcodeList.gcodeDatas = nullptr;
    devData.lanGcodeList.gcodeCnt = 0;
    devData.wanGcodeList.gcodeDatas = nullptr;
    devData.wanGcodeList.gcodeCnt = 0;
    memset(&devData.lanDevInfo, 0, sizeof(devData.lanDevInfo));
    return devData;
}

void MultiComMgr::maintianWanDev(ComErrno ret)
{
    BOOST_LOG_TRIVIAL(info) << "MultiComMgr::maintianWanDev " << (int)ret;
    if (ret == COM_UNREGISTER_USER || ret == COM_REPEAT_LOGIN) {
        removeWanDev();
        QueueEvent(new ComWanDevMaintainEvent(COM_WAN_DEV_MAINTAIN_EVENT, false, false, ret));
        return;
    }
    if (ret != COM_OK) {
        if (m_nimOnline && ret != COM_NIM_SEND_ERROR) {
            m_httpOnline = false;
            m_wanDevMaintainThd->setReloginHttp();
        }
        setWanDevOffline();
        QueueEvent(new ComWanDevMaintainEvent(COM_WAN_DEV_MAINTAIN_EVENT, true, false, ret));
    }
}

void MultiComMgr::setWanDevOffline()
{
    std::vector<std::string> nimAccountIds;
    for (auto comId : m_readyIdSet) {
        com_dev_data_t &devData = m_datMap.at(comId);
        if (devData.connectMode == COM_CONNECT_WAN) {
            devData.wanDevInfo.status = "offline";
            QueueEvent(new ComWanDevInfoUpdateEvent(COM_WAN_DEV_INFO_UPDATE_EVENT, comId));
        }
    }
}

void MultiComMgr::subscribeWanDevNimStatus()
{
    if (m_readyIdSet.empty()) {
        return;
    }
    std::vector<std::string> nimAccountIds;
    for (auto comId : m_readyIdSet) {
        com_dev_data_t &devData = m_datMap.at(comId);
        if (devData.connectMode == COM_CONNECT_WAN) {
            m_datMap.at(comId).wanDevInfo.nimAccountId;
        }
    }
    ComWanNimConn::inst()->subscribeDevStatus(nimAccountIds, SubscribeDevStatusDuration);
}

void MultiComMgr::updateWanDevDetail()
{
    if (!m_httpOnline || !m_nimOnline) {
        return;
    }
    std::vector<std::string> nimAccountIds;
    for (auto comId : m_readyIdSet) {
        com_dev_data_t &devData = m_datMap.at(comId);
        if (devData.connectMode == COM_CONNECT_WAN) {
            ComCommandPtr commandPtr(new ComSendUpdateDetail);
            m_ptrMap.left.at(comId)->putCommand(commandPtr, 1, true);
        }
    }
}

void MultiComMgr::updateWanDevInfo(com_id_t id, const std::string &name, const std::string &status,
    const std::string &location)
{
    com_dev_data_t &devData = m_datMap.at(id);
    if (devData.connectMode != COM_CONNECT_WAN) {
        return;
    }
    BOOST_LOG_TRIVIAL(info) << name << " status---" << status;
    devData.wanDevInfo.name = name;
    devData.wanDevInfo.status = status;
    devData.wanDevInfo.location = location;
    if (m_readyIdSet.find(id) != m_readyIdSet.end()) {
        QueueEvent(new ComWanDevInfoUpdateEvent(COM_WAN_DEV_INFO_UPDATE_EVENT, id));
    }
}

}} // namespace Slic3r::GUI
