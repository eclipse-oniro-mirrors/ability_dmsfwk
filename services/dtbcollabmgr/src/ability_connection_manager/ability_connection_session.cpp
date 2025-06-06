/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "ability_connection_session.h"

#include <charconv>
#include <chrono>
#include <map>
#include <unistd.h>
#include <sys/prctl.h>
#include <sstream>
#include <iomanip>
#include <future>
#include <vector>
 
#include "ability_connection_manager.h"
#include "dtbcollabmgr_log.h"
#include "dtbschedmgr_log.h"
#include "distributed_client.h"
#include "ipc_skeleton.h"
#include "message_data_header.h"
#include "openssl/sha.h"
#include "tokenid_kit.h"

namespace OHOS {
namespace DistributedCollab {
namespace {
const std::string TAG = "AbilityConnectionSession";
const std::string CONNECT_SESSION_TIMEOUT_END_TASK = "connect_session_timeout_end_task";
const std::u16string DMS_PROXY_INTERFACE_TOKEN = u"ohos.distributedschedule.accessToken";
const std::string EVENT_CONNECT = "connect";
const std::string EVENT_DISCONNECT = "disconnect";
const std::string EVENT_RECEIVE_MESSAGE = "receiveMessage";
const std::string EVENT_RECEIVE_DATA = "receiveData";
const std::string EVENT_RECEIVE_IMAGE = "receiveImage";
const std::string EVENT_COLLABORATE = "collaborateEvent";
constexpr int32_t DSCHED_COLLAB_PROTOCOL_VERSION = 1;
static constexpr uint16_t PROTOCOL_VERSION = 1;
constexpr int32_t CHANNEL_NAME_LENGTH = 48;
constexpr int32_t VIDEO_FRAME_RATE = 30;
constexpr int32_t DEFAULT_APP_UID = 0;
constexpr int32_t DEFAULT_APP_PID = 0;
constexpr int32_t DEFAULT_INSTANCE_ID = 0;
constexpr int32_t HEX_WIDTH = 2;
constexpr char FILL_CHAR = '0';
constexpr int32_t WAIT_FOR_CONNECT = 11;
}

AbilityConnectionSession::AbilityConnectionSession(int32_t sessionId, std::string serverSocketName,
    AbilityConnectionSessionInfo sessionInfo, ConnectOption opt)
{
    sessionId_ = sessionId;
    localSocketName_ = serverSocketName;
    sessionInfo_ = sessionInfo;
    connectOption_ = opt;
    version_ = DSCHED_COLLAB_PROTOCOL_VERSION;
    InitMessageHandlerMap();
}

AbilityConnectionSession::~AbilityConnectionSession()
{
}

void AbilityConnectionSession::InitMessageHandlerMap()
{
    messageHandlerMap_[static_cast<uint32_t>(MessageType::NORMAL)] =
        [this](const std::string& msg) { ExeuteMessageEventCallback(msg); };
    messageHandlerMap_[static_cast<uint32_t>(MessageType::WIFI_OPEN)] =
        [this](const std::string&) { ConnectStreamChannel(); };
    messageHandlerMap_[static_cast<uint32_t>(MessageType::UPDATE_RECV_ENGINE_CHANNEL)] =
        [this](const std::string&) { UpdateRecvEngineTransChannel(); };
    messageHandlerMap_[static_cast<uint32_t>(MessageType::UPDATE_SENDER_ENGINE_CHANNEL)] =
        [this](const std::string&) { UpdateSenderEngineTransChannel(); };
    messageHandlerMap_[static_cast<uint32_t>(MessageType::CONNECT_FILE_CHANNEL)] =
        [this](const std::string& msg) { ConnectFileChannel(msg); };
    messageHandlerMap_[static_cast<uint32_t>(MessageType::FILE_CHANNEL_CONNECT_SUCCESS)] =
        [this](const std::string&) { NotifyAppConnectResult(true); };
    messageHandlerMap_[static_cast<uint32_t>(MessageType::FILE_CHANNEL_CONNECT_FAILED)] =
        [this](const std::string&) { NotifyAppConnectResult(false); };
    messageHandlerMap_[static_cast<uint32_t>(MessageType::SESSION_CONNECT_SUCCESS)] =
        [this](const std::string&) { HandleSessionConnect(); };
    messageHandlerMap_[static_cast<uint32_t>(MessageType::RECEIVE_STREAM_START)] =
        [this](const std::string&) { UpdateRecvEngineStatus(); };
}

void AbilityConnectionSession::Init()
{
    HILOGI("Init AbilityConnectionSession start");
    if (eventHandler_ != nullptr) {
        HILOGI("AbilityConnectionSession already inited, end.");
        return;
    }
    eventThread_ = std::thread(&AbilityConnectionSession::StartEvent, this);
    std::unique_lock<std::mutex> lock(eventMutex_);
    eventCon_.wait(lock, [this] {
        return eventHandler_ != nullptr;
    });
    HILOGI("Init AbilityConnectionSession end");
}

void AbilityConnectionSession::StartEvent()
{
    HILOGI("StartEvent start");
    std::string name = localSocketName_ + std::to_string(sessionId_);
    prctl(PR_SET_NAME, name.c_str());
    auto runner = AppExecFwk::EventRunner::Create(false);
    {
        std::lock_guard<std::mutex> lock(eventMutex_);
        eventHandler_ = std::make_shared<OHOS::AppExecFwk::EventHandler>(runner);
    }
    eventCon_.notify_one();
    runner->Run();
    HILOGI("StartEvent end");
}

void AbilityConnectionSession::UnInit()
{
    HILOGI("UnInit start");
    std::unique_lock<std::mutex> lock(eventMutex_);
    if (eventHandler_ != nullptr) {
        eventHandler_->GetEventRunner()->Stop();
        if (eventThread_.joinable()) {
            eventThread_.join();
        }
        eventHandler_ = nullptr;
    } else {
        HILOGE("eventHandler_ is nullptr");
    }
    HILOGI("UnInit end");
}

void AbilityConnectionSession::Release()
{
    HILOGI("called.");
    {
        std::unique_lock<std::shared_mutex> sessionStatusWriteLock(sessionMutex_);
        if (sessionStatus_ == SessionStatus::UNCONNECTED) {
            HILOGI("The session resource has been released.");
            return;
        }
        sessionStatus_ = SessionStatus::UNCONNECTED;
    }
    AbilityConnectionManager::GetInstance().DeleteConnectSession(sessionInfo_, sessionId_);
    DestroyStream();
    
    std::unique_lock<std::shared_mutex> channelLock(transChannelMutex_);
    for (auto iter = transChannels_.begin(); iter != transChannels_.end(); iter++) {
        ChannelManager::GetInstance().DeleteChannel(iter->second.channelId);
    }
    transChannels_.clear();
}

PeerInfo AbilityConnectionSession::GetPeerInfo()
{
    return sessionInfo_.peerInfo_;
}

PeerInfo AbilityConnectionSession::GetLocalInfo()
{
    return sessionInfo_.localInfo_;
}

std::string AbilityConnectionSession::GetServerToken()
{
    return dmsServerToken_;
}

int32_t AbilityConnectionSession::HandlePeerVersion(int32_t version)
{
    HILOGI("called.");
    DistributedClient dmsClient;
    int32_t ret = dmsClient.CollabMission(sessionId_,
        localSocketName_, sessionInfo_,
        connectOption_, dmsServerToken_);
    if (ret != ERR_OK) {
        HILOGE("collab mission start failed.");
        ConnectResult connectResult(false, ConnectErrorCode::SYSTEM_INTERNAL_ERROR, "");
        ExeuteConnectCallback(connectResult);
    }
    return ret;
}

int32_t AbilityConnectionSession::Connect(ConnectCallback& callback)
{
    HILOGI("called.");
    connectCallback_ = callback;
    if (CheckConnectedSession()) {
        HILOGE("connected session %{public}d exists.", sessionId_);
        return CONNECTED_SESSION_EXISTS;
    }
    if (!CheckWifiStatus()) {
        HILOGI("Wi-Fi is not enabled.");
        ConnectResult connectResult(false, ConnectErrorCode::LOCAL_WIFI_NOT_OPEN, "");
        ExeuteConnectCallback(connectResult);
        return LOCAL_WIFI_NOT_OPEN;
    }

    {
        std::unique_lock<std::shared_mutex> sessionStatusWriteLock(sessionMutex_);
        if (sessionStatus_ != SessionStatus::UNCONNECTED) {
            HILOGE("session has start to connect, sessionStatus is %{public}d", sessionStatus_);
            ConnectResult connectResult(false, ConnectErrorCode::CONNECTED_SESSION_EXISTS, "");
            ExeuteConnectCallback(connectResult);
            return CONNECTED_SESSION_EXISTS;
        }
        sessionStatus_ = SessionStatus::CONNECTING;
    }
    direction_ = CollabrateDirection::COLLABRATE_SOURCE;
    dmsServerToken_ = CreateDmsServerToken();
    DistributedClient dmsClient;
    int32_t ret = dmsClient.GetPeerVersion(sessionId_, sessionInfo_.peerInfo_.deviceId, dmsServerToken_);
    if (ret != ERR_OK) {
        HILOGE("collab mission start failed.");
        ConnectResult connectResult(false, ConnectErrorCode::SYSTEM_INTERNAL_ERROR, "");
        ExeuteConnectCallback(connectResult);
    }
    return ret;
}

bool AbilityConnectionSession::CheckConnectedSession()
{
    if (IsConnected()) {
        HILOGE("session %{public}d connected", sessionId_);
        ConnectResult connectResult(false, ConnectErrorCode::CONNECTED_SESSION_EXISTS, "");
        ExeuteConnectCallback(connectResult);
        return true;
    }

    int32_t ret = AbilityConnectionManager::GetInstance().UpdateClientSession(sessionInfo_, sessionId_);
    if (ret != ERR_OK) {
        HILOGE("connected session exists.");
        ConnectResult connectResult(false, ConnectErrorCode::CONNECTED_SESSION_EXISTS, "");
        ExeuteConnectCallback(connectResult);
        return true;
    }
    return false;
}

bool AbilityConnectionSession::CheckWifiStatus()
{
    uint64_t tokenId = OHOS::IPCSkeleton::GetSelfTokenID();
    if (OHOS::Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(tokenId)) {
        HILOGI("The current application is a system app.");
        return true;
    }

    DistributedClient dmsClient;
    return dmsClient.GetWifiStatus();
}

std::string AbilityConnectionSession::CreateDmsServerToken()
{
    auto pid = getprocpid();
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    std::string input = std::to_string(pid) + std::to_string(sessionId_) + std::to_string(timestamp);
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((const unsigned char*)input.c_str(), input.length(), hash);
    std::stringstream hashStr;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        hashStr << std::hex << std::setw(HEX_WIDTH) << std::setfill(FILL_CHAR) << (int)hash[i];
    }
    return hashStr.str().substr(0, CHANNEL_NAME_LENGTH);
}

int32_t AbilityConnectionSession::Disconnect()
{
    HILOGI("called.");
    Release();
    DistributedClient dmsClient;
    int32_t ret = dmsClient.NotifyCloseCollabSession(dmsServerToken_);
    HILOGI("Notify Server DisConnect result is %{public}d", ret);
    return ERR_OK;
}

int32_t AbilityConnectionSession::AcceptConnect(const std::string& token)
{
    HILOGI("called.");
    DistributedClient dmsClient;
    if (!CheckWifiStatus()) {
        HILOGI("Wi-Fi is not enabled.");
        dmsClient.NotifyPrepareResult(token, PEER_WIFI_NOT_OPEN, sessionId_, localSocketName_);
        return PEER_WIFI_NOT_OPEN;
    }
    {
        std::unique_lock<std::shared_mutex> sessionStatusWriteLock(sessionMutex_);
        if (sessionStatus_ != SessionStatus::UNCONNECTED) {
            HILOGE("session has start to connect, sessionStatus is %{public}d", sessionStatus_);
            return INVALID_PARAMETERS_ERR;
        }
        sessionStatus_ = SessionStatus::CONNECTING;
    }
    dmsServerToken_ = token;
    int32_t ret = AbilityConnectionManager::GetInstance().UpdateServerSession(sessionInfo_, sessionId_);
    if (ret != ERR_OK) {
        dmsClient.NotifyPrepareResult(token, ret, sessionId_, localSocketName_);
        Release();
        return INVALID_PARAMETERS_ERR;
    }

    direction_ = CollabrateDirection::COLLABRATE_SINK;
    ret = InitChannels();
    if (ret != ERR_OK) {
        HILOGE("init sink client failed!");
        dmsClient.NotifyPrepareResult(token, ret, sessionId_, localSocketName_);
        Release();
        return ret;
    }
    ret = dmsClient.NotifyPrepareResult(token, ERR_OK, sessionId_, localSocketName_);
    if (ret != ERR_OK) {
        HILOGE("notify prepare result failed!");
        Release();
    }
    return ERR_OK;
}

int32_t AbilityConnectionSession::HandleCollabResult(int32_t result, const std::string& peerSocketName,
    const std::string& dmsServerToken, const std::string& reason)
{
    HILOGI("called.");
    dmsServerToken_ = dmsServerToken;
    peerSocketName_ = peerSocketName;
    if (result != ERR_OK) {
        HILOGE("collab result is failed, ret = %{public}d, reason = %{public}s", result, reason.c_str());
        NotifyCollabErrorResultFromSa(false, ConvertToConnectErrorCode(result), reason);
        return INVALID_PARAMETERS_ERR;
    }

    if (InitChannels() != ERR_OK || ConnectChannels() != ERR_OK) {
        NotifyAppConnectResult(false);
        return INVALID_PARAMETERS_ERR;
    }

    if (connectOption_.needReceiveFile) {
        return RequestReceiveFileChannelConnection();
    }

    NotifyPeerSessionConnected();
    NotifyAppConnectResult(true);
    return ERR_OK;
}

ConnectErrorCode AbilityConnectionSession::ConvertToConnectErrorCode(int32_t collabResult)
{
    HILOGI("Collaboration failed code is %{public}d.", collabResult);
    switch (collabResult) {
        case CONNECTED_SESSION_EXISTS:
            return ConnectErrorCode::CONNECTED_SESSION_EXISTS;
        case SAME_SESSION_IS_CONNECTING:
            return ConnectErrorCode::CONNECTED_SESSION_EXISTS;
        case PEER_WIFI_NOT_OPEN:
            return ConnectErrorCode::PEER_WIFI_NOT_OPEN;
        case DistributedSchedule::COLLAB_ABILITY_REJECT_ERR:
            return ConnectErrorCode::PEER_APP_REJECTED;
        case PEER_ABILITY_NO_ONCOLLABORATE:
            return ConnectErrorCode::PEER_ABILITY_NO_ONCOLLABORATE;
        default:
            return ConnectErrorCode::SYSTEM_INTERNAL_ERROR;
    }
}

int32_t AbilityConnectionSession::RequestReceiveFileChannelConnection()
{
    HILOGI("notify the peer end bind file.");
    int32_t ret = SendMessage(localSocketName_, MessageType::CONNECT_FILE_CHANNEL);
    if (ret != ERR_OK) {
        HILOGE("Failed to notify the file channel connection, ret = %{public}d", ret);
        NotifyAppConnectResult(false);
    }
    return ret;
}

void AbilityConnectionSession::NotifyPeerSessionConnected()
{
    if (!connectOption_.HasFileTransfer()) {
        HILOGI("No notification required.");
        return;
    }

    HILOGI("notify the peer end bind file.");
    int32_t ret = SendMessage("SESSION_CONNECT_SUCCESS", MessageType::SESSION_CONNECT_SUCCESS);
    if (ret != ERR_OK) {
        HILOGE("Failed to notify the session connection result, ret = %{public}d", ret);
    }
}

void AbilityConnectionSession::NotifyAppConnectResult(bool isConnected,
    const ConnectErrorCode errorCode, const std::string& reason)
{
    HILOGE("notify result from self %{public}d", sessionId_);
    ConnectResult connectResult(isConnected, errorCode, reason);
    if (isConnected) {
        connectResult.sessionId = sessionId_;
    } else {
        Release();
        DistributedClient dmsClient;
        dmsClient.NotifyCloseCollabSession(dmsServerToken_);
    }
    ExeuteConnectCallback(connectResult);
}

void AbilityConnectionSession::NotifyCollabErrorResultFromSa(bool isConnected,
    const ConnectErrorCode errorCode, const std::string& reason)
{
    HILOGE("notify result from sa %{public}d", sessionId_);
    ConnectResult connectResult(isConnected, errorCode, reason);
    connectResult.sessionId = sessionId_;
    Release();
    ExeuteConnectCallback(connectResult);
}

int32_t AbilityConnectionSession::HandleDisconnect()
{
    HILOGI("called.");
    {
        std::shared_lock<std::shared_mutex> sessionStatusReadLock(sessionMutex_);
        if (sessionStatus_ == SessionStatus::UNCONNECTED) {
            HILOGI("already disconnect");
            return ERR_OK;
        }
    }
    Release();
    std::shared_ptr<IAbilityConnectionSessionListener> listener;
    {
        std::shared_lock<std::shared_mutex> lock(sessionListenerMutex_);
        listener = sessionListener_;
    }
    if (listener) {
        HILOGI("handler sessionListener");
        listener->OnDisConnect(sessionId_);
    } else {
        EventCallbackInfo callbackInfo;
        callbackInfo.sessionId = sessionId_;
        callbackInfo.reason = DisconnectReason::PEER_APP_EXIT;
        ExeuteEventCallback(EVENT_DISCONNECT, callbackInfo);
    }
    return ERR_OK;
}

int32_t AbilityConnectionSession::SendMessage(const std::string& msg, const MessageType& messageType)
{
    HILOGI("called.");
    auto sendData = std::make_shared<AVTransDataBuffer>(msg.length() + 1);
    int32_t ret = memcpy_s(sendData->Data(), sendData->Capacity(), msg.c_str(), msg.length());
    if (ret != ERR_OK) {
        HILOGE("memory copy failed, ret %{public}d", ret);
        return ret;
    }

    uint32_t totalLen = sendData->Size() + MessageDataHeader::HEADER_LEN;

    MessageDataHeader headerPara(PROTOCOL_VERSION, static_cast<uint32_t>(messageType), totalLen);
    auto headerBuffer = headerPara.Serialize();
    auto sendBuffer = std::make_shared<AVTransDataBuffer>(totalLen);
    uint8_t* header = sendBuffer->Data();

    if (memcpy_s(header, sendBuffer->Size(), headerBuffer->Data(), MessageDataHeader::HEADER_LEN) != ERR_OK) {
        HILOGE("Write header failed");
        return WRITE_SESSION_HEADER_FAILED;
    }

    ret = memcpy_s(header + MessageDataHeader::HEADER_LEN, sendBuffer->Size() - MessageDataHeader::HEADER_LEN,
        sendData->Data(), sendData->Size());
    if (ret != ERR_OK) {
        HILOGE("Write data failed");
        return WRITE_SEND_DATA_BUFFER_FAILED;
    }

    TransChannelInfo transChannelInfo;
    ret = GetTransChannelInfo(TransChannelType::MESSAGE, transChannelInfo);
    if (ret != ERR_OK) {
        HILOGE("message channel not exit");
        return ret;
    }

    int32_t channelId = transChannelInfo.channelId;
    ret = ChannelManager::GetInstance().SendMessage(channelId, sendBuffer);
    if (ret != ERR_OK) {
        HILOGE("send message failed, channelId is %{public}d", channelId);
        return ret;
    }
    return ERR_OK;
}

int32_t AbilityConnectionSession::SendData(const std::shared_ptr<AVTransDataBuffer>& buffer)
{
    HILOGI("called.");
    TransChannelInfo transChannelInfo;
    int32_t ret = GetTransChannelInfo(TransChannelType::DATA, transChannelInfo);
    if (ret != ERR_OK) {
        HILOGE("data channel not exit");
        return ret;
    }
    
    int32_t channelId = transChannelInfo.channelId;
    ret = ChannelManager::GetInstance().SendBytes(channelId, buffer);
    if (ret != ERR_OK) {
        HILOGE("send bytes failed, channelId is %{public}d", channelId);
        return ret;
    }
    return ERR_OK;
}

int32_t AbilityConnectionSession::SendImage(const std::shared_ptr<Media::PixelMap>& image, int32_t imageQuality)
{
    HILOGI("called.");
    if (senderEngine_ == nullptr) {
        HILOGE("senderEngine_ is nullptr.");
        return INVALID_PARAMETERS_ERR;
    }
    int32_t ret = senderEngine_->SendPixelMap(image, imageQuality);
    if (ret != ERR_OK) {
        HILOGE("Send image failed, ret is %{public}d.", ret);
        return ret;
    }
    return ERR_OK;
}

int32_t AbilityConnectionSession::SendFile(const std::vector<std::string>& sFiles,
    const std::vector<std::string>& dFiles)
{
    HILOGI("called.");
    TransChannelInfo transChannelInfo;
    int32_t ret = GetTransChannelInfo(TransChannelType::SEND_FILE, transChannelInfo);
    if (ret != ERR_OK) {
        HILOGE("data channel not exit");
        return ret;
    }
    
    int32_t channelId = transChannelInfo.channelId;
    ret = ChannelManager::GetInstance().SendFile(channelId, sFiles, dFiles);
    if (ret != ERR_OK) {
        HILOGE("send bytes failed, channelId is %{public}d", channelId);
        return ret;
    }
    return ERR_OK;
}

int32_t AbilityConnectionSession::CreateStream(int32_t streamId, const StreamParams& param)
{
    HILOGI("called. StreamParams role is %{public}d", static_cast<int32_t>(param.role));
    streamParam_ = param;
    switch (param.role) {
        case StreamRole::SOURCE:
            streamId_ = streamId;
            return InitSenderEngine();
        case StreamRole::SINK:
            streamId_ = streamId;
            return InitRecvEngine();
        default:
            HILOGE("Unrecognized streamRole.");
            return INVALID_PARAMETERS_ERR;
        }
}

int32_t AbilityConnectionSession::InitSenderEngine()
{
    if (!connectOption_.needSendStream) {
        HILOGE("The stream sending option is not configured.");
        return INVALID_PARAMETERS_ERR;
    }
    std::unique_lock<std::shared_mutex> listenerWriteLock(engineMutex_);
    if (senderEngine_ != nullptr) {
        HILOGE("The stream sender engine has init.");
        return ONLY_SUPPORT_ONE_STREAM;
    }
    senderEngine_ = std::make_shared<AVSenderEngine>(DEFAULT_APP_UID, DEFAULT_APP_PID,
        sessionInfo_.localInfo_.bundleName, DEFAULT_INSTANCE_ID);
    senderEngine_->Init();
    return ERR_OK;
}

int32_t AbilityConnectionSession::InitRecvEngine()
{
    if (!connectOption_.needReceiveStream) {
        HILOGE("The stream receive option is not configured.");
        return INVALID_PARAMETERS_ERR;
    }
    std::unique_lock<std::shared_mutex> listenerWriteLock(engineMutex_);
    if (recvEngine_ != nullptr) {
        HILOGE("The stream receive has init.");
        return ONLY_SUPPORT_ONE_STREAM;
    }

    recvEngine_ = std::make_shared<AVReceiverEngine>();
    recvEngine_->Init();
    return ERR_OK;
}

int32_t AbilityConnectionSession::DestroyStream()
{
    HILOGI("called.");
    senderEngine_ = nullptr;
    recvEngine_ = nullptr;
    recvEngineState_ = EngineState::EMPTY;
    TransChannelInfo info;
    std::shared_lock<std::shared_mutex> channelReadLock(transChannelMutex_);
    auto item = transChannels_.find(TransChannelType::STREAM);
    if (item != transChannels_.end() && item->second.isConnected) {
        info = item->second;
    }
    ChannelManager::GetInstance().ClearSendTask(info.channelId);
    HILOGI("stream bytes channel clear task");
    item = transChannels_.find(TransChannelType::STREAM_BYTES);
    if (item != transChannels_.end() && item->second.isConnected) {
        info = item->second;
    }
    ChannelManager::GetInstance().ClearSendTask(info.channelId);
    HILOGI("stream bytes channel clear task");
    return ERR_OK;
}

template <typename T>
int32_t AbilityConnectionSession::ConfigEngineParam(std::shared_ptr<T> &engine, const SurfaceParams& param)
{
    engine->SetVideoSource(static_cast<VideoSourceType>(param.format));

    int32_t ret = ERR_OK;
    ret = engine->Configure(VidEnc(VideoCodecFormat::H264));
    if (ret != ERR_OK) {
        HILOGE("configure videnc failed");
        return ret;
    }
    ret = engine->Configure(VidRectangle(param.width, param.height));
    if (ret != ERR_OK) {
        HILOGE("configure VidRectangle failed");
        return ret;
    }
    ret = engine->Configure(VidBitRate(streamParam_.bitrate));
    if (ret != ERR_OK) {
        HILOGE("configure VidBitRate failed");
        return ret;
    }
    ret = engine->Configure(VidFrameRate(VIDEO_FRAME_RATE));
    if (ret != ERR_OK) {
        HILOGE("configure VidFrameRate failed");
        return ret;
    }
    ret = engine->Configure(VidIsHdr(false));
    if (ret != ERR_OK) {
        HILOGE("configure VidIsHdr failed");
        return ret;
    }
    ret = engine->Configure(VidEnableTemporalScale(false));
    if (ret != ERR_OK) {
        HILOGE("configure VidEnableTemporalScale failed");
        return ret;
    }
    ret = engine->Configure(VidSurfaceParam(ConvertToSurfaceParam(param)));
    if (ret != ERR_OK) {
        HILOGE("configure VidSurfaceParam failed");
        return ret;
    }
    return ERR_OK;
}

int32_t AbilityConnectionSession::GetSurfaceId(const SurfaceParams& param, std::string& surfaceId)
{
    HILOGI("called.");
    if (senderEngine_ == nullptr) {
        HILOGE("senderEngine_ Uninitialized.");
        return INVALID_PARAMETERS_ERR;
    }
    int32_t ret = ConfigEngineParam(senderEngine_, param);
    if (ret != ERR_OK) {
        HILOGE("config senderEngine param failed.");
        return ret;
    }

    ret = senderEngine_->Prepare();
    if (ret != ERR_OK) {
        HILOGE("error prepare senderEngine_");
        return ret;
    }

    TransChannelInfo info;
    if (GetStreamTransChannel(info) != ERR_OK) {
        HILOGE("senderEngine_ SetChannelListener failed");
        return INVALID_PARAMETERS_ERR;
    }

    HILOGI("SetChannelListener channelId is %{public}d, channelType is %{public}d",
        info.channelId, static_cast<int32_t>(info.channelType));
    senderEngine_->SetTransChannel(info.channelId, info.channelType);
    surfaceId = std::to_string(senderEngine_->GetSurface());
    return ERR_OK;
}

int32_t AbilityConnectionSession::SetSurfaceId(const std::string& surfaceId,
    const SurfaceParams& param)
{
    HILOGI("called.");
    if (recvEngine_ == nullptr) {
        HILOGE("recvEngine_ Uninitialized.");
        return INVALID_PARAMETERS_ERR;
    }
    int32_t ret = ConfigEngineParam(recvEngine_, param);
    if (ret != ERR_OK) {
        HILOGE("recvEngine_ ConfigEngineParam failed.");
        return ret;
    }

    uint64_t value = 0;
    auto result = std::from_chars(surfaceId.data(), surfaceId.data() + surfaceId.size(), value);
    if (result.ec != std::errc()) {
        HILOGE("Get value failed");
        return INVALID_PARAMETERS_ERR;
    }
    ret = recvEngine_->SetVideoSurface(value);
    if (ret != ERR_OK) {
        HILOGE("error set video surface!");
        return ret;
    }

    ret = recvEngine_->Prepare();
    if (ret != ERR_OK) {
        HILOGE("error prepare recvEngine_");
        return ret;
    }

    TransChannelInfo info;
    if (GetStreamTransChannel(info) != ERR_OK) {
        HILOGE("recvEngine_ SetChannelListener failed");
        return INVALID_PARAMETERS_ERR;
    }
    HILOGE("SetChannelListener channelId is %{public}d, channelType is %{public}d",
        info.channelId, static_cast<int32_t>(info.channelType));
    recvEngine_->SetChannelListener(info.channelId);
    pixelMapListener = std::make_shared<PixelMapListener>(shared_from_this());
    recvEngine_->SetEngineListener(pixelMapListener);
    return ERR_OK;
}

int32_t AbilityConnectionSession::UpdateSurfaceParam(const SurfaceParams& surfaceParam)
{
    SurfaceParam param = ConvertToSurfaceParam(surfaceParam);
    HILOGI("SurfaceParam rotate is %{public}d, filp is %{public}d.",
        static_cast<int32_t>(param.rotate), static_cast<int32_t>(param.filp));
    if (senderEngine_ != nullptr) {
        HILOGI("Update senderEngine_ SurfaceParam.");
        senderEngine_->SetSurfaceParam(param);
        return ERR_OK;
    }

    if (recvEngine_ != nullptr) {
        HILOGI("Update recvEngine_ SurfaceParam.");
        recvEngine_ -> OnRecvSurfaceParam(param);
        return ERR_OK;
    }

    HILOGE("senderEngine_ and recvEngine_ is nullptr!");
    return INVALID_PARAMETERS_ERR;
}

SurfaceParam AbilityConnectionSession::ConvertToSurfaceParam(const SurfaceParams& param)
{
    SurfaceParam surfaveParam;
    switch (param.rotation) {
        case SURFACE_ROTATE_NONE:
            surfaveParam.rotate = SurfaceRotate::ROTATE_NONE;
            break;
        case SURFACE_ROTATE_90:
            surfaveParam.rotate = SurfaceRotate::ROTATE_90;
            break;
        case SURFACE_ROTATE_180:
            surfaveParam.rotate = SurfaceRotate::ROTATE_180;
            break;
        case SURFACE_ROTATE_270:
            surfaveParam.rotate = SurfaceRotate::ROTATE_270;
            break;
        default:
            surfaveParam.rotate = SurfaceRotate::ROTATE_NONE;
            break;
    }

    switch (param.flip) {
        case FlipOptions::HORIZONTAL:
            surfaveParam.filp = SurfaceFilp::FLIP_H;
            break;
        case FlipOptions::VERTICAL:
            surfaveParam.filp = SurfaceFilp::FLIP_V;
            break;
        default:
            surfaveParam.filp = SurfaceFilp::FLIP_NONE;
            break;
    }

    return surfaveParam;
}

int32_t AbilityConnectionSession::GetStreamTransChannel(TransChannelInfo& info)
{
    std::shared_lock<std::shared_mutex> channelReadLock(transChannelMutex_);
    auto item = transChannels_.find(TransChannelType::STREAM);
    if (item != transChannels_.end() && item->second.isConnected) {
        info = item->second;
        return ERR_OK;
    }
    HILOGW("stream channel unconnected");
    item = transChannels_.find(TransChannelType::STREAM_BYTES);
    if (item != transChannels_.end() && item->second.isConnected) {
        info = item->second;
        return ERR_OK;
    }
    HILOGE("bytes stream channel unconnected");
    return INVALID_PARAMETERS_ERR;
}

int32_t AbilityConnectionSession::StartStream(int32_t streamId)
{
    HILOGI("called.");
    if (connectOption_.needSendStream && senderEngine_ != nullptr) {
        return StartSenderEngine();
    }

    if (connectOption_.needReceiveStream && recvEngine_ != nullptr) {
        return StartRecvEngine();
    }
    HILOGE("not config stream option or engine is null.");
    return INVALID_PARAMETERS_ERR;
}

int32_t AbilityConnectionSession::StartRecvEngine()
{
    HILOGI("recvEngine_ Start.");
    int32_t ret = recvEngine_->Start();
    if (ret != ERR_OK) {
        HILOGE("recvEngine_ start failed.");
        return ret;
    }
    return SendMessage("recvEngineStart", MessageType::RECEIVE_STREAM_START);
}

void AbilityConnectionSession::UpdateRecvEngineStatus()
{
    recvEngineState_ = EngineState::START;
}

int32_t AbilityConnectionSession::StartSenderEngine()
{
    HILOGI("senderEngine_ Start. recvEngineState_ is %{public}d", static_cast<int32_t>(recvEngineState_));
    if (recvEngineState_ != EngineState::START) {
        HILOGE("recvEngine not start");
        return RECEIVE_STREAM_NOT_START;
    }
    return senderEngine_->Start();
}

int32_t AbilityConnectionSession::StopStream(int32_t streamId)
{
    HILOGI("called.");
    if (connectOption_.needSendStream && senderEngine_ != nullptr) {
        HILOGI("senderEngine_ Stop.");
        return senderEngine_->Stop();
    }

    if (connectOption_.needReceiveStream && recvEngine_ != nullptr) {
        HILOGI("recvEngine_ Stop.");
        return recvEngine_->Stop();
    }
    return ERR_OK;
}

int32_t AbilityConnectionSession::RegisterEventCallback(const std::string& eventType,
    const std::shared_ptr<JsAbilityConnectionSessionListener>& listener)
{
    HILOGI("called.");
    std::unique_lock<std::shared_mutex> listenerWriteLock(listenerMutex_);
    listeners_[eventType] = listener;
    return ERR_OK;
}

int32_t AbilityConnectionSession::UnregisterEventCallback(const std::string& eventType)
{
    HILOGI("called.");
    std::unique_lock<std::shared_mutex> listenerWriteLock(listenerMutex_);
    listeners_.erase(eventType);
    return ERR_OK;
}

int32_t AbilityConnectionSession::RegisterEventCallback(
    const std::shared_ptr<IAbilityConnectionSessionListener>& listener)
{
    if (listener == nullptr) {
        HILOGE("listener empty");
        return INVALID_LISTENER;
    }
    std::unique_lock<std::shared_mutex> lock(sessionListenerMutex_);
    sessionListener_ = listener;
    return ERR_OK;
}

int32_t AbilityConnectionSession::UnregisterEventCallback()
{
    std::unique_lock<std::shared_mutex> lock(sessionListenerMutex_);
    sessionListener_ = nullptr;
    return ERR_OK;
}

int32_t AbilityConnectionSession::ExeuteEventCallback(const std::string& eventType, EventCallbackInfo& info)
{
    return ExeuteEventCallbackTemplate(eventType, info);
}

int32_t AbilityConnectionSession::ExeuteEventCallback(const std::string& eventType, CollaborateEventInfo& info)
{
    return ExeuteEventCallbackTemplate(eventType, info);
}

template <typename T>
int32_t AbilityConnectionSession::ExeuteEventCallbackTemplate(const std::string& eventType, T& info)
{
    HILOGI("called, eventType is %{public}s", eventType.c_str());
    std::shared_lock<std::shared_mutex> listenerReadLock(listenerMutex_);
    if (listeners_.empty()) {
        HILOGE("listeners_ is empty");
        return INVALID_PARAMETERS_ERR;
    }

    auto item = listeners_.find(eventType);
    if (item == listeners_.end()) {
        HILOGE("The event callback is not registered. event is %{public}s", eventType.c_str());
        return INVALID_PARAMETERS_ERR;
    }

    if constexpr (std::is_same_v<T, EventCallbackInfo>) {
        info.eventType = eventType;
    }
    auto eventCallback = item->second;
    if (eventCallback == nullptr) {
        HILOGE("eventCallback is nullptr");
        return INVALID_PARAMETERS_ERR;
    }
    eventCallback->CallJsMethod(info);
    return ERR_OK;
}

int32_t AbilityConnectionSession::InitChannels()
{
    HILOGI("called.");
    channelName_ = GetChannelName(sessionInfo_);
    channelListener_ = std::make_shared<CollabChannelListener>(shared_from_this());
    bool isClientChannel = direction_ == CollabrateDirection::COLLABRATE_SOURCE;
    int32_t ret = CreateChannel(channelName_, ChannelDataType::MESSAGE, TransChannelType::MESSAGE, isClientChannel);
    if (ret != ERR_OK) {
        HILOGE("create message channel failed!");
        return FAILED_TO_CREATE_MESSAGE_CHANNEL;
    }

    if (connectOption_.needSendData &&
        CreateChannel(channelName_, ChannelDataType::BYTES, TransChannelType::DATA, isClientChannel) != ERR_OK) {
        HILOGE("create data channel failed!");
        return FAILED_TO_CREATE_DATA_CHANNEL;
    }

    if ((connectOption_.needSendStream || connectOption_.needReceiveStream) &&
        CreateStreamChannel(channelName_, isClientChannel) != ERR_OK) {
        HILOGE("create stream channel failed!");
        return FAILED_TO_CREATE_STREAM_CHANNEL;
    }

    if (connectOption_.needSendFile && isClientChannel &&
        CreateChannel(channelName_, ChannelDataType::FILE, TransChannelType::SEND_FILE, true) != ERR_OK) {
        HILOGE("create send file channel failed!");
        return FAILED_TO_CREATE_SEND_FILE_CHANNEL;
    }

    if (connectOption_.needReceiveFile &&
        CreateChannel(channelName_, ChannelDataType::FILE, TransChannelType::RECEIVE_FILE, false) != ERR_OK) {
        HILOGE("create receive file channel failed!");
        return FAILED_TO_CREATE_RECEIVE_FILE_CHANNEL;
    }
    return ERR_OK;
}

int32_t AbilityConnectionSession::CreateStreamChannel(const std::string& channelName, bool isClientChannel)
{
    std::string streamChannelName = channelName + "stream";
    int32_t ret = CreateChannel(streamChannelName, ChannelDataType::BYTES, TransChannelType::STREAM_BYTES,
        isClientChannel);
    if (ret != ERR_OK) {
        HILOGE("init bytes channel failed!");
        return INVALID_PARAMETERS_ERR;
    }

    ret = CreateChannel(streamChannelName, ChannelDataType::VIDEO_STREAM, TransChannelType::STREAM, isClientChannel);
    if (ret != ERR_OK) {
        HILOGE("init bytes channel failed!");
        return INVALID_PARAMETERS_ERR;
    }
    return ERR_OK;
}

int32_t AbilityConnectionSession::CreateChannel(const std::string& channelName, const ChannelDataType& dataType,
    const TransChannelType& channelType, bool isClientChannel)
{
    HILOGI("called.");
    ChannelPeerInfo channelPeerInfo = { peerSocketName_, sessionInfo_.peerInfo_.deviceId };
    ChannelManager &channelManager = ChannelManager::GetInstance();
    int32_t channelId = isClientChannel ?
        channelManager.CreateClientChannel(channelName, dataType, channelPeerInfo) :
        channelManager.CreateServerChannel(channelName, dataType, channelPeerInfo);
    if (!channelManager.isValidChannelId(channelId)) {
        HILOGE("CreateChannel failed, channelId is %{public}d", channelId);
        return INVALID_PARAMETERS_ERR;
    }

    if (channelManager.RegisterChannelListener(channelId, channelListener_) != ERR_OK) {
        HILOGE("register channel listener failed, channelId is %{public}d", channelId);
        return INVALID_PARAMETERS_ERR;
    }
    
    std::unique_lock<std::shared_mutex> channelWriteLock(transChannelMutex_);
    TransChannelInfo channelInfo = {channelId, dataType, channelType, false};
    transChannels_.emplace(channelType, channelInfo);
    return ERR_OK;
}

std::string AbilityConnectionSession::GetChannelName(const AbilityConnectionSessionInfo& sessionInfo)
{
    PeerInfo localInfo = sessionInfo.localInfo_;
    PeerInfo peerInfo = sessionInfo.peerInfo_;
    bool isClientChannel = direction_ == CollabrateDirection::COLLABRATE_SOURCE;
    std::string input = isClientChannel ?
        localInfo.moduleName + localInfo.abilityName + peerInfo.moduleName + peerInfo.abilityName :
        peerInfo.moduleName + peerInfo.abilityName + localInfo.moduleName + localInfo.abilityName;

    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((const unsigned char*)input.c_str(), input.length(), hash);

    std::stringstream hashStr;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        hashStr << std::hex << std::setw(HEX_WIDTH) << std::setfill(FILL_CHAR) << (int)hash[i];
    }

