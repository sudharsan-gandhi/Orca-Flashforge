#include "WanDevSendGcodeThd.hpp"
#include <algorithm>
#include <fstream>
#include <iterator>
#include <boost/algorithm/hex.hpp>
#include <openssl/md5.h>
#include "ComCommand.hpp"
#include "ComWanNimConn.hpp"
#include "FreeInDestructor.h"
#include "MultiComUtils.hpp"
#include "WanDevTokenMgr.hpp"

namespace Slic3r { namespace GUI {

WanDevSendGcodeThd::WanDevSendGcodeThd(fnet::FlashNetworkIntfc *networkIntfc)
    : m_exitThread(false)
    , m_thread(boost::bind(&WanDevSendGcodeThd::run, this))
    , m_networkIntfc(networkIntfc)
{
}

void WanDevSendGcodeThd::exit()
{
    abortSendGcode();
    m_exitThread = true;
    m_sendGcodeEvent.set(true);
    m_thread.join();
}

bool WanDevSendGcodeThd::startSendGcode(const std::string &uid, const std::vector<std::string> &devIds,
    const std::vector<std::string> &serialNumbers, const std::string &teamId,
    const std::vector<std::string> &nimAccountIds, const com_send_gcode_data_t &sendGocdeData)
{
    if (m_sendGcodeEvent.get()) {
        return false;
    }
    m_uid = uid;
    m_devIds = devIds;
    m_nimTeamId = teamId;
    m_serialNumberMap.clear();
    m_nimAccountIdMap.clear();
    for (size_t i = 0; i < devIds.size(); ++i) {
        m_serialNumberMap.emplace(devIds[i], serialNumbers[i]);
        m_nimAccountIdMap.emplace(devIds[i], nimAccountIds[i]);
    }
    m_comSendGcodeData = sendGocdeData;
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
    m_progress = 0.0;
    m_callbackRet = 0;
    m_sendGcodeEvent.set(true);
    return true;
}

bool WanDevSendGcodeThd::abortSendGcode()
{
    if (!m_sendGcodeEvent.get()) {
        return false;
    }
    m_callbackRet = 1;
    return true;
}

void WanDevSendGcodeThd::run()
{
    while (!m_exitThread) {
        m_sendGcodeEvent.waitTrue();
        if (!m_exitThread) {
            ScopedWanDevToken token = WanDevTokenMgr::inst()->getScopedToken();
            const char *accessToken = token.accessToken().c_str();

            fnet_clound_gcode_data_t *cloundGcodeData;
            int fnetRet = m_networkIntfc->wanDevSendGcodeClound(
                m_uid.c_str(), accessToken, &m_sendGcodeData, &cloundGcodeData, 15000);
            fnet::FreeInDestructor freeCloundGcodeData(
                cloundGcodeData, m_networkIntfc->freeCloundGcodeData);

            std::map<std::string, ComCloundJobErrno> errorMap;
            if (fnetRet == FNET_OK) {
                fnetRet = startCloundJob(accessToken, cloundGcodeData, errorMap);
            }
            ComErrno ret = MultiComUtils::fnetRet2ComErrno(fnetRet);
            QueueEvent(new ComSendGcodeFinishEvent(COM_SEND_GCODE_FINISH_EVENT, errorMap, ret));
        }
        m_sendGcodeEvent.set(false);
    }
}

int WanDevSendGcodeThd::startCloundJob(const char *accessToken, const fnet_clound_gcode_data_t *cloundGcodeData,
    std::map<std::string, ComCloundJobErrno> &errorMap)
{
    std::vector<const char *> devIds;
    for (auto &devId : m_devIds) {
        devIds.push_back(devId.c_str());
    }
    boost::filesystem::path gcodeDstPath(m_sendGcodeData.gcodeDstName);
    boost::filesystem::path gcodePath(m_sendGcodeData.gcodeFilePath);
    boost::filesystem::path thumbPath(m_sendGcodeData.thumbFilePath);
    std::string gcodeType = gcodePath.extension().string().substr(1);
    std::string gcodeMd5 = getFileMd5(m_sendGcodeData.gcodeFilePath);
    std::string thumbName = gcodeDstPath.replace_extension(".").string() + gcodeType;
    std::string thumbType = thumbPath.extension().string().substr(1);
    std::string thumbMd5 = getFileMd5(m_sendGcodeData.thumbFilePath);

    fnet_clound_job_data_t jobData;
    jobData.devIds = devIds.data();
    jobData.devSerialNumbers = nullptr;
    jobData.jobIds = nullptr;
    jobData.devCnt = devIds.size();
    jobData.gcodeName = m_sendGcodeData.gcodeDstName;
    jobData.gcodeType = gcodeType.c_str();
    jobData.gcodeMd5 = gcodeMd5.c_str();
    jobData.gcodeSize = boost::filesystem::file_size(m_sendGcodeData.gcodeFilePath);
    jobData.thumbName = thumbName.c_str();
    jobData.thumbType = thumbType.c_str();
    jobData.thumbMd5 = thumbMd5.c_str();
    jobData.thumbSize = boost::filesystem::file_size(m_sendGcodeData.thumbFilePath);
    jobData.bucketName = cloundGcodeData->bucketName;
    jobData.endpoint = cloundGcodeData->endpoint;
    jobData.gcodeStorageKey = cloundGcodeData->gcodeStorageKey;
    jobData.gcodeStorageUrl = cloundGcodeData->gcodeStorageUrl;
    jobData.thumbStorageKey = cloundGcodeData->thumbStorageKey;
    jobData.thumbStorageUrl = cloundGcodeData->thumbStorageUrl;
    jobData.printNow = m_sendGcodeData.printNow;
    jobData.levelingBeforePrint = m_sendGcodeData.levelingBeforePrint;
    jobData.flowCalibration = m_sendGcodeData.flowCalibration;
    jobData.useMatlStation = m_sendGcodeData.useMatlStation;
    jobData.gcodeToolCnt = m_sendGcodeData.gcodeToolCnt;
    jobData.materialMappings = m_sendGcodeData.materialMappings;

    fnet_add_clound_job_result_t *results;
    int resultCnt;
    int fnetRet = m_networkIntfc->wanDevAddCloundJob(
        m_uid.c_str(), accessToken, &jobData, &results, &resultCnt, ComTimeoutWan);
    if (fnetRet != FNET_OK) {
        return fnetRet;
    }
    fnet::FreeInDestructorArg freeResults(results, m_networkIntfc->freeAddCloudJobResults, resultCnt);
    std::vector<ComCloundJobErrno> sendStartCloundJobRets = sendStartCloundJob(results, resultCnt, jobData);
    for (int i = 0; i < resultCnt; ++i) {
        switch (results[i].error) {
        case FNET_ADD_CLOUND_JOB_OK:
            errorMap.emplace(results[i].devId, sendStartCloundJobRets[i]);
            break;
        case FNET_ADD_CLOUND_JOB_DEVICE_BUSY:
            errorMap.emplace(results[i].devId, COM_CLOUND_JOB_DEVICE_BUSY);
            break;
        case FNET_ADD_CLOUND_JOB_DEVICE_NOT_FOUND:
            errorMap.emplace(results[i].devId, COM_CLOUND_JOB_DEVICE_NOT_FOUND);
            break;
        case FNET_ADD_CLOUND_JOB_SERVER_INTERNAL_ERROR:
            errorMap.emplace(results[i].devId, COM_CLOUND_JOB_SERVER_INTERNAL_ERROR);
            break;
        default:
            errorMap.emplace(results[i].devId, COM_CLOUND_JOB_UNKNOWN_ERROR);
        }
    }
    return fnetRet;
}

std::string WanDevSendGcodeThd::getFileMd5(const char *filePath)
{
    std::ifstream fs;
    fs.open(filePath, std::fstream::binary);
    if (!fs.is_open()) {
        return std::string();
    }
    MD5_CTX md5;
    if (MD5_Init(&md5) == 0) {
        return std::string();
    }
    std::vector<char> buf(4096);
    do {
        fs.read(buf.data(), buf.size());
        if (fs.gcount() > 0) {
            MD5_Update(&md5, buf.data(), fs.gcount());
        }
    } while (fs.good());

    unsigned char hash[MD5_DIGEST_LENGTH];
    MD5_Final(hash, &md5);

    std::string result;
    boost::algorithm::hex_lower(std::begin(hash), std::end(hash), std::back_inserter(result));
    return result;
}

std::vector<ComCloundJobErrno> WanDevSendGcodeThd::sendStartCloundJob(const fnet_add_clound_job_result_t *results,
    int resultCnt, fnet_clound_job_data_t &jobData)
{
    for (int i = 0; i < resultCnt; ++i) {
        if (m_serialNumberMap.find(results[i].devId) == m_serialNumberMap.end()) {
            BOOST_LOG_TRIVIAL(error) << "invalid devId: " << results[i].devId;
            return std::vector<ComCloundJobErrno>(resultCnt, COM_CLOUND_JOB_UNKNOWN_ERROR);
        }
    }
    std::vector<ComCloundJobErrno> rets;
    for (int i = 0; i < resultCnt; i += 30) {
        std::vector<const char *> serialNumbers(std::min(30, resultCnt - i));
        std::vector<const char *> jobIds(serialNumbers.size());
        for (size_t j = 0; j < serialNumbers.size(); ++j) {
            serialNumbers[j] = m_serialNumberMap.at(results[i + j].devId).c_str();
            jobIds[j] = results[i + j].jobId;
        }
        jobData.devIds = nullptr;
        jobData.devSerialNumbers = serialNumbers.data();
        jobData.jobIds = jobIds.data();
        jobData.devCnt = serialNumbers.size();
        ComCloundJobErrno ret = COM_CLOUND_JOB_OK;
        if (serialNumbers.size() == 1) {
            const char *nimAccountId = m_nimAccountIdMap.at(results[i].devId).c_str();
            if (ComWanNimConn::inst()->sendStartCloundJob(0, nimAccountId, jobData) != COM_OK) {
                ret = COM_CLOUND_JOB_NIM_SEND_ERROR;
            }
        } else {
            if (ComWanNimConn::inst()->sendStartCloundJob(1, m_nimTeamId.c_str(), jobData) != COM_OK) {
                ret = COM_CLOUND_JOB_NIM_SEND_ERROR;
            }
        }
        rets.insert(rets.end(), serialNumbers.size(), ret);
    }
    return rets;
}

int WanDevSendGcodeThd::callback(long long now, long long total, void *callbackData)
{
    WanDevSendGcodeThd *inst = (WanDevSendGcodeThd *)callbackData;
    if (total != 0) {
        double progress = (double)now / total;
        if (progress - inst->m_progress > 0.025) {
            inst->QueueEvent(new ComSendGcodeProgressEvent(COM_SEND_GCODE_PROGRESS_EVENT, now, total));
            inst->m_progress = progress;
        }
    }
    return inst->m_callbackRet;
}

}} // namespace Slic3r::GUI
