#include "MultiComUtils.hpp"
#include "FreeInDestructor.h"
#include "MultiComMgr.hpp"

#include "nlohmann/json.hpp"
#include "slic3r/Utils/Http.hpp"

#include <initializer_list>

namespace Slic3r { namespace GUI {

namespace {

constexpr const char *CREATOR_SELF_PROFILE_URL = "https://api.voxelshare.com/api/v3/creator/self/profile";
constexpr size_t kResponseLogLimit = 5000;

std::string truncate_for_log(const std::string &text, size_t maxLen = kResponseLogLimit)
{
    if (text.size() <= maxLen) {
        return text;
    }
    return text.substr(0, maxLen) + "...(truncated)";
}

std::string com_errno_to_string(ComErrno err)
{
    switch (err) {
    case COM_OK:
        return "COM_OK";
    case COM_ERROR:
        return "COM_ERROR";
    case COM_UNAUTHORIZED:
        return "COM_UNAUTHORIZED";
    default:
        return "COM_UNKNOWN(" + std::to_string(static_cast<int>(err)) + ")";
    }
}

const nlohmann::json *profile_json_object(const nlohmann::json &json)
{
    if (json.contains("data") && json["data"].is_object()) {
        return profile_json_object(json["data"]);
    }
    if (json.contains("result") && json["result"].is_object()) {
        return profile_json_object(json["result"]);
    }
    if (json.contains("profile") && json["profile"].is_object()) {
        return profile_json_object(json["profile"]);
    }
    if (json.contains("user") && json["user"].is_object()) {
        return profile_json_object(json["user"]);
    }
    return json.is_object() ? &json : nullptr;
}

std::string get_json_string(const nlohmann::json &json, const std::initializer_list<const char *> keys)
{
    for (const char *key : keys) {
        if (json.contains(key) && json[key].is_string()) {
            return json[key].get<std::string>();
        }
        if (json.contains(key) && json[key].is_number_integer()) {
            return std::to_string(json[key].get<int64_t>());
        }
    }
    return {};
}

std::string get_json_string_path(const nlohmann::json &json, const std::initializer_list<const char *> path)
{
    const nlohmann::json *node = &json;
    size_t               index = 0;
    const size_t         path_size = path.size();

    for (const char *key : path) {
        if (!node->is_object() || !node->contains(key)) {
            return {};
        }

        const nlohmann::json &value = (*node).at(key);
        ++index;

        if (index == path_size) {
            if (value.is_string()) {
                return value.get<std::string>();
            }
            if (value.is_number_integer()) {
                return std::to_string(value.get<int64_t>());
            }
            return {};
        }

        node = &value;
    }

    return {};
}

std::string get_json_string(const nlohmann::json &json,
    const std::initializer_list<std::initializer_list<const char *>> key_paths)
{
    for (const auto &path : key_paths) {
        const std::string value = get_json_string_path(json, path);
        if (!value.empty()) {
            return value;
        }
    }
    return {};
}

ComErrno http_status_to_com_errno(unsigned status)
{
    return status == 401 || status == 403 ? COM_UNAUTHORIZED : COM_ERROR;
}

ComErrno get_fnet_user_profile(const std::string &accessToken, com_user_profile_t &userProfile, int msTimeout)
{
    BOOST_LOG_TRIVIAL(info) << "[user-profile] getUserProfile(fnet): start, token_len=" << accessToken.size()
                            << ", timeout_ms=" << msTimeout;
    fnet::FlashNetworkIntfc *intfc = MultiComMgr::inst()->networkIntfc();
    if (intfc == nullptr) {
        BOOST_LOG_TRIVIAL(warning) << "[user-profile] getUserProfile(fnet): network interface is null";
        return COM_ERROR;
    }
    fnet_user_profile_t *fnetProfile;
    int fnetRet = intfc->getUserProfile(accessToken.c_str(), &fnetProfile, msTimeout);
    if (fnetRet != FNET_OK) {
        BOOST_LOG_TRIVIAL(warning) << "[user-profile] getUserProfile(fnet): failed, ret=" << fnetRet
                                << ", com_errno=" << MultiComUtils::fnetRet2ComErrno(fnetRet);
        return MultiComUtils::fnetRet2ComErrno(fnetRet);
    }
    fnet::FreeInDestructor freeProfile(fnetProfile, intfc->freeUserProfile);
    userProfile.uid        = fnetProfile->uid;
    userProfile.nickname   = fnetProfile->nickname;
    userProfile.headImgUrl = fnetProfile->headImgUrl;
    userProfile.email      = fnetProfile->email;
    BOOST_LOG_TRIVIAL(info) << "[user-profile] getUserProfile(fnet): success, uid=" << userProfile.uid
                            << ", nickname=" << userProfile.nickname
                            << ", headImgUrl=" << userProfile.headImgUrl
                            << ", email=" << userProfile.email;
    return COM_OK;
}

} // namespace

ComErrno MultiComUtils::getLanDevList(std::vector<fnet_lan_dev_info> &devInfos)
{
    fnet::FlashNetworkIntfc *intfc = MultiComMgr::inst()->networkIntfc();
    if (intfc == nullptr) {
        return COM_ERROR;
    }
    int devCnt;
    fnet_lan_dev_info *fnetDevInfos;
    int fnetRet = intfc->getLanDevList(&fnetDevInfos, &devCnt, 500);
    if (fnetRet != FNET_OK) {
        return fnetRet2ComErrno(fnetRet);
    }
    fnet::FreeInDestructor freeDevInfos(fnetDevInfos, intfc->freeLanDevInfos);
    devInfos.clear();
    for (int i = 0; i < devCnt; ++i) {
        devInfos.push_back(fnetDevInfos[i]);
    }
    return COM_OK;
}

ComErrno MultiComUtils::getTokenByPassword(const std::string &userName, const std::string &password,
    const std::string &language, com_token_data_t &tokenData, std::string &message, int msTimeout)
{
    fnet::FlashNetworkIntfc *intfc = MultiComMgr::inst()->networkIntfc();
    if (intfc == nullptr) {
        return COM_ERROR;
    }
    time_t startTime = time(nullptr);
    fnet_token_data_t *fnetTokenData;
    char *fnetMessage = nullptr;
    fnet::FreeInDestructor freeFnetMessage(fnetMessage, intfc->freeString);
    int fnetRet = intfc->getTokenByPassword(userName.c_str(), password.c_str(), language.c_str(),
        &fnetTokenData, &fnetMessage, msTimeout);
    if (fnetMessage != nullptr) {
        message = fnetMessage;
    }
    if (fnetRet != FNET_OK) {
        return fnetRet2ComErrno(fnetRet);
    }
    fnet::FreeInDestructor freeTokenInfo(fnetTokenData, intfc->freeToken);
    tokenData.expiresIn = fnetTokenData->expiresIn;
    tokenData.accessToken = fnetTokenData->accessToken;
    tokenData.refreshToken = fnetTokenData->refreshToken;
    tokenData.startTime = startTime;
    return COM_OK;
}

ComErrno MultiComUtils::refreshToken(const std::string &refreshToken, com_token_data_t &tokenData, int msTimeout)
{
    fnet::FlashNetworkIntfc *intfc = MultiComMgr::inst()->networkIntfc();
    if (intfc == nullptr) {
        return COM_ERROR;
    }
    time_t startTime = time(nullptr);
    fnet_token_data_t *fnetTokenData;
    int fnetRet = intfc->refreshToken(refreshToken.c_str(), "en", &fnetTokenData, nullptr, msTimeout);
    if (fnetRet != FNET_OK) {
        return fnetRet2ComErrno(fnetRet);
    }
    fnet::FreeInDestructor freeTokenData(fnetTokenData, intfc->freeToken);
    tokenData.expiresIn = fnetTokenData->expiresIn;
    tokenData.accessToken = fnetTokenData->accessToken;
    tokenData.refreshToken = fnetTokenData->refreshToken;
    tokenData.startTime = startTime;
    return COM_OK;
}

ComErrno MultiComUtils::sendSMSCode(const std::string &phoneNumber, const std::string &language,
    std::string &message, int msTimeout)
{
    fnet::FlashNetworkIntfc *intfc = MultiComMgr::inst()->networkIntfc();
    if (intfc == nullptr) {
        return COM_ERROR;
    }
    char *fnetMessage = nullptr;
    fnet::FreeInDestructor freeFnetMessage(fnetMessage, intfc->freeString);
    int fnetRet = intfc->sendSMSCode(phoneNumber.c_str(), language.c_str(), &fnetMessage, msTimeout);
    if (fnetMessage != nullptr) {
        message = fnetMessage;
    }
    if (fnetRet != FNET_OK) {
        return fnetRet2ComErrno(fnetRet);
    }
    return COM_OK;
}

ComErrno MultiComUtils::getTokenBySMSCode(const std::string &userName, const std::string &SMSCode,
    const std::string &language, com_token_data_t &tokenData, std::string &message, int msTimeout)
{
    fnet::FlashNetworkIntfc *intfc = MultiComMgr::inst()->networkIntfc();
    if (intfc == nullptr) {
        return COM_ERROR;
    }
    time_t startTime = time(nullptr);
    fnet_token_data_t *fnetTokenData;
    char *fnetMessage = nullptr;
    fnet::FreeInDestructor freeFnetMessage(fnetMessage, intfc->freeString);
    int fnetRet = intfc->getTokenBySMSCode(userName.c_str(), SMSCode.c_str(), language.c_str(),
        &fnetTokenData, &fnetMessage, msTimeout);
    if (fnetMessage != nullptr) {
        message = fnetMessage;
    }
    if (fnetRet != FNET_OK) {
        return fnetRet2ComErrno(fnetRet);
    }
    fnet::FreeInDestructor freeTokenInfo(fnetTokenData, intfc->freeToken);
    tokenData.expiresIn = fnetTokenData->expiresIn;
    tokenData.accessToken = fnetTokenData->accessToken;
    tokenData.refreshToken = fnetTokenData->refreshToken;
    tokenData.startTime = startTime;
    return COM_OK;
}

ComErrno MultiComUtils::getUserProfile(const std::string &accessToken, com_user_profile_t &userProfile,
    int msTimeout)
{
    std::string        body;
    std::string        responseBody;
    std::string        requestError;
    unsigned           status = 0;
    ComErrno           ret = COM_ERROR;
    com_user_profile_t creatorProfile;
    std::string        profile_source = "new";

    BOOST_LOG_TRIVIAL(info) << "[user-profile] getUserProfile: start new endpoint "
                            << CREATOR_SELF_PROFILE_URL << ", token_len=" << accessToken.size()
                            << ", timeout_ms=" << msTimeout;

    Slic3r::Http::get(CREATOR_SELF_PROFILE_URL)
        .header("Authorization", "Bearer " + accessToken)
        .timeout_connect(msTimeout / 1000)
        .timeout_max(msTimeout / 1000)
        .on_complete([&](std::string response_body, unsigned code) {
            status = code;
            responseBody = std::move(response_body);
            if (status == 200) {
                body = responseBody;
                ret  = COM_OK;
            } else {
                ret = http_status_to_com_errno(status);
            }
        })
        .on_error([&](std::string response_body, std::string error, unsigned code) {
            status = code;
            responseBody = std::move(response_body);
            requestError = std::move(error);
            ret = http_status_to_com_errno(status);
        })
        .perform_sync();

    responseBody = responseBody.empty() ? body : responseBody;
    BOOST_LOG_TRIVIAL(info) << "[user-profile] getUserProfile(new): status=" << status
                            << ", ret=" << com_errno_to_string(ret)
                            << ", body_len=" << responseBody.size()
                            << ", raw_body=" << truncate_for_log(responseBody);
    if (!requestError.empty()) {
        BOOST_LOG_TRIVIAL(warning) << "[user-profile] getUserProfile(new): request_error=" << requestError;
    }

    if (ret != COM_OK) {
        if (ret == COM_UNAUTHORIZED) {
            BOOST_LOG_TRIVIAL(warning) << "[user-profile] getUserProfile: new endpoint unauthorized, stop";
            return ret;
        }
        profile_source = "old";
        BOOST_LOG_TRIVIAL(warning) << "[user-profile] getUserProfile: fallback to old fnet profile, because new endpoint failed";
        ComErrno oldRet = get_fnet_user_profile(accessToken, userProfile, msTimeout);
        BOOST_LOG_TRIVIAL(info) << "[user-profile] getUserProfile: final source=" << profile_source
                                << ", ret=" << com_errno_to_string(oldRet)
                                << ", uid=" << userProfile.uid << ", nickname=" << userProfile.nickname
                                << ", headImgUrl=" << userProfile.headImgUrl << ", email=" << userProfile.email;
        return oldRet;
    }

    const auto json = nlohmann::json::parse(body, nullptr, false, true);
    if (json.is_discarded()) {
        profile_source = "old";
        BOOST_LOG_TRIVIAL(warning) << "[user-profile] getUserProfile: new endpoint JSON parse discarded, fallback old";
        ComErrno oldRet = get_fnet_user_profile(accessToken, userProfile, msTimeout);
        BOOST_LOG_TRIVIAL(info) << "[user-profile] getUserProfile: final source=" << profile_source
                                << ", ret=" << com_errno_to_string(oldRet)
                                << ", uid=" << userProfile.uid << ", nickname=" << userProfile.nickname
                                << ", headImgUrl=" << userProfile.headImgUrl << ", email=" << userProfile.email;
        return oldRet;
    }

    const nlohmann::json *profile = profile_json_object(json);
    if (profile == nullptr) {
        profile_source = "old";
        BOOST_LOG_TRIVIAL(warning) << "[user-profile] getUserProfile: cannot extract profile json object, fallback old";
        ComErrno oldRet = get_fnet_user_profile(accessToken, userProfile, msTimeout);
        BOOST_LOG_TRIVIAL(info) << "[user-profile] getUserProfile: final source=" << profile_source
                                << ", ret=" << com_errno_to_string(oldRet)
                                << ", uid=" << userProfile.uid << ", nickname=" << userProfile.nickname
                                << ", headImgUrl=" << userProfile.headImgUrl << ", email=" << userProfile.email;
        return oldRet;
    }

    creatorProfile.uid        = get_json_string(*profile, {"uid", "id", "userId", "user_id"});
    creatorProfile.nickname   = get_json_string(*profile, {"nickname", "nickName", "name", "username", "userName"});
    creatorProfile.headImgUrl = get_json_string(*profile, {
        {"headImgUrl"},
        {"avatar"},
        {"avatarUrl"},
        {"avatar_url"},
        {"head_img_url"},
        {"avatar", "url"},
        {"avatar", "Icon"},
        {"avatar", "Avatar"},
        {"avatar", "avatarUrl"},
        {"avatar", "avatar_url"},
        {"image", "url"},
        {"picture", "url"},
        {"headImg", "url"}
    });
    creatorProfile.email      = get_json_string(*profile, {"email", "mail"});
    BOOST_LOG_TRIVIAL(info) << "[user-profile] getUserProfile(new): parsed headImgUrl=" << creatorProfile.headImgUrl
                            << ", uid=" << creatorProfile.uid
                            << ", nickname=" << creatorProfile.nickname
                            << ", email=" << creatorProfile.email;

    userProfile = creatorProfile;
    bool fallbackUsed = false;
    if (userProfile.uid.empty() || userProfile.email.empty()) {
        fallbackUsed = true;
        profile_source = "new->old";
        BOOST_LOG_TRIVIAL(warning) << "[user-profile] getUserProfile: missing required field, fallback old for missing values";
        com_user_profile_t fallbackProfile;
        if (get_fnet_user_profile(accessToken, fallbackProfile, msTimeout) == COM_OK) {
            if (userProfile.uid.empty()) {
                userProfile.uid = fallbackProfile.uid;
            }
            if (userProfile.nickname.empty()) {
                userProfile.nickname = fallbackProfile.nickname;
            }
            if (userProfile.headImgUrl.empty()) {
                userProfile.headImgUrl = fallbackProfile.headImgUrl;
            }
            if (userProfile.email.empty()) {
                userProfile.email = fallbackProfile.email;
            }
            BOOST_LOG_TRIVIAL(info) << "[user-profile] getUserProfile: fallback old result uid=" << fallbackProfile.uid
                                    << ", nickname=" << fallbackProfile.nickname
                                    << ", headImgUrl=" << fallbackProfile.headImgUrl
                                    << ", email=" << fallbackProfile.email;
        }
    }
    if (!fallbackUsed && userProfile.headImgUrl.empty()) {
        fallbackUsed = true;
        profile_source = "new->old";
        BOOST_LOG_TRIVIAL(warning) << "[user-profile] getUserProfile: headImgUrl empty, fallback old for full values";
        com_user_profile_t fallbackProfile;
        if (get_fnet_user_profile(accessToken, fallbackProfile, msTimeout) == COM_OK) {
            userProfile.uid       = userProfile.uid.empty() ? fallbackProfile.uid : userProfile.uid;
            userProfile.nickname  = userProfile.nickname.empty() ? fallbackProfile.nickname : userProfile.nickname;
            userProfile.headImgUrl = fallbackProfile.headImgUrl;
            userProfile.email     = userProfile.email.empty() ? fallbackProfile.email : userProfile.email;
            BOOST_LOG_TRIVIAL(info) << "[user-profile] getUserProfile: fallback old (headImg) result "
                                    << "uid=" << fallbackProfile.uid << ", nickname=" << fallbackProfile.nickname
                                    << ", headImgUrl=" << fallbackProfile.headImgUrl << ", email=" << fallbackProfile.email;
        }
    }

    BOOST_LOG_TRIVIAL(info) << "[user-profile] getUserProfile: final source=" << profile_source
                            << ", uid=" << userProfile.uid
                            << ", nickname=" << userProfile.nickname
                            << ", headImgUrl=" << userProfile.headImgUrl
                            << ", email=" << userProfile.email;
    return COM_OK;
}

ComErrno MultiComUtils::bindAccountRelp(const std::string &clientId, const std::string &accessToken,
    const std::string &email, bool &showUserPoints, int msTimeout)
{
    fnet::FlashNetworkIntfc *intfc = MultiComMgr::inst()->networkIntfc();
    if (intfc == nullptr) {
        return COM_ERROR;
    }
    fnet_bind_account_relp_result_t *bindResult;
    int fnetRet = intfc->bindAccountRelp(
        clientId.c_str(), accessToken.c_str(), email.c_str(), &bindResult, msTimeout);
    if (fnetRet != FNET_OK) {
        return fnetRet2ComErrno(fnetRet);
    }
    fnet::FreeInDestructor freeBindResult(bindResult, intfc->freeBindAccountRelpResult);
    showUserPoints = bindResult->showUserPoints;
    return COM_OK;
}

ComErrno MultiComUtils::getMqttConfig(const std::string &clientId, const std::string &accessToken,
    com_mqtt_config_t &mqttConfig, int msTimeout)
{
    fnet::FlashNetworkIntfc *intfc = MultiComMgr::inst()->networkIntfc();
    if (intfc == nullptr) {
        return COM_ERROR;
    }
    fnet_mqtt_config_t *fnetMqttConfig;
    int fnetRet = intfc->getMqttConfig(clientId.c_str(), accessToken.c_str(), &fnetMqttConfig, msTimeout);
    if (fnetRet != FNET_OK) {
        return fnetRet2ComErrno(fnetRet);
    }
    fnet::FreeInDestructor freeMqttConfig(fnetMqttConfig, intfc->freeMqttConfig);
    mqttConfig.userTopic = fnetMqttConfig->userTopic;
    for (int i = 0; i < fnetMqttConfig->commonTopicCnt; ++i) {
        mqttConfig.commonTopics.push_back(fnetMqttConfig->commonTopics[i]);
    }
    return COM_OK;
}

ComErrno MultiComUtils::downloadFileMem(const std::string &url, std::vector<char> &bytes,
    fnet_progress_callback_t callback, void *callbackData, int msConnectTimeout, int msTimeout)
{
    fnet::FlashNetworkIntfc *intfc = MultiComMgr::inst()->networkIntfc();
    if (intfc == nullptr) {
        return COM_ERROR;
    }
    fnet_file_data_t *fileData;
    int fnetRet = intfc->downloadFileMem(
        url.c_str(), &fileData, callback, callbackData, msConnectTimeout, msTimeout);
    if (fnetRet != FNET_OK) {
        return fnetRet2ComErrno(fnetRet);
    }
    fnet::FreeInDestructor freeFileData(fileData, intfc->freeFileData);
    bytes.assign(fileData->data, fileData->data + fileData->size);
    return COM_OK;
}

ComErrno MultiComUtils::downloadFileDisk(const std::string &url, const wxString &saveName,
    fnet_progress_callback_t callback, void *callbackData, int msConnectTimeout, int msTimeout)
{
    fnet::FlashNetworkIntfc *intfc = MultiComMgr::inst()->networkIntfc();
    if (intfc == nullptr) {
        return COM_ERROR;
    }
    int fnetRet = intfc->downloadFileDisk(
        url.c_str(), saveName.ToUTF8().data(), callback, callbackData, msConnectTimeout, msTimeout);
    if (fnetRet != FNET_OK) {
        return fnetRet2ComErrno(fnetRet);
    }
    return COM_OK;
}

ComErrno MultiComUtils::fnetRet2ComErrno(int networkRet)
{
    switch (networkRet) {
    case FNET_OK:
        return COM_OK;
    case FNET_ABORTED_BY_CALLBACK:
        return COM_ABORTED_BY_USER;
    case FNET_DIVICE_IS_BUSY:
        return COM_DEVICE_IS_BUSY;
    case FNET_GCODE_NOT_FOUND:
        return COM_GCODE_NOT_FOUND;
    case FNET_VERIFY_LAN_DEV_FAILED:
        return COM_VERIFY_LAN_DEV_FAILED;
    case FNET_UNAUTHORIZED:
        return COM_UNAUTHORIZED;
    case FNET_INVALID_VALIDATION:
        return COM_INVALID_VALIDATION;
    case FNET_DEVICE_HAS_BEEN_BOUND:
        return COM_DEVICE_HAS_BEEN_BOUND;
    case FNET_ABORT_AI_JOB_FAILED:
        return COM_ABORT_AI_JOB_FAILED;
    case FENT_AI_JOB_NOT_ENOUGH_POINTS:
        return COM_AI_JOB_NOT_ENOUGH_POINTS;
    case FNET_NO_EXISTING_AI_MODEL_JOB:
        return COM_NO_EXISTING_AI_MODEL_JOB;
    case FNET_INPUT_FAILED_THE_REVIEW:
        return COM_INPUT_FAILED_THE_REVIEW;
    case FNET_PRINT_LIST_MODEL_COUNT_EXCEEDED:
        return COM_PRINT_LIST_MODEL_COUNT_EXCEEDED;
    case FNET_CONN_SEND_ERROR:
        return COM_CONN_SEND_ERROR;
    default:
        return COM_ERROR;
    }
}

std::vector<fnet_material_mapping_t> MultiComUtils::comMaterialMappings2Fnet(
    const std::vector<com_material_mapping_t> &comMaterialMappings)
{
    std::vector<fnet_material_mapping_t> ret(comMaterialMappings.size());
    for (size_t i = 0; i < ret.size(); ++i) {
        const com_material_mapping_t &comMaterialMapping = comMaterialMappings[i];
        ret[i].toolId = comMaterialMapping.toolId;
        ret[i].slotId = comMaterialMapping.slotId;
        ret[i].materialName = comMaterialMapping.materialName.c_str();
        ret[i].toolMaterialColor = comMaterialMapping.toolMaterialColor.c_str();
        ret[i].slotMaterialColor = comMaterialMapping.slotMaterialColor.c_str();
    }
    return ret;
}

}} // namespace Slic3r::GUI