    std::string channelName = hashStr.str().substr(0, CHANNEL_NAME_LENGTH);
    return channelName;
}

int32_t AbilityConnectionSession::ConnectChannels()
{
    HILOGI("parallel connect channels");
    std::vector<std::future<int32_t>> futures;
    // message
    futures.emplace_back(std::async(std::launch::async, [this]() -> int32_t {
        if (ConnectTransChannel(TransChannelType::MESSAGE) != ERR_OK) {
            HILOGE("connect message channel failed.");
            return CONNECT_MESSAGE_CHANNEL_FAILED;
        }
        return ERR_OK;
    }));
    // data
    if (connectOption_.needSendData) {
        futures.emplace_back(std::async(std::launch::async, [this]() -> int32_t {
            if (ConnectTransChannel(TransChannelType::DATA) != ERR_OK) {
                HILOGE("connect data channel failed.");
                return CONNECT_DATA_CHANNEL_FAILED;
            }
            return ERR_OK;
        }));
    }
    // stream
    if (connectOption_.needSendStream || connectOption_.needReceiveStream) {
        ConnectStreamChannel();
        futures.emplace_back(std::async(std::launch::async, [this]() -> int32_t {
            if (ConnectTransChannel(TransChannelType::STREAM_BYTES) != ERR_OK) {
                HILOGE("connect stream channel failed.");
                return CONNECT_STREAM_CHANNEL_FAILED;
            }
            return ERR_OK;
        }));
    }
    // file
    if (connectOption_.needSendFile && ConnectTransChannel(TransChannelType::SEND_FILE) != ERR_OK) {
        HILOGE("connect send file channel failed.");
        return CONNECT_SEND_FILE_CHANNEL_FAILED;
    }
    // wait for task
    for (auto&& future : futures) {
        int32_t result = future.get();
        if (result != ERR_OK) {
            return result;
        }
    }
    return ERR_OK;
}

