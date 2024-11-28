#include "ComWanNimConn.hpp"
#include "MultiComUtils.hpp"

namespace Slic3r { namespace GUI {

wxDEFINE_EVENT(WAN_CONN_STATUS_EVENT, WanConnStatusEvent);
wxDEFINE_EVENT(WAN_CONN_READ_EVENT, WanConnReadEvent);
wxDEFINE_EVENT(WAN_CONN_SUBSCRIBE_EVENT, WanConnSubscribeEvent);

ComWanNimConn::ComWanNimConn()
    : m_networkIntfc(nullptr)
    , m_isInitalizeNim(false)
    , m_conn(nullptr)
{
}

void ComWanNimConn::initalize(fnet::FlashNetworkIntfc *networkIntfc, const char *nimAppDir)
{
    m_networkIntfc = networkIntfc;
    m_nimAppDir = nimAppDir;
}

void ComWanNimConn::uninitalize()
{
    if (m_isInitalizeNim) {
        m_networkIntfc->uninitlizeNim();
        m_isInitalizeNim = false;
    }
}

ComErrno ComWanNimConn::createConn(const char *nimAppKey, const char *nimAccount, const char *nimToken)
{
    if (m_networkIntfc == nullptr) {
        return COM_ERROR;
    }
    if (!m_isInitalizeNim) {
        if (m_networkIntfc->initlizeNim(nimAppKey, m_nimAppDir.c_str()) != FNET_OK) {
            return COM_ERROR;
        }
        m_isInitalizeNim = true;
    }
    m_threadPool.reset(new boost::asio::thread_pool);
    void *conn;
    fnet_conn_settings_t settings;
    settings.nimAccount = nimAccount;
    settings.nimToken = nimToken;
    settings.statusCallback = statusCallback;
    settings.statusCallbackData = this;
    settings.readCallback = readCallback;
    settings.readCallbackData = this;
    settings.subscribeCallback = subscribeCallback;
    settings.subscribeCallbackData = this;
    ComErrno ret = MultiComUtils::fnetRet2ComErrno(m_networkIntfc->createConnection(&conn, &settings));
    if (ret != COM_OK) {
        return ret;
    }
    boost::unique_lock<boost::shared_mutex> lock(m_connMutex);
    m_conn = conn;
    return COM_OK;
}

void ComWanNimConn::freeConn()
{
    boost::unique_lock<boost::shared_mutex> lock(m_connMutex);
    if (m_conn != nullptr) {
        m_threadPool.reset();
        m_networkIntfc->freeConnection(m_conn);
        m_conn = nullptr;
    }
}

void ComWanNimConn::syncBindDev(const std::string &nimAccountId)
{
    boost::asio::post(*m_threadPool, [this, nimAccountId]() {
        boost::shared_lock<boost::shared_mutex> lock(m_connMutex);
        if (m_conn == nullptr) {
            return;
        }
        fnet_conn_write_data_t writeData = { FNET_CONN_WRITE_SYNC_BIND_DEVICE, nullptr };
        writeData.nimAccountId = nimAccountId.c_str();
        m_networkIntfc->connectionSend(m_conn, &writeData);
    });
}

void ComWanNimConn::syncUnbindDev(const std::string &nimAccountId)
{
    boost::asio::post(*m_threadPool, [this, nimAccountId]() {
        boost::shared_lock<boost::shared_mutex> lock(m_connMutex);
        if (m_conn == nullptr) {
            return;
        }
        fnet_conn_write_data_t writeData = { FNET_CONN_WRITE_SYNC_UNBIND_DEVICE, nullptr };
        writeData.nimAccountId = nimAccountId.c_str();
        m_networkIntfc->connectionSend(m_conn, &writeData);
    });
}

void ComWanNimConn::subscribeDevStatus(const std::vector<std::string> &nimAcctountIds, int duration)
{
    boost::asio::post(*m_threadPool, [this, nimAcctountIds, duration]() {
        boost::shared_lock<boost::shared_mutex> lock(m_connMutex);
        if (m_conn == nullptr) {
            return;
        }
        std::vector<const char *> nimAccountIdPtrs;
        for (size_t i = 0; i < nimAcctountIds.size(); i += 100) {
            for (size_t j = 0; j < 100 && i + j < nimAcctountIds.size(); ++j) {
                nimAccountIdPtrs.push_back(nimAcctountIds[i].c_str());
            }
            fnet_conn_subscribe_data_t subscribeData;
            subscribeData.nimAccountIds = nimAccountIdPtrs.data();
            subscribeData.accountCnt = nimAccountIdPtrs.size();
            subscribeData.duration = duration;
            subscribeData.immediateSync = 1;
            m_networkIntfc->connectionSubscribe(m_conn, &subscribeData);
            nimAccountIdPtrs.clear();
        }
    });
}

ComErrno ComWanNimConn::sendStartJob(const char *nimAccountId, const fnet_local_job_data_t &jobData)
{
    boost::shared_lock<boost::shared_mutex> lock(m_connMutex);
    if (m_conn == nullptr) {
        return COM_ERROR;
    }
    fnet_conn_write_data_t writeData = { FNET_CONN_WRITE_START_JOB, &jobData };
    writeData.nimAccountId = nimAccountId;
    return MultiComUtils::fnetRet2ComErrno(m_networkIntfc->connectionSend(m_conn, &writeData));
}

ComErrno ComWanNimConn::sendStartCloundJob(const char *nimAccountId, const fnet_clound_job_data_t &jobData)
{
    boost::shared_lock<boost::shared_mutex> lock(m_connMutex);
    if (m_conn == nullptr) {
        return COM_ERROR;
    }
    fnet_conn_write_data_t writeData = { FNET_CONN_WRITE_START_CLOUND_JOB, &jobData };
    writeData.nimAccountId = nimAccountId;
    return MultiComUtils::fnetRet2ComErrno(m_networkIntfc->connectionSend(m_conn, &writeData));
}

ComErrno ComWanNimConn::sendTempCtrl(const char *nimAccountId, const fnet_temp_ctrl_t &tempCtrl)
{
    boost::shared_lock<boost::shared_mutex> lock(m_connMutex);
    if (m_conn == nullptr) {
        return COM_ERROR;
    }
    fnet_conn_write_data_t writeData = { FNET_CONN_WRITE_TEMP_CTRL, &tempCtrl };
    writeData.nimAccountId = nimAccountId;
    return MultiComUtils::fnetRet2ComErrno(m_networkIntfc->connectionSend(m_conn, &writeData));
}

ComErrno ComWanNimConn::sendLightCtrl(const char *nimAccountId, const fnet_light_ctrl_t &lightCtrl)
{
    boost::shared_lock<boost::shared_mutex> lock(m_connMutex);
    if (m_conn == nullptr) {
        return COM_ERROR;
    }
    fnet_conn_write_data_t writeData = { FNET_CONN_WRITE_LIGHT_CTRL, &lightCtrl };
    writeData.nimAccountId = nimAccountId;
    return MultiComUtils::fnetRet2ComErrno(m_networkIntfc->connectionSend(m_conn, &writeData));
}

ComErrno ComWanNimConn::sendAirFilterCtrl(const char *nimAccountId,
    const fnet_air_filter_ctrl_t &airFilterCtrl)
{
    boost::shared_lock<boost::shared_mutex> lock(m_connMutex);
    if (m_conn == nullptr) {
        return COM_ERROR;
    }
    fnet_conn_write_data_t writeData = { FNET_CONN_WRITE_AIR_FILTER_CTRL, &airFilterCtrl };
    writeData.nimAccountId = nimAccountId;
    return MultiComUtils::fnetRet2ComErrno(m_networkIntfc->connectionSend(m_conn, &writeData));
}

ComErrno ComWanNimConn::sendClearFanCtrl(const char *nimAccountId,
    const fnet_clear_fan_ctrl_t &clearFanCtrl)
{
    boost::shared_lock<boost::shared_mutex> lock(m_connMutex);
    if (m_conn == nullptr) {
        return COM_ERROR;
    }
    fnet_conn_write_data_t writeData = { FNET_CONN_WRITE_CLEAR_FAN_CTRL, &clearFanCtrl };
    writeData.nimAccountId = nimAccountId;
    return MultiComUtils::fnetRet2ComErrno(m_networkIntfc->connectionSend(m_conn, &writeData));
}

ComErrno ComWanNimConn::sendMatlStationCtrl(const char *nimAccountId,
    const fnet_matl_station_ctrl_t &matlStationCtrl)
{
    boost::shared_lock<boost::shared_mutex> lock(m_connMutex);
    if (m_conn == nullptr) {
        return COM_ERROR;
    }
    fnet_conn_write_data_t writeData = { FNET_CONN_WRITE_MATL_STATION_CTRL, &matlStationCtrl };
    writeData.nimAccountId = nimAccountId;
    return MultiComUtils::fnetRet2ComErrno(m_networkIntfc->connectionSend(m_conn, &writeData));
}

ComErrno ComWanNimConn::sendIndepMatlCtrl(const char *nimAccountId,
    const fnet_indep_matl_ctrl_t &indepMatlCtrl)
{
    boost::shared_lock<boost::shared_mutex> lock(m_connMutex);
    if (m_conn == nullptr) {
        return COM_ERROR;
    }
    fnet_conn_write_data_t writeData = { FNET_CONN_WRITE_INDEP_MATL_CTRL, &indepMatlCtrl };
    writeData.nimAccountId = nimAccountId;
    return MultiComUtils::fnetRet2ComErrno(m_networkIntfc->connectionSend(m_conn, &writeData));
}

ComErrno ComWanNimConn::sendPrintCtrl(const char *nimAccountId, const fnet_print_ctrl_t &printCtrl)
{
    boost::shared_lock<boost::shared_mutex> lock(m_connMutex);
    if (m_conn == nullptr) {
        return COM_ERROR;
    }
    fnet_conn_write_data_t writeData = { FNET_CONN_WRITE_PRINT_CTRL, &printCtrl };
    writeData.nimAccountId = nimAccountId;
    return MultiComUtils::fnetRet2ComErrno(m_networkIntfc->connectionSend(m_conn, &writeData));
}

ComErrno ComWanNimConn::sendJobCtrl(const char *nimAccountId, const fnet_job_ctrl_t &jobCtrl)
{
    boost::shared_lock<boost::shared_mutex> lock(m_connMutex);
    if (m_conn == nullptr) {
        return COM_ERROR;
    }
    fnet_conn_write_data_t writeData = { FNET_CONN_WRITE_JOB_CTRL, &jobCtrl };
    writeData.nimAccountId = nimAccountId;
    return MultiComUtils::fnetRet2ComErrno(m_networkIntfc->connectionSend(m_conn, &writeData));
}

ComErrno ComWanNimConn::sendStateCtrl(const char *nimAccountId, const fnet_state_ctrl_t &stateCtrl)
{
    boost::shared_lock<boost::shared_mutex> lock(m_connMutex);
    if (m_conn == nullptr) {
        return COM_ERROR;
    }
    fnet_conn_write_data_t writeData = { FNET_CONN_WRITE_STATE_CTRL, &stateCtrl };
    writeData.nimAccountId = nimAccountId;
    return MultiComUtils::fnetRet2ComErrno(m_networkIntfc->connectionSend(m_conn, &writeData));
}

ComErrno ComWanNimConn::sendCameraStreamCtrl(const char *nimAccountId,
    const fnet_camera_stream_ctrl_t &cameraStreamCtrl)
{
    boost::shared_lock<boost::shared_mutex> lock(m_connMutex);
    if (m_conn == nullptr) {
        return COM_ERROR;
    }
    fnet_conn_write_data_t writeData = {FNET_CONN_WRITE_CAMERA_STREAM_CTRL, &cameraStreamCtrl};
    writeData.nimAccountId = nimAccountId;
    return MultiComUtils::fnetRet2ComErrno(m_networkIntfc->connectionSend(m_conn, &writeData));
}

ComErrno ComWanNimConn::sendMatlStationConfig(const char *nimAccountId,
    const fnet_matl_station_config_t &matlStationConfig)
{
    boost::shared_lock<boost::shared_mutex> lock(m_connMutex);
    if (m_conn == nullptr) {
        return COM_ERROR;
    }
    fnet_conn_write_data_t writeData = { FNET_CONN_WRITE_MATL_STATION_CONFIG, &matlStationConfig };
    writeData.nimAccountId = nimAccountId;
    return MultiComUtils::fnetRet2ComErrno(m_networkIntfc->connectionSend(m_conn, &writeData));
}

ComErrno ComWanNimConn::sendIndepMatlConfig(const char *nimAccountId,
    const fnet_indep_matl_config_t &indepMatlConfig)
{
    boost::shared_lock<boost::shared_mutex> lock(m_connMutex);
    if (m_conn == nullptr) {
        return COM_ERROR;
    }
    fnet_conn_write_data_t writeData = { FNET_CONN_WRITE_INDEP_MATL_CONFIG, &indepMatlConfig };
    writeData.nimAccountId = nimAccountId;
    return MultiComUtils::fnetRet2ComErrno(m_networkIntfc->connectionSend(m_conn, &writeData));
}

void ComWanNimConn::statusCallback(fnet_conn_status_t status, void *data)
{
    WanConnStatusEvent *event = new WanConnStatusEvent;
    event->SetEventType(WAN_CONN_STATUS_EVENT);
    event->status = status;
    ((ComWanNimConn *)data)->QueueEvent(event);
}

void ComWanNimConn::readCallback(fnet_conn_read_data_t *readData, void *data)
{
    WanConnReadEvent *event = new WanConnReadEvent;
    event->SetEventType(WAN_CONN_READ_EVENT);
    event->readData = *readData;
    ((ComWanNimConn *)data)->QueueEvent(event);
}

void ComWanNimConn::subscribeCallback(const char *nimAccountId, unsigned int status, void *data)
{
    WanConnSubscribeEvent *event = new WanConnSubscribeEvent;
    event->SetEventType(WAN_CONN_SUBSCRIBE_EVENT);
    event->nimAccountId = nimAccountId;
    event->status = status;
    ((ComWanNimConn *)data)->QueueEvent(event);
}

}} // namespace Slic3r::GUI
