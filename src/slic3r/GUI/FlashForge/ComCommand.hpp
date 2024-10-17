#ifndef slic3r_GUI_ComCommand_hpp_
#define slic3r_GUI_ComCommand_hpp_

#include <atomic>
#include <wx/event.h>
#include "ComWanAsyncConn.hpp"
#include "FlashNetworkIntfc.h"
#include "FreeInDestructor.h"
#include "MultiComDef.hpp"
#include "MultiComEvent.hpp"
#include "MultiComUtils.hpp"

namespace Slic3r { namespace GUI {

class ComCommand
{
public:
    ComCommand()
        : m_commandId(s_commandNum++)
    {
    }
    virtual ~ComCommand()
    {
    }
    int commandId() const
    {
        return m_commandId;
    }
    virtual bool isDup(const ComCommand *that)
    {
        return typeid(*this) == typeid(*that);
    }
    virtual ComErrno exec(fnet::FlashNetworkIntfc *networkIntfc, const std::string &ip,
        unsigned int port, const std::string &serialNumber, const std::string &checkCode) = 0;

    virtual ComErrno exec(fnet::FlashNetworkIntfc *networkIntfc, const std::string &uid,
        const std::string &accessToken, const std::string &deviceId) = 0;

protected:
    int m_commandId;
    static int s_commandNum;
};

class ComGetDevProduct : public ComCommand
{
public:
    ComGetDevProduct()
        : m_devProduct(nullptr)
    {
    }
    ComErrno exec(fnet::FlashNetworkIntfc *networkIntfc, const std::string &ip,
        unsigned int port, const std::string &serialNumber, const std::string &checkCode)
    {
        int ret = networkIntfc->getLanDevProduct(
            ip.c_str(), port, serialNumber.c_str(), checkCode.c_str(), &m_devProduct, ComTimeoutLan);
        return MultiComUtils::fnetRet2ComErrno(ret);
    }
    ComErrno exec(fnet::FlashNetworkIntfc *networkIntfc, const std::string &uid,
        const std::string &accessToken, const std::string &deviceId)
    {
        return COM_ERROR;
    }
    fnet_dev_product_t *devProduct()
    {
        return m_devProduct;
    }
 
private:
    fnet_dev_product_t *m_devProduct;
};

class ComGetDevDetail : public ComCommand
{
public:
    ComGetDevDetail()
        : m_devDetail(nullptr)
    {
    }
    ComErrno exec(fnet::FlashNetworkIntfc *networkIntfc, const std::string &ip,
        unsigned int port, const std::string &serialNumber, const std::string &checkCode)
    {
        int ret = networkIntfc->getLanDevDetail(
            ip.c_str(), port, serialNumber.c_str(), checkCode.c_str(), &m_devDetail, ComTimeoutLan);
        return MultiComUtils::fnetRet2ComErrno(ret);
    }
    ComErrno exec(fnet::FlashNetworkIntfc *networkIntfc, const std::string &uid,
        const std::string &accessToken, const std::string &deviceId)
    {
        return COM_ERROR;
    }
    fnet_dev_detail_t *devDetail()
    {
        return m_devDetail;
    }
 
private:
    fnet_dev_detail_t *m_devDetail;
};

class ComGetDevProductDetail : public ComCommand
{
public:
    ComGetDevProductDetail()
        : m_devProduct(nullptr)
        , m_devDetail(nullptr)
    {
    }
    ComErrno exec(fnet::FlashNetworkIntfc *networkIntfc, const std::string &ip,
        unsigned int port, const std::string &serialNumber, const std::string &checkCode)
    {
        return COM_ERROR;
    }
    ComErrno exec(fnet::FlashNetworkIntfc *networkIntfc, const std::string &uid,
        const std::string &accessToken, const std::string &deviceId)
    {
        int ret = networkIntfc->getWanDevProductDetail(uid.c_str(), accessToken.c_str(),
            deviceId.c_str(), &m_devProduct, &m_devDetail, ComTimeoutWan);
        return MultiComUtils::fnetRet2ComErrno(ret);
    }
    fnet_dev_product_t *devProduct()
    {
        return m_devProduct;
    }
    fnet_dev_detail_t *devDetail()
    {
        return m_devDetail;
    }
 
private:
    fnet_dev_product_t *m_devProduct;
    fnet_dev_detail_t *m_devDetail;
};

class ComGetDevGcodeList : public ComCommand
{
public:
    ComGetDevGcodeList()
    {
        m_lanGcodeList.gcodeCnt = 0;
        m_lanGcodeList.gcodeDatas = nullptr;
        m_wanGcodeList.gcodeCnt = 0;
        m_wanGcodeList.gcodeDatas = nullptr;
    }
    ComErrno exec(fnet::FlashNetworkIntfc *networkIntfc, const std::string &ip,
        unsigned int port, const std::string &serialNumber, const std::string &checkCode)
    {
        int ret = networkIntfc->getLanDevGcodeList(ip.c_str(), port, serialNumber.c_str(),
            checkCode.c_str(), &m_lanGcodeList.gcodeDatas, &m_lanGcodeList.gcodeCnt, ComTimeoutLan);
        return MultiComUtils::fnetRet2ComErrno(ret);
    }
    ComErrno exec(fnet::FlashNetworkIntfc *networkIntfc, const std::string &uid,
        const std::string &accessToken, const std::string &deviceId)
    {
        int ret = networkIntfc->getWanDevGcodeList(uid.c_str(), accessToken.c_str(),
            deviceId.c_str(), &m_wanGcodeList.gcodeDatas, &m_wanGcodeList.gcodeCnt, ComTimeoutWan);
        return MultiComUtils::fnetRet2ComErrno(ret);
    }
    const com_gcode_list_t &lanGcodeList()
    {
        return m_lanGcodeList;
    }
    const com_gcode_list_t &wanGcodeList()
    {
        return m_wanGcodeList;
    }

private:
    com_gcode_list_t m_lanGcodeList;
    com_gcode_list_t m_wanGcodeList;
};

class ComGetGcodeThumb : public ComCommand
{
public:
    ComGetGcodeThumb(const std::string &fileNameOrThumbUrl)
        : m_fileNameOrThumbUrl(fileNameOrThumbUrl)
    {
    }
    ComErrno exec(fnet::FlashNetworkIntfc *networkIntfc, const std::string &ip,
        unsigned int port, const std::string &serialNumber, const std::string &checkCode)
    {
        fnet_file_data_t *fileData;
        int fnetRet = networkIntfc->getLanDevGcodeThumb(ip.c_str(), port, serialNumber.c_str(),
            checkCode.c_str(), m_fileNameOrThumbUrl.c_str(), &fileData, 15000);
        if (fnetRet != FNET_OK) {
            return MultiComUtils::fnetRet2ComErrno(fnetRet);
        }
        fnet::FreeInDestructor freeFileData(fileData, networkIntfc->freeFileData);
        m_thumbData.assign(fileData->data, fileData->data + fileData->size);
        return COM_OK;
    }
    ComErrno exec(fnet::FlashNetworkIntfc *networkIntfc, const std::string &uid,
        const std::string &accessToken, const std::string &deviceId)
    {
        return MultiComUtils::downloadFile(m_fileNameOrThumbUrl, m_thumbData, 15000);
    }
    std::vector<char> &thumbData()
    {
        return m_thumbData;
    }

private:
    std::string m_fileNameOrThumbUrl;
    std::vector<char> m_thumbData;
};

class ComStartJob : public ComCommand
{
public:
    ComStartJob(const com_local_job_data_t &comJobData)
        : m_comJobData(comJobData)
    {
        m_materialMappings = MultiComUtils::comMaterialMappings2Fnet(m_comJobData.materialMappings);
        m_jobData.fileName = m_comJobData.fileName.c_str();
        m_jobData.printNow = m_comJobData.printNow;
        m_jobData.levelingBeforePrint = m_comJobData.levelingBeforePrint;
        m_jobData.useMatlStation = m_comJobData.useMatlStation;
        m_jobData.gcodeToolCnt = (int)m_comJobData.materialMappings.size();
        m_jobData.materialMappings = m_materialMappings.data();
    }
    ComErrno exec(fnet::FlashNetworkIntfc *networkIntfc, const std::string &ip,
        unsigned int port, const std::string &serialNumber, const std::string &checkCode)
    {
        int ret = networkIntfc->lanDevStartJob(ip.c_str(), port, serialNumber.c_str(),
            checkCode.c_str(), &m_jobData, ComTimeoutLan);
        return MultiComUtils::fnetRet2ComErrno(ret);
    }
    ComErrno exec(fnet::FlashNetworkIntfc *networkIntfc, const std::string &uid,
        const std::string &accessToken, const std::string &deviceId)
    {
        int ret = networkIntfc->wanDevStartJob(uid.c_str(), accessToken.c_str(),
            deviceId.c_str(), &m_jobData, ComTimeoutWan);
        return MultiComUtils::fnetRet2ComErrno(ret);
    }

private:
    fnet_local_job_data_t m_jobData;
    com_local_job_data_t  m_comJobData;
    std::vector<fnet_material_mapping_t> m_materialMappings;
};

class ComSendGcode : public ComCommand
{
public:
    ComSendGcode(const com_send_gcode_data_t &comSendGcodeData)
        : m_progress(0)
        , m_callbackRet(0)
        , m_comId(ComInvalidId)
        , m_evtHandler(nullptr)
        , m_comSendGcodeData(comSendGcodeData)
    {
        m_materialMappings = MultiComUtils::comMaterialMappings2Fnet(m_comSendGcodeData.materialMappings);
        m_sendGcodeData.gcodeFilePath = m_comSendGcodeData.gcodeFilePath.c_str();
        m_sendGcodeData.thumbFilePath = m_comSendGcodeData.thumbFilePath.c_str();
        m_sendGcodeData.gcodeDstName = m_comSendGcodeData.gcodeDstName.c_str();
        m_sendGcodeData.printNow = m_comSendGcodeData.printNow;
        m_sendGcodeData.levelingBeforePrint = m_comSendGcodeData.levelingBeforePrint;
        m_sendGcodeData.flowCalibration = m_comSendGcodeData.flowCalibration;
        m_sendGcodeData.useMatlStation = m_comSendGcodeData.useMatlStation;
        m_sendGcodeData.gcodeToolCnt = (int)m_comSendGcodeData.materialMappings.size();
        m_sendGcodeData.materialMappings = m_materialMappings.data();
        m_sendGcodeData.callback = callback;
        m_sendGcodeData.callbackData = this;
    }
    ComErrno exec(fnet::FlashNetworkIntfc *networkIntfc, const std::string &ip,
        unsigned int port, const std::string &serialNumber, const std::string &checkCode)
    {
        int ret = networkIntfc->lanDevSendGcode(ip.c_str(), port, serialNumber.c_str(),
            checkCode.c_str(), &m_sendGcodeData, 15000);
        return MultiComUtils::fnetRet2ComErrno(ret);
    }
    ComErrno exec(fnet::FlashNetworkIntfc *networkIntfc, const std::string &uid,
        const std::string &accessToken, const std::string &deviceId)
    {
        return COM_ERROR;
    }
    void abort()
    {
        m_callbackRet = 1;
    }
    void setConectionData(com_id_t comId, wxEvtHandler *evtHandler)
    {
        m_comId = comId;
        m_evtHandler = evtHandler;
    }

private:
    static int callback(long long now, long long total, void *callbackData)
    {
        ComSendGcode *inst = (ComSendGcode *) callbackData;
        if (total != 0) {
            double progress = (double)now / total;
            if (progress - inst->m_progress > 0.025 && inst->m_evtHandler != nullptr) {
                inst->m_evtHandler->QueueEvent(new ComSendGcodeProgressEvent(
                    COM_SEND_GCODE_PROGRESS_EVENT, inst->m_comId, inst->m_commandId, now, total));
                inst->m_progress = progress;
            }
        }
        return inst->m_callbackRet;
    }

private:
    double                  m_progress;
    std::atomic<int>        m_callbackRet;
    com_id_t                m_comId;
    wxEvtHandler           *m_evtHandler;
    fnet_send_gcode_data_t  m_sendGcodeData;
    com_send_gcode_data_t   m_comSendGcodeData;
    std::vector<fnet_material_mapping_t> m_materialMappings;
};

class ComWanAsyncCommand : public ComCommand
{
public:
    ComErrno exec(fnet::FlashNetworkIntfc *networkIntfc, const std::string &uid,
        const std::string &accessToken, const std::string &deviceId)
    {
        return COM_ERROR;
    }
    virtual void asyncExec(ComWanAsyncConn *wanAsyncConn, const std::string &devId) = 0;
};

class ComTempCtrl : public ComWanAsyncCommand
{
public:
    ComTempCtrl(double platformTemp, double rightTemp, double leftTemp, double chamberTemp)
    {
        m_tempCtrl.platformTemp = platformTemp;
        m_tempCtrl.rightTemp = rightTemp;
        m_tempCtrl.leftTemp = leftTemp;
        m_tempCtrl.chamberTemp = chamberTemp;
    }
    ComErrno exec(fnet::FlashNetworkIntfc *networkIntfc, const std::string &ip,
        unsigned int port, const std::string &serialNumber, const std::string &checkCode)
    {
        int ret = networkIntfc->ctrlLanDevTemp(ip.c_str(), port, serialNumber.c_str(),
            checkCode.c_str(), &m_tempCtrl, ComTimeoutLan);
        return MultiComUtils::fnetRet2ComErrno(ret);
    }
    void asyncExec(ComWanAsyncConn *wanAsyncConn, const std::string &devId)
    {
        wanAsyncConn->postTempCtrl(devId, m_tempCtrl);
    }

private:
    fnet_temp_ctrl_t m_tempCtrl;
};

class ComLightCtrl : public ComWanAsyncCommand
{
public:
    ComLightCtrl(const std::string &lightStatus)
        : m_lightStatus(lightStatus)
    {
        m_lightCtrl.lightStatus = m_lightStatus.c_str();
    }
    ComErrno exec(fnet::FlashNetworkIntfc *networkIntfc, const std::string &ip,
        unsigned int port, const std::string &serialNumber, const std::string &checkCode)
    {
        int ret = networkIntfc->ctrlLanDevLight(ip.c_str(), port, serialNumber.c_str(),
            checkCode.c_str(), &m_lightCtrl, ComTimeoutLan);
        return MultiComUtils::fnetRet2ComErrno(ret);
    }
    void asyncExec(ComWanAsyncConn *wanAsyncConn, const std::string &devId)
    {
        wanAsyncConn->postLightCtrl(devId, m_lightCtrl);
    }

private:
    std::string m_lightStatus;
    fnet_light_ctrl_t m_lightCtrl;
};

class ComAirFilterCtrl : public ComWanAsyncCommand
{
public:
    ComAirFilterCtrl(const std::string &internalFanStatus, const std::string &externalFanStatus)
        : m_internalFanStatus(internalFanStatus)
        , m_externalFanStatus(externalFanStatus)
    {
        m_airFilterCtrl.internalFanStatus = m_internalFanStatus.c_str();
        m_airFilterCtrl.externalFanStatus = m_externalFanStatus.c_str();
    }
    ComErrno exec(fnet::FlashNetworkIntfc *networkIntfc, const std::string &ip,
        unsigned int port, const std::string &serialNumber, const std::string &checkCode)
    {
        int ret = networkIntfc->ctrlLanDevAirFilter(ip.c_str(), port, serialNumber.c_str(),
            checkCode.c_str(), &m_airFilterCtrl, ComTimeoutLan);
        return MultiComUtils::fnetRet2ComErrno(ret);
    }
    void asyncExec(ComWanAsyncConn *wanAsyncConn, const std::string &devId)
    {
        wanAsyncConn->postAirFilterCtrl(devId, m_airFilterCtrl);
    }

private:
    std::string m_internalFanStatus;
    std::string m_externalFanStatus;
    fnet_air_filter_ctrl_t m_airFilterCtrl;
};

class ComClearFanCtrl : public ComWanAsyncCommand
{
public:
    ComClearFanCtrl(const std::string &clearFanStatus)
        : m_clearFanStatus(clearFanStatus)
    {
        m_clearFanCtrl.clearFanStatus = m_clearFanStatus.c_str();
    }
    ComErrno exec(fnet::FlashNetworkIntfc *networkIntfc, const std::string &ip,
        unsigned int port, const std::string &serialNumber, const std::string &checkCode)
    {
        int ret = networkIntfc->ctrlLanDevClearFan(ip.c_str(), port, serialNumber.c_str(),
            checkCode.c_str(), &m_clearFanCtrl, ComTimeoutLan);
        return MultiComUtils::fnetRet2ComErrno(ret);
    }
    void asyncExec(ComWanAsyncConn *wanAsyncConn, const std::string &devId)
    {
        wanAsyncConn->postClearFanCtrl(devId, m_clearFanCtrl);
    }

private:
    std::string m_clearFanStatus;
    fnet_clear_fan_ctrl_t m_clearFanCtrl;
};

class ComMatlStationCtrl : public ComWanAsyncCommand
{
public:
    ComMatlStationCtrl(int slotId, int action)
    {
        m_matlStationCtrl.slotId = slotId;
        m_matlStationCtrl.action = action;
    }
    ComErrno exec(fnet::FlashNetworkIntfc *networkIntfc, const std::string &ip,
        unsigned int port, const std::string &serialNumber, const std::string &checkCode)
    {
        int ret = networkIntfc->ctrlLanDevMatlStation(ip.c_str(), port, serialNumber.c_str(),
            checkCode.c_str(), &m_matlStationCtrl, ComTimeoutLan);
        return MultiComUtils::fnetRet2ComErrno(ret);
    }
    void asyncExec(ComWanAsyncConn *wanAsyncConn, const std::string &devId)
    {
        wanAsyncConn->postMatlStationCtrl(devId, m_matlStationCtrl);
    }

private:
    fnet_matl_station_ctrl_t m_matlStationCtrl;
};

class ComIndepMatlCtrl : public ComWanAsyncCommand
{
public:
    ComIndepMatlCtrl(int action)
    {
        m_indepMatlCtrl.action = action;
    }
    ComErrno exec(fnet::FlashNetworkIntfc *networkIntfc, const std::string &ip,
        unsigned int port, const std::string &serialNumber, const std::string &checkCode)
    {
        int ret = networkIntfc->ctrlLanDevIndepMatl(ip.c_str(), port, serialNumber.c_str(),
            checkCode.c_str(), &m_indepMatlCtrl, ComTimeoutLan);
        return MultiComUtils::fnetRet2ComErrno(ret);
    }
    void asyncExec(ComWanAsyncConn *wanAsyncConn, const std::string &devId)
    {
        wanAsyncConn->postIndepMatlCtrl(devId, m_indepMatlCtrl);
    }

private:
    fnet_indep_matl_ctrl_t m_indepMatlCtrl;
};

class ComPrintCtrl : public ComWanAsyncCommand
{
public:
    ComPrintCtrl(double zAxisCompensation, double printSpeedAdjust, double coolingFanSpeed,
        double coolingFanLeftSpeed, double chamberFanSpeed)
    {
        m_printCtrl.zAxisCompensation = zAxisCompensation;
        m_printCtrl.printSpeedAdjust = printSpeedAdjust;
        m_printCtrl.coolingFanSpeed = coolingFanSpeed;
        m_printCtrl.coolingFanLeftSpeed = coolingFanLeftSpeed;
        m_printCtrl.chamberFanSpeed = chamberFanSpeed;
    }
    ComErrno exec(fnet::FlashNetworkIntfc *networkIntfc, const std::string &ip,
        unsigned int port, const std::string &serialNumber, const std::string &checkCode)
    {
        int ret = networkIntfc->ctrlLanDevPrint(ip.c_str(), port, serialNumber.c_str(),
            checkCode.c_str(), &m_printCtrl, ComTimeoutLan);
        return MultiComUtils::fnetRet2ComErrno(ret);
    }
    void asyncExec(ComWanAsyncConn *wanAsyncConn, const std::string &devId)
    {
        wanAsyncConn->postPrintCtrl(devId, m_printCtrl);
    }

private:
    fnet_print_ctrl_t m_printCtrl;
};

class ComJobCtrl : public ComWanAsyncCommand
{
public:
    ComJobCtrl(const std::string &jobId, const std::string &action)
        : m_jobId(jobId)
        , m_action(action)
    {
        m_jobCtrl.jobId = m_jobId.c_str();
        m_jobCtrl.action = m_action.c_str();
    }
    ComErrno exec(fnet::FlashNetworkIntfc *networkIntfc, const std::string &ip,
        unsigned int port, const std::string &serialNumber, const std::string &checkCode)
    {
        int ret = networkIntfc->ctrlLanDevJob(ip.c_str(), port, serialNumber.c_str(),
            checkCode.c_str(), &m_jobCtrl, ComTimeoutLan);
        return MultiComUtils::fnetRet2ComErrno(ret);
    }
    void asyncExec(ComWanAsyncConn *wanAsyncConn, const std::string &devId)
    {
        wanAsyncConn->postJobCtrl(devId, m_jobCtrl);
    }

private:
    std::string m_jobId;
    std::string m_action;
    fnet_job_ctrl_t m_jobCtrl;
};

class ComStateCtrl : public ComWanAsyncCommand
{
public:
    ComStateCtrl(const std::string &action)
        : m_action(action)
    {
        m_stateCtrl.action = m_action.c_str();
    }
    ComErrno exec(fnet::FlashNetworkIntfc *networkIntfc, const std::string &ip,
        unsigned int port, const std::string &serialNumber, const std::string &checkCode)
    {
        int ret = networkIntfc->ctrlLanDevState(ip.c_str(), port, serialNumber.c_str(),
            checkCode.c_str(), &m_stateCtrl, ComTimeoutLan);
        return MultiComUtils::fnetRet2ComErrno(ret);
    }
    void asyncExec(ComWanAsyncConn *wanAsyncConn, const std::string &devId)
    {
        wanAsyncConn->postStateCtrl(devId, m_stateCtrl);
    }

private:
    std::string m_action;
    fnet_state_ctrl_t m_stateCtrl;
};

class ComCameraStreamCtrl : public ComWanAsyncCommand
{
public:
    ComCameraStreamCtrl(const std::string &action)
        : m_action(action)
    {
        m_cameraStreamCtrl.action = m_action.c_str();
    }
    ComErrno exec(fnet::FlashNetworkIntfc *networkIntfc, const std::string &ip,
        unsigned int port, const std::string &serialNumber, const std::string &checkCode)
    {
        return COM_OK;
    }
    void asyncExec(ComWanAsyncConn *wanAsyncConn, const std::string &devId)
    {
        wanAsyncConn->postCameraStreamCtrl(devId, m_cameraStreamCtrl);
    }

private:
    std::string m_action;
    fnet_camera_stream_ctrl_t m_cameraStreamCtrl;
};

class ComMatlStationConfig : public ComWanAsyncCommand
{
public:
    ComMatlStationConfig(int slotId, const std::string &materialName, const std::string &materialColor)
        : m_materialName(materialName)
        , m_materialColor(materialColor)
    {
        m_matlStationConfig.slotId = slotId;
        m_matlStationConfig.materialName = m_materialName.c_str();
        m_matlStationConfig.materialColor = m_materialColor.c_str();
    }
    ComErrno exec(fnet::FlashNetworkIntfc *networkIntfc, const std::string &ip,
        unsigned int port, const std::string &serialNumber, const std::string &checkCode)
    {
        int ret = networkIntfc->configLanDevMatlStation(ip.c_str(), port, serialNumber.c_str(),
            checkCode.c_str(), &m_matlStationConfig, ComTimeoutLan);
        return MultiComUtils::fnetRet2ComErrno(ret);
    }
    void asyncExec(ComWanAsyncConn *wanAsyncConn, const std::string &devId)
    {
        wanAsyncConn->postMatlStationConfig(devId, m_matlStationConfig);
    }

private:
    std::string m_materialName;
    std::string m_materialColor;
    fnet_matl_station_config_t m_matlStationConfig;
};

class ComIndepMatlConfig : public ComWanAsyncCommand
{
public:
    ComIndepMatlConfig(const std::string &materialName, const std::string &materialColor)
        : m_materialName(materialName)
        , m_materialColor(materialColor)
    {
        m_indepMatlConfig.materialName = m_materialName.c_str();
        m_indepMatlConfig.materialColor = m_materialColor.c_str();
    }
    ComErrno exec(fnet::FlashNetworkIntfc *networkIntfc, const std::string &ip,
        unsigned int port, const std::string &serialNumber, const std::string &checkCode)
    {
        int ret = networkIntfc->configLanDevIndepMatl(ip.c_str(), port, serialNumber.c_str(),
            checkCode.c_str(), &m_indepMatlConfig, ComTimeoutLan);
        return MultiComUtils::fnetRet2ComErrno(ret);
    }
    void asyncExec(ComWanAsyncConn *wanAsyncConn, const std::string &devId)
    {
        wanAsyncConn->postIndepMatlConfig(devId, m_indepMatlConfig);
    }

private:
    std::string m_materialName;
    std::string m_materialColor;
    fnet_indep_matl_config_t m_indepMatlConfig;
};

}} // namespace Slic3r::GUI

#endif