int32_t AbilityConnectionSession::ConnectTransChannel(const TransChannelType channelType)
{
    TransChannelInfo info;
    int32_t ret = GetTransChannelInfo(channelType, info);
    if (ret != ERR_OK) {
        HILOGE("stream channel not exits!");
        return STREAM_CHANNEL_NOT_EXITS;
    }

    ret = ChannelManager::GetInstance().ConnectChannel(info.channelId);
    if (ret != ERR_OK) {
        HILOGE("connect channel failed. ret is %{public}d", ret);
        return ret;
    }
    HILOGI("connect channel success, channel type is %{public}d", static_cast<int32_t>(channelType));
    UpdateTransChannelStatus(info.channelId, true);
    return ERR_OK;
}

int32_t AbilityConnectionSession::ConnectStreamChannel()
{
    HILOGI("called.");
    if (!connectOption_.needSendStream && !connectOption_.needReceiveStream) {
        HILOGI("Streaming is not required.");
        return ERR_OK;
    }

    TransChannelInfo info;
    int32_t ret = GetTransChannelInfo(TransChannelType::STREAM, info);
    if (ret != ERR_OK) {
        HILOGE("stream channel not exits!");
        return INVALID_PARAMETERS_ERR;
    }

    if (info.isConnected) {
        HILOGE("stream channel has connected.");
        return ERR_OK;
    }

    if (direction_ != CollabrateDirection::COLLABRATE_SOURCE) {
        HILOGI("notify source connect stream channel.");
        SendMessage("WIFI_OPEN", MessageType::WIFI_OPEN);
        return ERR_OK;
    }

    std::thread task([this, info]() {
        DoConnectStreamChannel(info.channelId);
    });
    task.detach();
    return ERR_OK;
}

