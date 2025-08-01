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

#include "dsched_collab_manager.h"

#include <chrono>
#include <random>
#include <ctime>
#include <sys/prctl.h>

#include "cJSON.h"

#include "distributed_sched_utils.h"
#include "dsched_transport_softbus_adapter.h"
#include "dtbschedmgr_device_info_storage.h"
#include "dtbschedmgr_log.h"
#include "dtbcollabmgr_log.h"
#include "ipc_skeleton.h"
#include "mission/distributed_bm_storage.h"
#include "mission/wifi_state_adapter.h"
#include "multi_user_manager.h"

namespace OHOS {
namespace DistributedSchedule {
namespace {
const std::string TAG = "DSchedCollabManager";
const std::string DSCHED_COLLAB_MANAGER = "dsched_collab_manager";
constexpr static int32_t NO_NEED_TO_CHECK_ID = -1;
const int32_t RANDOM_STRING_LENGTH = 20;
std::map<int32_t, std::string> CMDDATA = {
    {MIN_CMD, "MIN_CMD"},
    {SINK_GET_VERSION_CMD, "SINK_GET_VERSION_CMD"},
    {SOURCE_GET_VERSION_CMD, "SOURCE_GET_VERSION_CMD"},
    {SINK_START_CMD, "SINK_START_CMD"},
    {NOTIFY_RESULT_CMD, "NOTIFY_RESULT_CMD"},
    {DISCONNECT_CMD, "DISCONNECT_CMD"},
    {MAX_CMD, "MAX_CMD"},
};
}

IMPLEMENT_SINGLE_INSTANCE(DSchedCollabManager);

DSchedCollabManager::DSchedCollabManager()
{
}

DSchedCollabManager::~DSchedCollabManager()
{
    HILOGI("DSchedCollabManager delete");
    UnInit();
}

static int32_t CheckSynergisticRelation(const CollabInfo *sourceInfo, const CollabInfo *sinkInfo)
{
    if (sourceInfo == nullptr || sinkInfo == nullptr) {
        HILOGE("Invalid parameters.");
        return INVALID_PARAMETERS_ERR;
    }
    return DSchedCollabManager::GetInstance().CheckCollabRelation(sourceInfo, sinkInfo);
}

IFeatureAbilityRelationChecker iAbilityRelationChecker = {
    .CheckCollabRelation = CheckSynergisticRelation
};

void DSchedCollabManager::Init()
{
    HILOGI("Init DSchedCollabManager start");
    if (eventHandler_ != nullptr) {
        HILOGI("DSchedCollabManager already inited, end.");
        return;
    }
    DSchedTransportSoftbusAdapter::GetInstance().InitChannel();
    softbusListener_ = std::make_shared<DSchedCollabManager::SoftbusListener>();
    DSchedTransportSoftbusAdapter::GetInstance().RegisterListener(SERVICE_TYPE_COLLAB, softbusListener_);
    int32_t ret =  RegisterRelationChecker(&iAbilityRelationChecker);
    if (ret != ERR_OK) {
        HILOGE("RegisterRelationChecker failed, ret: %{public}d", ret);
        return;
    }
    eventThread_ = std::thread(&DSchedCollabManager::StartEvent, this);
    std::unique_lock<std::mutex> lock(eventMutex_);
    eventCon_.wait(lock, [this] {
        return eventHandler_ != nullptr;
    });
    HILOGI("Init DSchedCollabManager end");
}

int32_t DSchedCollabManager::CheckCollabRelation(const CollabInfo *sourceInfo, const CollabInfo *sinkInfo)
{
    HILOGI("called");
    if (collabs_.empty()) {
        HILOGE("No collab in progress.");
        return INVALID_PARAMETERS_ERR;
    }
    DSchedCollabInfo collabInfo;
    for (auto iter = collabs_.begin(); iter != collabs_.end(); iter++) {
        if (iter->second == nullptr) {
            HILOGE("collab is nullptr");
            continue;
        }
        collabInfo = iter->second->GetCollabInfo();
        HILOGI("collabInfo: %{public}s", collabInfo.ToString().c_str());
        if (CheckSrcCollabRelation(sourceInfo, &collabInfo) == ERR_OK &&
            (sinkInfo->pid == NO_NEED_TO_CHECK_ID || CheckSinkCollabRelation(sinkInfo, &collabInfo) == ERR_OK)) {
            HILOGI("the collaboration relationship is matched successfully.");
            return ERR_OK;
        }
    }
    HILOGE("failed to match the collaboration relationship.");
    return INVALID_PARAMETERS_ERR;
}

int32_t DSchedCollabManager::CheckSrcCollabRelation(const CollabInfo *sourceInfo, const DSchedCollabInfo *collabInfo)
{
    HILOGI("called");
    if (sourceInfo == nullptr) {
        HILOGE("Invalid sourceInfo.");
        return INVALID_PARAMETERS_ERR;
    }
    if (collabInfo == nullptr) {
        HILOGE("collab is nullptr");
        return INVALID_PARAMETERS_ERR;
    }
    if (std::string(sourceInfo->deviceId) == collabInfo->srcUdid_ &&
        sourceInfo->pid == collabInfo->srcInfo_.pid_ &&
        static_cast<int32_t>(sourceInfo->tokenId) == collabInfo->srcInfo_.accessToken_ &&
        sourceInfo->userId == collabInfo->srcAccountInfo_.userId) {
        HILOGI("the collaboration relationship is matched successfully.");
        return ERR_OK;
    }
    HILOGI("Try to verify the info on the other side.");
    if (std::string(sourceInfo->deviceId) == collabInfo->sinkUdid_ &&
        sourceInfo->pid == collabInfo->sinkInfo_.pid_ &&
        static_cast<int32_t>(sourceInfo->tokenId) == collabInfo->sinkInfo_.accessToken_ &&
        sourceInfo->userId == collabInfo->sinkUserId_) {
        HILOGI("the collaboration relationship is matched successfully.");
        return ERR_OK;
    }
    HILOGW("deviceId: %{public}s, pid: %{public}d, tokenId: %{public}d, userId: %{public}d, srcUdid: %{public}s",
        GetAnonymStr(std::string(sourceInfo->deviceId)).c_str(),
        sourceInfo->pid, static_cast<int32_t>(sourceInfo->tokenId),
        sourceInfo->userId, GetAnonymStr(collabInfo->srcUdid_).c_str());
    return INVALID_PARAMETERS_ERR;
}

int32_t DSchedCollabManager::CheckSinkCollabRelation(const CollabInfo *sinkInfo, const DSchedCollabInfo *collabInfo)
{
    HILOGI("called");
    if (sinkInfo == nullptr) {
        HILOGE("Invalid sinkInfo.");
        return INVALID_PARAMETERS_ERR;
    }
    if (collabInfo == nullptr) {
        HILOGE("collab is nullptr");
        return INVALID_PARAMETERS_ERR;
    }
    if (std::string(sinkInfo->deviceId) == collabInfo->sinkUdid_ &&
        sinkInfo->pid == collabInfo->sinkInfo_.pid_ &&
        static_cast<int32_t>(sinkInfo->tokenId) == collabInfo->sinkInfo_.accessToken_ &&
        sinkInfo->userId == collabInfo->sinkUserId_) {
        HILOGI("the collaboration relationship is matched successfully.");
        return ERR_OK;
    }
    HILOGI("Try to verify the info on the other side.");
    if (std::string(sinkInfo->deviceId) == collabInfo->srcUdid_ &&
        sinkInfo->pid == collabInfo->srcInfo_.pid_ &&
        static_cast<int32_t>(sinkInfo->tokenId) == collabInfo->srcInfo_.accessToken_ &&
        sinkInfo->userId == collabInfo->srcAccountInfo_.userId) {
        HILOGI("the collaboration relationship is matched successfully.");
        return ERR_OK;
    }
    HILOGW("deviceId: %{public}s, pid: %{public}d, tokenId: %{public}d, userId: %{public}d, sinkUdid: %{public}s",
        GetAnonymStr(std::string(sinkInfo->deviceId)).c_str(),
        sinkInfo->pid, static_cast<int32_t>(sinkInfo->tokenId),
        sinkInfo->userId, GetAnonymStr(collabInfo->sinkUdid_).c_str());
    return INVALID_PARAMETERS_ERR;
}

void DSchedCollabManager::StartEvent()
{
    HILOGI("called");
    prctl(PR_SET_NAME, DSCHED_COLLAB_MANAGER.c_str());
    auto runner = AppExecFwk::EventRunner::Create(false);
    {
        std::lock_guard<std::mutex> lock(eventMutex_);
        eventHandler_ = std::make_shared<OHOS::AppExecFwk::EventHandler>(runner);
    }
    eventCon_.notify_one();
    if (runner == nullptr) {
        HILOGE("runner is nullptr");
        return;
    }
    runner->Run();
    HILOGI("StartEvent end");
}

void DSchedCollabManager::UnInit()
{
    HILOGI("UnInit start");
    DSchedTransportSoftbusAdapter::GetInstance().UnregisterListener(SERVICE_TYPE_COLLAB, softbusListener_);
    DSchedTransportSoftbusAdapter::GetInstance().ReleaseChannel();
    collabs_.clear();

    if (eventHandler_ != nullptr) {
        eventHandler_->GetEventRunner()->Stop();
        eventThread_.join();
        eventHandler_ = nullptr;
    } else {
        HILOGE("eventHandler is nullptr");
    }
    HILOGI("UnInit end");
}

int32_t DSchedCollabManager::GetSinkCollabVersion(DSchedCollabInfo &info)
{
    HILOGI("called, sessionId is: %{public}s", info.ToString().c_str());
    if (info.srcCollabSessionId_ < 0 || info.sinkInfo_.deviceId_.empty() || info.srcClientCB_ == nullptr) {
        HILOGE("invalid parameters.");
        return INVALID_PARAMETERS_ERR;
    }
    if (!DtbschedmgrDeviceInfoStorage::GetInstance().GetLocalDeviceId(info.srcInfo_.deviceId_)) {
        HILOGE("get local deviceId failed!");
        return FIND_LOCAL_DEVICEID_ERR;
    }
    if (DtbschedmgrDeviceInfoStorage::GetInstance().GetDeviceInfoById(info.sinkInfo_.deviceId_) == nullptr) {
        HILOGW("subsequent queries on application level binding relationships");
    }
    auto func = [this, info]() {
        HandleGetSinkCollabVersion(info);
    };
    if (eventHandler_ == nullptr) {
        HILOGE("eventHandler is nullptr");
        return INVALID_PARAMETERS_ERR;
    }
    eventHandler_->PostTask(func);
    return ERR_OK;
}
 
void DSchedCollabManager::HandleGetSinkCollabVersion(const DSchedCollabInfo &info)
{
    HILOGI("called");
    const std::string collabToken = info.collabToken_;
    auto newCollab = std::make_shared<DSchedCollab>(collabToken, info);
    newCollab->Init();
    newCollab->UpdateState(SOURCE_GET_PEER_VERSION_STATE);
    collabs_.insert(std::make_pair(collabToken, newCollab));
    newCollab->PostSrcGetPeerVersionTask();
    SetTimeOut(collabToken, COLLAB_TIMEOUT);
 
#ifdef COLLAB_ALL_CONNECT_DECISIONS
    if (!DSchedTransportSoftbusAdapter::GetInstance().IsNeedAllConnect(SERVICE_TYPE_COLLAB)) {
        HILOGW("don't need wait all connect decision");
        return;
    }
    const std::string sinkDeviceId = info.sinkInfo_.deviceId_;
    if (peerConnectDecision_.find(sinkDeviceId) != peerConnectDecision_.end()) {
        peerConnectDecision_.erase(sinkDeviceId);
    }
    WaitAllConnectDecision(sinkDeviceId, newCollab);
#endif
    HILOGI("end");
}

int32_t DSchedCollabManager::CollabMission(DSchedCollabInfo &info)
{
    HILOGI("called, collabInfo is: %{public}s", info.ToString().c_str());
    if (!MultiUserManager::GetInstance().IsCallerForeground(info.srcInfo_.uid_)) {
        HILOGW("The current user is not foreground. callingUid: %{public}d .", info.srcInfo_.uid_);
        return DMS_NOT_FOREGROUND_USER;
    }
    if (!info.srcOpt_.startParams_.GetParam(KEY_START_OPTION) &&
        !Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(IPCSkeleton::GetSelfTokenID())) {
        HILOGW("Non-system applications are prohibited from launching peer ability to the background.");
        return DMS_START_CONTROL_PERMISSION_DENIED;
    }
    if (info.srcInfo_.bundleName_.empty() || info.sinkInfo_.bundleName_.empty() ||
        info.srcInfo_.moduleName_.empty() || info.sinkInfo_.moduleName_.empty() ||
        info.srcInfo_.abilityName_.empty() || info.sinkInfo_.abilityName_.empty()) {
        HILOGE("input parameter validation error!");
        return INVALID_PARAMETERS_ERR;
    }
    if (!DtbschedmgrDeviceInfoStorage::GetInstance().GetLocalDeviceId(info.srcInfo_.deviceId_)) {
        HILOGE("get local deviceId failed!");
        return FIND_LOCAL_DEVICEID_ERR;
    }
    if (DtbschedmgrDeviceInfoStorage::GetInstance().GetDeviceInfoById(info.sinkInfo_.deviceId_) == nullptr &&
        !DtbschedmgrDeviceInfoStorage::GetInstance().CheckNetworkIdByBundleName(info.sinkInfo_.bundleName_,
        info.sinkInfo_.deviceId_)) {
        HILOGE("failed to find sinkDeviceId.");
        return FIND_REMOTE_DEVICEID_ERR;
    }
    auto dCollab = GetDSchedCollabByTokenId(info.collabToken_);
    if (dCollab == nullptr) {
        HILOGE("can't find collab");
        return INVALID_PARAMETERS_ERR;
    }
    dCollab->SetSrcCollabInfo(info);
    dCollab->UpdateState(SOURCE_START_STATE);
    dCollab->PostSrcStartTask();
    return ERR_OK;
}

bool DSchedCollabManager::IsSessionExists(const DSchedCollabInfo &info)
{
    HILOGI("called");
    for (const auto& pair : collabs_) {
        if (pair.second == nullptr) {
            continue;
        }
        DSchedCollabInfo existCollabInfo = pair.second->GetCollabInfo();
        if (existCollabInfo.srcInfo_.pid_ == info.srcInfo_.pid_ &&
            existCollabInfo.srcCollabSessionId_ == info.srcCollabSessionId_) {
            return true;
        }
    }
    return false;
}

std::string DSchedCollabManager::GenerateCollabToken(const std::string &srcDeviceId)
{
    HILOGI("called");
    std::string characters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890";
    std::default_random_engine gen;
    std::random_device rd;
    gen.seed(rd());
    std::uniform_int_distribution<> dis(0, characters.size() - 1);
    std::string randomValue;
    randomValue.resize(RANDOM_STRING_LENGTH);
    size_t index = 0;
    bool isUnique = false;
    while (!isUnique) {
        for (int32_t i = 0; i < RANDOM_STRING_LENGTH; ++i) {
            index = static_cast<size_t>(dis(gen));
            randomValue[i] = characters[index];
        }
        randomValue = srcDeviceId + randomValue;
        if (collabs_.count(randomValue) == 0) {
            isUnique = true;
        }
    }
    HILOGI("end");
    return randomValue;
}

void DSchedCollabManager::SetTimeOut(const std::string &collabToken, int32_t timeout)
{
    HILOGI("called, collabToken: %{public}s", GetAnonymStr(collabToken).c_str());
    auto func = [this, collabToken]() {
        if (collabs_.count(collabToken) == 0) {
            HILOGW("collab not exist.");
            return;
        }
        HILOGE("collab timeout! info: %{public}s", GetAnonymStr(collabToken).c_str());
        auto dCollab = collabs_[collabToken];
        if (dCollab != nullptr) {
            dCollab->PostErrEndTask(COLLAB_ABILITY_TIMEOUT_ERR);
        }
    };
    if (eventHandler_ == nullptr) {
        HILOGE("eventHandler is nullptr");
        return;
    }
    timeout > 0 ? eventHandler_->PostTask(func, collabToken, timeout) :
        eventHandler_->PostTask(func);
}

void DSchedCollabManager::RemoveTimeout(const std::string &collabToken)
{
    HILOGI("called, collabToken: %{public}s", GetAnonymStr(collabToken).c_str());
    if (eventHandler_ == nullptr) {
        HILOGE("eventHandler is nullptr");
        return;
    }
    eventHandler_->RemoveTask(collabToken);
}

void DSchedCollabManager::WaitAllConnectDecision(const std::string &peerDeviceId,
    const std::shared_ptr<DSchedCollab> &dCollab)
{
#ifdef DMSFWK_ALL_CONNECT_MGR
    {
        HILOGI("called");
        std::unique_lock<std::mutex> decisionLock(connectDecisionMutex_);
        connectDecisionCond_.wait_for(decisionLock, std::chrono::seconds(CONNECT_DECISION_TIME_OUT),
            [this, peerDeviceId]() {
                return peerConnectDecision_.find(peerDeviceId) != peerConnectDecision_.end() &&
                    peerConnectDecision_.at(peerDeviceId).load();
            });

        if (peerConnectDecision_.find(peerDeviceId) == peerConnectDecision_.end()) {
            HILOGE("Not find peerDeviceId %{public}s in peerConnectDecision.", GetAnonymStr(peerDeviceId).c_str());
            if (dCollab != nullptr) {
                dCollab->PostErrEndTask(COLLAB_ABILITY_TIMEOUT_ERR);
            }
            return;
        }
        if (!peerConnectDecision_.at(peerDeviceId).load()) {
            HILOGE("All connect manager refuse bind to PeerDeviceId %{public}s.", GetAnonymStr(peerDeviceId).c_str());
            peerConnectDecision_.erase(peerDeviceId);
            if (dCollab != nullptr) {
                dCollab->PostErrEndTask(COLLAB_ABILITY_TIMEOUT_ERR);
            }
            return;
        }
        peerConnectDecision_.erase(peerDeviceId);
        HILOGI("end");
    }
#endif
}

void DSchedCollabManager::NotifyAllConnectDecision(std::string peerDeviceId, bool isSupport)
{
    HILOGI("Notify all connect decision, peerDeviceId %{public}s, isSupport %{public}d.",
        GetAnonymStr(peerDeviceId).c_str(), isSupport);
#ifdef DMSFWK_ALL_CONNECT_MGR
    std::lock_guard<std::mutex> decisionLock(connectDecisionMutex_);
    peerConnectDecision_[peerDeviceId] = isSupport;
    connectDecisionCond_.notify_all();
#endif
}

int32_t DSchedCollabManager::NotifyStartAbilityResult(const std::string& collabToken, const int32_t &result,
    const int32_t &sinkPid, const int32_t &sinkUid, const int32_t &sinkAccessTokenId)
{
    HILOGI("called, result %{public}d", result);
    auto dCollab = GetDSchedCollabByTokenId(collabToken);
    if (dCollab == nullptr) {
        HILOGE("can't find collab");
        return INVALID_PARAMETERS_ERR;
    }
    if (result != ACCEPT) {
        HILOGE("start ability failed");
        return dCollab->PostErrEndTask(ConvertCollaborateResult(result));
    }
    dCollab->SaveSinkAbilityData(collabToken, result, sinkPid, sinkUid, sinkAccessTokenId);
    HILOGI("end, info: %{public}s.", dCollab->GetCollabInfo().ToString().c_str());
    return ERR_OK;
}

int32_t DSchedCollabManager::ConvertCollaborateResult(int32_t result)
{
    HILOGI("collaborate result is %{public}d.", result);
    switch (result) {
        case REJECT:
            return DistributedCollab::PEER_APP_REJECTED;
        case ON_COLLABORATE_ERR:
            return DistributedCollab::PEER_ABILITY_NO_ONCOLLABORATE;
        default:
            return INVALID_PARAMETERS_ERR;
    }
}

int32_t DSchedCollabManager::NotifySinkPrepareResult(const DSchedCollabInfo &dSchedCollabInfo, const int32_t &result)
{
    HILOGI("end, dSchedCollabInfo: %{public}s.", dSchedCollabInfo.ToString().c_str());
    if (GetDSchedCollabByTokenId(dSchedCollabInfo.collabToken_) == nullptr) {
        HILOGE("not find dCollab");
        return INVALID_PARAMETERS_ERR;
    }
    auto func = [this, dSchedCollabInfo, result]() {
        HandleCollabPrepareResult(dSchedCollabInfo, result);
    };
    if (eventHandler_ == nullptr) {
        HILOGE("eventHandler is nullptr");
        return INVALID_PARAMETERS_ERR;
    }
    eventHandler_->PostTask(func);
    return ERR_OK;
}

void DSchedCollabManager::HandleCollabPrepareResult(const DSchedCollabInfo &dSchedCollabInfo, const int32_t &result)
{
    HILOGI("called");
    auto dCollab = GetDSchedCollabByTokenId(dSchedCollabInfo.collabToken_);
    if (dCollab == nullptr) {
        HILOGE("not find dCollab");
        return;
    }
    dCollab->PostSinkPrepareResultTask(result, dSchedCollabInfo);
    RemoveTimeout(dSchedCollabInfo.collabToken_);
    HILOGI("end, info: %{public}s.", dCollab->GetCollabInfo().ToString().c_str());
    return;
}

int32_t DSchedCollabManager::NotifySinkRejectReason(const std::string& collabToken, const std::string& reason)
{
    HILOGI("called, resion: %{public}s", reason.c_str());
    auto dCollab = GetDSchedCollabByTokenId(collabToken);
    if (dCollab == nullptr) {
        HILOGE("not find dCollab");
        return INVALID_PARAMETERS_ERR;
    }
    return dCollab->PostAbilityRejectTask(reason);
}

int32_t DSchedCollabManager::NotifyAbilityDied(const std::string &bundleName, const int32_t &pid)
{
    HILOGI("called");
    auto func = [this, bundleName, pid]() {
        HandleCloseSessions(bundleName, pid);
    };
    if (eventHandler_ == nullptr) {
        HILOGE("eventHandler is nullptr");
        return INVALID_PARAMETERS_ERR;
    }
    eventHandler_->PostTask(func);
    return ERR_OK;
}

int32_t DSchedCollabManager::HandleCloseSessions(const std::string &bundleName, const int32_t &pid)
{
    HILOGI("called");
    DSchedCollabInfo dSchedCollabInfo;
    for (auto iter = collabs_.begin(); iter != collabs_.end(); iter++) {
        if (iter->second == nullptr) {
            continue;
        }
        dSchedCollabInfo = iter->second->GetCollabInfo();
        if ((bundleName == dSchedCollabInfo.srcInfo_.bundleName_ && pid == dSchedCollabInfo.srcInfo_.pid_) ||
            (bundleName == dSchedCollabInfo.srcInfo_.bundleName_ && pid == dSchedCollabInfo.sinkInfo_.pid_)) {
            HILOGI("find collab, need be clear, collabInfo: %{public}s", dSchedCollabInfo.ToString().c_str());
            return iter->second->PostEndTask();
        }
    }
    return ERR_OK;
}

int32_t DSchedCollabManager::ReleaseAbilityLink(const std::string &bundleName, const int32_t &pid)
{
    HILOGW("called");
    DSchedCollabInfo dSchedCollabInfo;
    std::string collabToken;
    {
        std::shared_lock<std::shared_mutex> readLock(collabReadMutex_);
        for (auto iter = collabs_.begin(); iter != collabs_.end(); iter++) {
            if (iter->second == nullptr) {
                continue;
            }
            dSchedCollabInfo = iter->second->GetCollabInfo();
            if (bundleName == dSchedCollabInfo.srcInfo_.bundleName_ && pid == dSchedCollabInfo.srcInfo_.pid_) {
                collabToken = dSchedCollabInfo.collabToken_;
            }
            if (bundleName == dSchedCollabInfo.srcInfo_.bundleName_ && pid == dSchedCollabInfo.sinkInfo_.pid_) {
                collabToken = dSchedCollabInfo.collabToken_;
            }
        }
    }
    if (collabToken.empty()) {
        HILOGE("can't find collab");
        return INVALID_PARAMETERS_ERR;
    }

    auto func = [this, bundleName, pid, collabToken]() {
        HandleReleaseAbilityLink(bundleName, pid, collabToken);
    };
    const std::string taskName = bundleName + std::to_string(pid);
    HILOGW("disconnect the existing link after 5S");
    if (eventHandler_ == nullptr) {
        HILOGE("eventHandler is nullptr");
        return INVALID_PARAMETERS_ERR;
    }
    eventHandler_->PostTask(func, taskName, BACKGROUND_TIMEOUT);
    return ERR_OK;
}

void DSchedCollabManager::HandleReleaseAbilityLink(const std::string &bundleName, const int32_t &pid,
    const std::string &collabToken)
{
    HILOGI("called");
    auto dCollab = GetDSchedCollabByTokenId(collabToken);
    if (dCollab == nullptr) {
        HILOGE("can't find collab");
        return;
    }
    DSchedCollabInfo dSchedCollabInfo = dCollab->GetCollabInfo();
    if (pid == dSchedCollabInfo.srcInfo_.pid_) {
        HILOGI("source ability been background, collabInfo: %{public}s", dSchedCollabInfo.ToString().c_str());
        PrivilegeShutdown(static_cast<uint64_t>(dSchedCollabInfo.srcInfo_.accessToken_),
            pid, dSchedCollabInfo.sinkInfo_.deviceId_.c_str());
            dCollab->PostEndTask();
    }
    if (pid == dSchedCollabInfo.sinkInfo_.pid_) {
        HILOGI("sink ability been background, collabInfo: %{public}s", dSchedCollabInfo.ToString().c_str());
        dCollab->PostEndTask();
    }
    return;
}

int32_t DSchedCollabManager::CancleReleaseAbilityLink(const std::string &bundleName, const int32_t &pid)
{
    HILOGW("called");
    if (eventHandler_ == nullptr) {
        HILOGE("eventHandler is nullptr");
        return INVALID_PARAMETERS_ERR;
    }
    const std::string taskName = bundleName + std::to_string(pid);
    eventHandler_->RemoveTask(taskName);
    return ERR_OK;
}

void DSchedCollabManager::NotifyWifiOpen()
{
    HILOGI("called");
    for (auto iter = collabs_.begin(); iter != collabs_.end(); iter++) {
        if (iter->second != nullptr) {
            iter->second->NotifyWifiOpen();
        }
    }
}

bool DSchedCollabManager::GetWifiStatus()
{
    return DistributedSchedule::WifiStateAdapter::GetInstance().IsWifiActive();
}

int32_t DSchedCollabManager::NotifySessionClose(const std::string &collabToken)
{
    HILOGI("called, collabToken: %{public}s", GetAnonymStr(collabToken).c_str());
    auto dCollab = GetDSchedCollabByTokenId(collabToken);
    if (dCollab == nullptr) {
        HILOGE("can't find collab");
        return INVALID_PARAMETERS_ERR;
    }
    dCollab->PostEndTask();
    HILOGI("end.");
    return ERR_OK;
}

int32_t DSchedCollabManager::CleanUpSession(const std::string &collabToken)
{
    HILOGI("called, collabToken: %{public}s", GetAnonymStr(collabToken).c_str());
    auto func = [this, collabToken]() {
        HILOGI("called, collabToken: %{public}s", GetAnonymStr(collabToken).c_str());
        auto dCollab = GetDSchedCollabByTokenId(collabToken);
        if (dCollab == nullptr) {
            HILOGE("can't find collab");
            return;
        }
        dCollab->UnInit();
        RemoveTimeout(collabToken);
        {
            std::lock_guard<std::mutex> collabLock(collabMutex_);
            collabs_.erase(collabToken);
        }
    };
    if (eventHandler_ == nullptr) {
        HILOGE("eventHandler is nullptr");
        return INVALID_PARAMETERS_ERR;
    }
    eventHandler_->PostTask(func);
    HILOGI("end.");
    return ERR_OK;
}

std::shared_ptr<DSchedCollab> DSchedCollabManager::GetDSchedCollabByTokenId(const std::string &tokenId)
{
    HILOGI("called, tokenId: %{public}s", tokenId.c_str());
    std::lock_guard<std::mutex> collabLock(collabMutex_);
    if (tokenId.empty() || collabs_.count(tokenId) == 0) {
        HILOGE("no such collaboration in progress.");
        return nullptr;
    }
    return collabs_[tokenId];
}

void DSchedCollabManager::OnDataRecv(int32_t softbusSessionId, std::shared_ptr<DSchedDataBuffer> dataBuffer)
{
    auto func = [this, softbusSessionId, dataBuffer]() {
        HandleDataRecv(softbusSessionId, dataBuffer);
    };
    if (eventHandler_ == nullptr) {
        HILOGE("eventHandler is nullptr");
        return;
    }
    eventHandler_->PostTask(func);
}

void DSchedCollabManager::HandleDataRecv(const int32_t &softbusSessionId, std::shared_ptr<DSchedDataBuffer> dataBuffer)
{
    HILOGI("called, softbusSessionId: %{public}d.", softbusSessionId);
    if (dataBuffer == nullptr) {
        HILOGE("dataBuffer is null.");
        return;
    }
    uint8_t *data = dataBuffer->Data();
    std::string jsonStr(reinterpret_cast<const char *>(data), dataBuffer->Capacity());
    cJSON *rootValue = cJSON_Parse(jsonStr.c_str());
    if (rootValue == nullptr) {
        HILOGE("Parse jsonStr error.");
        return;
    }
    cJSON *baseCmd = cJSON_GetObjectItemCaseSensitive(rootValue, "BaseCmd");
    if (baseCmd == nullptr || !cJSON_IsString(baseCmd) || (baseCmd->valuestring == nullptr)) {
        cJSON_Delete(rootValue);
        HILOGE("Parse base cmd error.");
        return;
    }

    cJSON *cmdValue = cJSON_Parse(baseCmd->valuestring);
    cJSON_Delete(rootValue);
    if (cmdValue == nullptr) {
        HILOGE("Parse cmd value error.");
        return;
    }

    cJSON *comvalue = cJSON_GetObjectItemCaseSensitive(cmdValue, "Command");
    if (comvalue == nullptr || !cJSON_IsNumber(comvalue)) {
        cJSON_Delete(cmdValue);
        HILOGE("parse command failed");
        return;
    }
    int32_t command = comvalue->valueint;
    cJSON *collabTokenvalue = cJSON_GetObjectItemCaseSensitive(cmdValue, "CollabToken");
    if (collabTokenvalue == nullptr || !cJSON_IsString(collabTokenvalue)) {
        cJSON_Delete(cmdValue);
        HILOGE("parse collabToken failed");
        return;
    }
    const std::string collabToken = collabTokenvalue->valuestring;
    cJSON_Delete(cmdValue);
    NotifyDataRecv(softbusSessionId, command, jsonStr, dataBuffer, collabToken);
    HILOGI("end");
}

void DSchedCollabManager::NotifyDataRecv(const int32_t &softbusSessionId, int32_t command, const std::string& jsonStr,
    std::shared_ptr<DSchedDataBuffer> dataBuffer, const std::string& collabToken)
{
    HILOGI("called, parsed cmd %{public}s", CMDDATA[command].c_str());
    for (auto iter = collabs_.begin(); iter != collabs_.end(); iter++) {
        if (iter->second != nullptr && softbusSessionId == iter->second->GetSoftbusSessionId() &&
            collabToken == iter->second->GetCollabInfo().collabToken_) {
            HILOGI("softbusSessionId exist.");
            const std::string peerDeviceId =
                DSchedTransportSoftbusAdapter::GetInstance().GetPeerDeviceIdBySocket(softbusSessionId);
            iter->second->OnDataRecv(peerDeviceId, command, dataBuffer);
            if (command == NOTIFY_RESULT_CMD) {
                RemoveTimeout(iter->first);
            }
            return;
        }
    }
    if (command == SINK_GET_VERSION_CMD) {
        auto getVersionCmd = std::make_shared<GetSinkCollabVersionCmd>();
        int32_t ret = getVersionCmd->Unmarshal(jsonStr);
        if (ret != ERR_OK) {
            HILOGE("Unmarshal cmd failed, ret: %{public}d", ret);
            return;
        }
        std::string localDevId;
        if (!DtbschedmgrDeviceInfoStorage::GetInstance().GetLocalDeviceId(localDevId)) {
            HILOGE("get local deviceId failed!");
            return;
        }
        if (getVersionCmd->sinkDeviceId_ != localDevId) {
            HILOGE("Irrecognized deviceId: %{public}s", GetAnonymStr(getVersionCmd->sinkDeviceId_).c_str());
            return;
        }
        auto newCollab = std::make_shared<DSchedCollab>(getVersionCmd, softbusSessionId);
        newCollab->Init();
        newCollab->UpdateState(SINK_GET_VERSION_STATE);
        collabs_.insert(std::make_pair(getVersionCmd->collabToken_, newCollab));
        newCollab->PostSinkGetVersionTask();
        SetTimeOut(getVersionCmd->collabToken_, COLLAB_TIMEOUT);
        HILOGI("end");
        return;
    }
}

void DSchedCollabManager::OnShutdown(int32_t socket, bool isSelfCalled)
{
    HILOGW("called,softbusSessionId: %{public}d, isSelfCalled: %{public}d", socket, isSelfCalled);
    if (isSelfCalled) {
        HILOGW("called, shutdown by local");
        return;
    }
    auto func = [this, socket]() {
        std::lock_guard<std::mutex> collabLock(collabMutex_);
        if (collabs_.empty()) {
            return;
        }
        for (auto iter = collabs_.begin(); iter != collabs_.end(); iter++) {
            if (iter->second != nullptr && socket == iter->second->GetSoftbusSessionId()) {
                HILOGE("collab: %{public}s", iter->second->GetCollabInfo().ToString().c_str());
                iter->second->PostErrEndTask(COLLAB_SESSION_SHUTDOWN);
            }
        }
    };
    if (eventHandler_ == nullptr) {
        HILOGE("eventHandler is nullptr");
        return;
    }
    eventHandler_->PostTask(func);
    return;
}

void DSchedCollabManager::SoftbusListener::OnBind(int32_t socket, PeerSocketInfo info)
{
}

void DSchedCollabManager::SoftbusListener::OnShutdown(int32_t socket, bool isSelfCalled)
{
    DSchedCollabManager::GetInstance().OnShutdown(socket, isSelfCalled);
}

void DSchedCollabManager::SoftbusListener::OnDataRecv(int32_t socket, std::shared_ptr<DSchedDataBuffer> dataBuffer)
{
    DSchedCollabManager::GetInstance().OnDataRecv(socket, dataBuffer);
}
}  // namespace DistributedSchedule
}  // namespace OHOS