int32_t AbilityConnectionSession::DoConnectStreamChannel(int32_t channelId)
{
    HILOGI("called.");
    int32_t ret = ChannelManager::GetInstance().ConnectChannel(channelId);
    if (ret != ERR_OK) {
        HILOGE("stream channel bind failed, ret is %{public}d", ret);
        return ret;
    }
    HILOGI("stream channel bind success");
    UpdateTransChannelStatus(channelId, true);

    if (recvEngine_ == nullptr) {
        HILOGI("notify peer update recvEngine channel.");
        SendMessage("updateRecvEngineTransChannel", MessageType::UPDATE_RECV_ENGINE_CHANNEL);
        return ERR_OK;
    }
    UpdateRecvEngineTransChannel();
    return ERR_OK;
}

int32_t AbilityConnectionSession::GetTransChannelInfo(const TransChannelType& type, TransChannelInfo& info)
{
    HILOGI("called.");
    std::shared_lock<std::shared_mutex> channelReadLock(transChannelMutex_);
    auto item = transChannels_.find(type);
    if (item == transChannels_.end()) {
        return INVALID_PARAMETERS_ERR;
    }
    info = item->second;
    return ERR_OK;
}

void AbilityConnectionSession::OnChannelConnect(int32_t channelId)
{
    HILOGI("called. channelId is %{public}d", channelId);
    if (!IsVaildChannel(channelId)) {
        HILOGE("is vaild channelId");
        return;
    }

    UpdateTransChannelStatus(channelId, true);
    if (IsAllChannelConnected() && !connectOption_.HasFileTransfer()) {
        HandleSessionConnect();
    }
}

void AbilityConnectionSession::HandleSessionConnect()
{
    HILOGI("called.");
    std::unique_lock<std::shared_mutex> sessionStatusWriteLock(sessionMutex_);
    if (sessionStatus_ == SessionStatus::CONNECTED) {
        HILOGI("session has connected.");
        return;
    }
    sessionStatus_ = SessionStatus::CONNECTED;

    std::shared_ptr<IAbilityConnectionSessionListener> listener;
    {
        std::shared_lock<std::shared_mutex> lock(sessionListenerMutex_);
        listener = sessionListener_;
    }
    if (listener) {
        HILOGI("handler sessionListener");
        listener->OnConnect(sessionId_);
    } else {
        EventCallbackInfo callbackInfo;
        callbackInfo.sessionId = sessionId_;
        ExeuteEventCallback(EVENT_CONNECT, callbackInfo);
    }
}

void AbilityConnectionSession::UpdateTransChannelStatus(int32_t channelId, bool isConnected)
{
    std::unique_lock<std::shared_mutex> channelWriteLock(transChannelMutex_);
    for (auto& iter : transChannels_) {
        if (iter.second.channelId == channelId) {
            HILOGI("transType is %{public}d.", static_cast<int32_t>(iter.second.transType));
            iter.second.isConnected = isConnected;
        }
    }
}

bool AbilityConnectionSession::IsAllChannelConnected()
{
    HILOGD("called.");
    std::shared_lock<std::shared_mutex> channelReadLock(transChannelMutex_);
    for (auto& iter : transChannels_) {
        TransChannelInfo info = iter.second;
        if (!info.isConnected && info.transType != TransChannelType::STREAM) {
            HILOGI("transType is %{public}d.", static_cast<int32_t>(info.transType));
            return false;
        }
    }
    HILOGI("AllChannelConnected.");
    return true;
}

void AbilityConnectionSession::OnChannelClosed(int32_t channelId, const ShutdownReason& reason)
{
    HILOGI("called. channelId is %{public}d", channelId);
    if (!IsVaildChannel(channelId)) {
        HILOGE("is vaild channelId");
        return;
    }

    if (!IsConnected()) {
        HILOGE("session is not connected.");
        return;
    }

    HILOGI("notidy app disconnect");
    Disconnect();

    std::shared_ptr<IAbilityConnectionSessionListener> listener;
    {
        std::shared_lock<std::shared_mutex> lock(sessionListenerMutex_);
        listener = sessionListener_;
    }
    if (listener) {
        HILOGI("handler sessionListener");
        listener->OnDisConnect(sessionId_);
    } else {
        EventCallbackInfo callbackInfo;
        callbackInfo.sessionId = sessionId_;
        callbackInfo.reason = ConvertToDisconnectReason(reason);
        ExeuteEventCallback(EVENT_DISCONNECT, callbackInfo);
    }
}

DisconnectReason AbilityConnectionSession::ConvertToDisconnectReason(const ShutdownReason& reason)
{
    HILOGI("Shutdown reason code is %{public}d.", static_cast<int32_t>(reason));
    switch (reason) {
        case ShutdownReason::SHUTDOWN_REASON_PEER:
            return DisconnectReason::PEER_APP_EXIT;
        default:
            return DisconnectReason::NETWORK_DISCONNECTED;
    }
}

void AbilityConnectionSession::OnMessageReceived(int32_t channelId, const std::shared_ptr<AVTransDataBuffer> dataBuffer)
{
    HILOGI("called.");
    if (!IsVaildChannel(channelId)) {
        return;
    }
    uint8_t *data = dataBuffer->Data();
    auto headerPara = MessageDataHeader::Deserialize(data, dataBuffer->Size());
    if (!headerPara) {
        HILOGE("read session header from buffer failed");
        return;
    }
    std::string msg(reinterpret_cast<const char *>(data + MessageDataHeader::HEADER_LEN),
        dataBuffer->Size() - MessageDataHeader::HEADER_LEN);
    HILOGI("headerPara type is %{public}d", headerPara->dataType_);
    // common handler
    auto handler = [this, msg](uint32_t dataType) {
        auto iter = messageHandlerMap_.find(dataType);
        if (iter != messageHandlerMap_.end()) {
            iter->second(msg);
        } else {
            HILOGE("unhandled code!");
        }
    };

    if (headerPara->dataType_ == static_cast<uint32_t>(MessageType::CONNECT_FILE_CHANNEL)) {
        TransChannelInfo info;
        int32_t ret = GetTransChannelInfo(TransChannelType::SEND_FILE, info);
        if (ret != ERR_OK) {
            HILOGI("send file channel now not exists!");
            std::thread(handler, headerPara->dataType_).detach();  // only connect file need async
        }
    } else {
        handler(headerPara->dataType_);  // keep sync
    }
}

void AbilityConnectionSession::OnSendFile(const int32_t channelId, const FileInfo& info)
{
    HILOGI("called.");
    if (!IsVaildChannel(channelId)) {
        return;
    }

    std::shared_ptr<IAbilityConnectionSessionListener> listener;
    {
        std::shared_lock<std::shared_mutex> lock(sessionListenerMutex_);
        listener = sessionListener_;
    }
    if (listener == nullptr) {
        HILOGE("listener is nullptr");
        return;
    }
    listener->OnSendFile(sessionId_, info);
}

void AbilityConnectionSession::OnRecvFile(const int32_t channelId, const FileInfo& info)
{
    HILOGI("called.");
    if (!IsVaildChannel(channelId)) {
        return;
    }

    std::shared_ptr<IAbilityConnectionSessionListener> listener;
    {
        std::shared_lock<std::shared_mutex> lock(sessionListenerMutex_);
        listener = sessionListener_;
    }
    if (listener == nullptr) {
        HILOGE("listener is nullptr");
        return;
    }
    listener->OnRecvFile(sessionId_, info);
}

const char* AbilityConnectionSession::GetRecvPath(const int32_t channelId)
{
    HILOGI("called.");
    if (!IsVaildChannel(channelId)) {
        return nullptr;
    }

    std::shared_ptr<IAbilityConnectionSessionListener> listener;
    {
        std::shared_lock<std::shared_mutex> lock(sessionListenerMutex_);
        listener = sessionListener_;
    }
    if (listener == nullptr) {
        HILOGE("listener is nullptr");
        return nullptr;
    }
    return listener->GetRecvPath(sessionId_);
}

void AbilityConnectionSession::ExeuteMessageEventCallback(const std::string msg)
{
    HILOGI("called.");
    std::shared_ptr<IAbilityConnectionSessionListener> listener;
    {
        std::shared_lock<std::shared_mutex> lock(sessionListenerMutex_);
        listener = sessionListener_;
    }
    // bi-channel need wait
    {
        std::unique_lock<std::mutex> lock(connectionMutex_);
        bool isConnected = connectionCondition_.wait_for(
            lock,
            std::chrono::seconds(WAIT_FOR_CONNECT),
            [this]() { return sessionStatus_ == SessionStatus::CONNECTED; }
        );
        if (!isConnected) {
            HILOGE("Wait for channel connection timed out after %{public}d seconds.", WAIT_FOR_CONNECT);
            return;
        }
    }
    HILOGI("start to add msg callback to handler");
    if (listener != nullptr) {
        HILOGI("handler sessionListener");
        auto func = [listener, msg, this]() {
            listener->OnMessage(sessionId_, msg);
        };
        eventHandler_->PostTask(func, AppExecFwk::EventQueue::Priority::LOW);
    } else {
        EventCallbackInfo callbackInfo;
        callbackInfo.sessionId = sessionId_;
        callbackInfo.msg = msg;
        auto func = [callbackInfo, this]() mutable {
            ExeuteEventCallback(EVENT_RECEIVE_MESSAGE, callbackInfo);
        };
        eventHandler_->PostTask(func, AppExecFwk::EventQueue::Priority::LOW);
    }
}

void AbilityConnectionSession::UpdateRecvEngineTransChannel()
{
    HILOGI("called.");
    if (recvEngine_ == nullptr) {
        HILOGE("recvEngine_ is nullptr.");
        return;
    }

    TransChannelInfo info;
    int32_t ret = GetTransChannelInfo(TransChannelType::STREAM, info);
    if (ret != ERR_OK) {
        HILOGI("not find stream chennel.");
        return;
    }
    recvEngine_->SetChannelListener(info.channelId);
    SendMessage("updateSenderEngineTransChannel", MessageType::UPDATE_SENDER_ENGINE_CHANNEL);
}

void AbilityConnectionSession::UpdateSenderEngineTransChannel()
{
    HILOGI("called.");
    if (senderEngine_ == nullptr) {
        HILOGE("senderEngine_ is nullptr.");
        return;
    }

    TransChannelInfo info;
    int32_t ret = GetTransChannelInfo(TransChannelType::STREAM, info);
    if (ret != ERR_OK) {
        HILOGI("not find stream chennel.");
        return;
    }
    HILOGI("SetChannelListener channelId is %{public}d, channelType is %{public}d",
        info.channelId, static_cast<int32_t>(info.channelType));
    senderEngine_->SetTransChannel(info.channelId, info.channelType);
}

void AbilityConnectionSession::ConnectFileChannel(const std::string& peerSocketName)
{
    HILOGI("called.");
    peerSocketName_ = peerSocketName;
    int32_t ret = CreateChannel(channelName_, ChannelDataType::FILE, TransChannelType::SEND_FILE, true);
    if (ret != ERR_OK) {
        HILOGE("create send file channel failed!");
        return;
    }

    ret = ConnectTransChannel(TransChannelType::SEND_FILE);
    if (ret != ERR_OK) {
        HILOGI("connect file chennel failed.");
        SendMessage("FILE_CHANNEL_CONNECT_FAILED", MessageType::FILE_CHANNEL_CONNECT_FAILED);
        return;
    }
    SendMessage("FILE_CHANNEL_CONNECT_SUCCESS", MessageType::FILE_CHANNEL_CONNECT_SUCCESS);
    HandleSessionConnect();
}

void AbilityConnectionSession::OnRecvPixelMap(const std::shared_ptr<Media::PixelMap>& pixelMap)
{
    HILOGI("called.");
    if (pixelMap == nullptr) {
        HILOGE("pixelMap is nullptr.");
    }

    EventCallbackInfo callbackInfo;
    callbackInfo.sessionId = sessionId_;
    callbackInfo.image = pixelMap;
    ExeuteEventCallback(EVENT_RECEIVE_IMAGE, callbackInfo);
}

void AbilityConnectionSession::OnBytesReceived(int32_t channelId, const std::shared_ptr<AVTransDataBuffer> dataBuffer)
{
    HILOGI("called.");
    if (!IsVaildChannel(channelId)) {
        return;
    }

    if (IsStreamBytesChannel(channelId)) {
        HILOGE("is stream bytes channel, no need to send.");
        return;
    }
    // bi-channel need wait
    {
        std::unique_lock<std::mutex> lock(connectionMutex_);
        bool isConnected = connectionCondition_.wait_for(
            lock,
            std::chrono::seconds(WAIT_FOR_CONNECT),
            [this]() { return sessionStatus_ == SessionStatus::CONNECTED; }
        );
        if (!isConnected) {
            HILOGE("Wait for channel connection timed out after %{public}d seconds.", WAIT_FOR_CONNECT);
            return;
        }
    }
    std::shared_ptr<IAbilityConnectionSessionListener> listener;
    {
        std::shared_lock<std::shared_mutex> lock(sessionListenerMutex_);
        listener = sessionListener_;
    }
    if (listener != nullptr) {
        HILOGI("handler sessionListener");
        listener->OnData(sessionId_, dataBuffer);
    } else {
        EventCallbackInfo callbackInfo;
        callbackInfo.sessionId = sessionId_;
        callbackInfo.data = dataBuffer;
        ExeuteEventCallback(EVENT_RECEIVE_DATA, callbackInfo);
    }
}

void AbilityConnectionSession::OnError(int32_t channelId, const int32_t errorCode)
{
    HILOGI("error receive");
    if (!IsVaildChannel(channelId)) {
        return;
    }
    std::shared_ptr<IAbilityConnectionSessionListener> listener;
    {
        std::shared_lock<std::shared_mutex> lock(sessionListenerMutex_);
        listener = sessionListener_;
    }
    if (listener != nullptr) {
        HILOGI("handler sessionListener");
        listener->OnError(sessionId_, errorCode);
    } else {
        CollaborateEventInfo info;
        info.eventType = CollaborateEventType::SEND_FAILURE;
        info.sessionId = sessionId_;
        ExeuteEventCallback(EVENT_COLLABORATE, info);
    }
}

bool AbilityConnectionSession::IsStreamBytesChannel(const int32_t channelId)
{
    TransChannelInfo transChannelInfo;
    int32_t ret = GetTransChannelInfo(TransChannelType::STREAM_BYTES, transChannelInfo);
    if (ret != ERR_OK) {
        HILOGE("stream bytes channel not exit!");
        return false;
    }

    return transChannelInfo.channelId == channelId;
}

bool AbilityConnectionSession::IsVaildChannel(const int32_t channelId)
{
    HILOGD("called");
    std::shared_lock<std::shared_mutex> channelReadLock(transChannelMutex_);
    if (transChannels_.empty()) {
        HILOGE("transChannels_ is empty");
        return false;
    }
    
    for (auto& iter : transChannels_) {
        if (iter.second.channelId == channelId) {
            return true;
        }
    }

    return false;
}

void AbilityConnectionSession::SetTimeOut(int32_t time)
{
    HILOGD("called.");
    auto func = [this]() {
        Release();
    };
    if (eventHandler_ == nullptr) {
        HILOGE("eventHandler_ is nullptr");
        return;
    }
    eventHandler_->PostTask(func, CONNECT_SESSION_TIMEOUT_END_TASK, time);
}

void AbilityConnectionSession::RemoveTimeout()
{
    HILOGD("called.");
    if (eventHandler_ == nullptr) {
        HILOGE("eventHandler_ is nullptr");
        return;
    }
    eventHandler_->RemoveTask(CONNECT_SESSION_TIMEOUT_END_TASK);
}

bool AbilityConnectionSession::IsConnecting()
{
    std::shared_lock<std::shared_mutex> sessionStatusReadLock(sessionMutex_);
    return sessionStatus_ == SessionStatus::CONNECTING;
}

bool AbilityConnectionSession::IsConnected()
{
    std::shared_lock<std::shared_mutex> sessionStatusReadLock(sessionMutex_);
    return sessionStatus_ == SessionStatus::CONNECTED;
}

void AbilityConnectionSession::FinishSessionConnect()
{
    HILOGI("finish %{public}d connect callback", sessionId_);
    std::lock_guard<std::shared_mutex> sessionStatusLock(sessionMutex_);
    sessionStatus_ = SessionStatus::CONNECTED;
    connectionCondition_.notify_all();
}

void AbilityConnectionSession::ExeuteConnectCallback(const ConnectResult& result)
{
    HILOGI("called.");
    if (eventHandler_ == nullptr) {
        HILOGE("eventHandler_ is nullptr");
        return;
    }

    auto task = [this, result]() {
        HILOGI("execute connect callback task.");
        if (connectCallback_ == nullptr) {
            HILOGE("connect callback is nullptr.");
            return;
        }
        auto callback = std::move(connectCallback_);
        connectCallback_ = nullptr;
        callback(result);
        if (!result.isConnected) {
            Release();
        }
    };
    eventHandler_->PostTask(task,
        "ExeuteConnectCallback", 0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
}

AbilityConnectionSession::CollabChannelListener::CollabChannelListener(
    const std::shared_ptr<AbilityConnectionSession>& abilityConnectionSession)
    : abilityConnectionSession_(abilityConnectionSession)
{
}

void AbilityConnectionSession::CollabChannelListener::OnConnect(const int32_t channelId) const
{
    HILOGI("called.");
    std::shared_ptr<AbilityConnectionSession> abilityConnectionSession = abilityConnectionSession_.lock();
    if (abilityConnectionSession == nullptr) {
        HILOGE("abilityConnectionSession is null");
        return;
    }

    abilityConnectionSession->OnChannelConnect(channelId);
}

void AbilityConnectionSession::CollabChannelListener::OnDisConnect(const int32_t channelId,
    const ShutdownReason& reason) const
{
    HILOGI("called.");
    std::shared_ptr<AbilityConnectionSession> abilityConnectionSession = abilityConnectionSession_.lock();
    if (abilityConnectionSession == nullptr) {
        HILOGE("abilityConnectionSession is null");
        return;
    }

    abilityConnectionSession->OnChannelClosed(channelId, reason);
}

void AbilityConnectionSession::CollabChannelListener::OnMessage(const int32_t channelId,
    const std::shared_ptr<AVTransDataBuffer>& buffer) const
{
    HILOGI("called.");
    std::shared_ptr<AbilityConnectionSession> abilityConnectionSession = abilityConnectionSession_.lock();
    if (abilityConnectionSession == nullptr) {
        HILOGE("abilityConnectionSession is null");
        return;
    }

    abilityConnectionSession->OnMessageReceived(channelId, buffer);
}

void AbilityConnectionSession::CollabChannelListener::OnBytes(const int32_t channelId,
    const std::shared_ptr<AVTransDataBuffer>& buffer) const
{
    HILOGI("called.");
    std::shared_ptr<AbilityConnectionSession> abilityConnectionSession = abilityConnectionSession_.lock();
    if (abilityConnectionSession == nullptr) {
        HILOGE("abilityConnectionSession is null");
        return;
    }

    abilityConnectionSession->OnBytesReceived(channelId, buffer);
}

void AbilityConnectionSession::CollabChannelListener::OnStream(const int32_t channelId,
    const std::shared_ptr<AVTransStreamData>& sendData) const
{
}

void AbilityConnectionSession::CollabChannelListener::OnError(const int32_t channelId, const int32_t errorCode) const
{
    HILOGI("called.");
    if (auto abilityConnectionSession = abilityConnectionSession_.lock()) {
        abilityConnectionSession->OnError(channelId, errorCode);
    }
}

void AbilityConnectionSession::CollabChannelListener::OnSendFile(const int32_t channelId, const FileInfo& info) const
{
    HILOGI("called.");
    std::shared_ptr<AbilityConnectionSession> abilityConnectionSession = abilityConnectionSession_.lock();
    if (abilityConnectionSession == nullptr) {
        HILOGE("abilityConnectionSession is null");
        return;
    }

    abilityConnectionSession->OnSendFile(channelId, info);
}

void AbilityConnectionSession::CollabChannelListener::OnRecvFile(const int32_t channelId, const FileInfo& info) const
{
    HILOGI("called.");
    std::shared_ptr<AbilityConnectionSession> abilityConnectionSession = abilityConnectionSession_.lock();
    if (abilityConnectionSession == nullptr) {
        HILOGE("abilityConnectionSession is null");
        return;
    }

    abilityConnectionSession->OnRecvFile(channelId, info);
}

const char* AbilityConnectionSession::CollabChannelListener::GetRecvPath(const int32_t channelId) const
{
    HILOGI("called.");
    std::shared_ptr<AbilityConnectionSession> abilityConnectionSession = abilityConnectionSession_.lock();
    if (abilityConnectionSession == nullptr) {
        HILOGE("abilityConnectionSession is null");
        return nullptr;
    }

    return abilityConnectionSession->GetRecvPath(channelId);
}

AbilityConnectionSession::PixelMapListener::PixelMapListener(
    const std::shared_ptr<AbilityConnectionSession>& session) : abilityConnectionSession_(session)
{
}

void AbilityConnectionSession::PixelMapListener::OnRecvPixelMap(const std::shared_ptr<Media::PixelMap>& pixelMap)
{
    HILOGI("called.");
    std::shared_ptr<AbilityConnectionSession> abilityConnectionSession = abilityConnectionSession_.lock();
    if (abilityConnectionSession == nullptr) {
        HILOGE("abilityConnectionSession is null");
        return;
    }

    abilityConnectionSession->OnRecvPixelMap(pixelMap);
}

void AbilityConnectionSession::PixelMapListener::OnRecvSurfaceParam(const SurfaceParam& param)
{
    HILOGI("called.");
}
}  // namespace DistributedCollab
}  // namespace OHOS