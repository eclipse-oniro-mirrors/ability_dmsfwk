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

#ifndef OHOS_ABILITY_CONNECTION_MANAGER_DISTRIBUTED_CLIENT_H
#define OHOS_ABILITY_CONNECTION_MANAGER_DISTRIBUTED_CLIENT_H


#include <string>

#include "ability_connection_info.h"
#include "ability_connection_session.h"
#include "iremote_object.h"
#include "system_ability_status_change_stub.h"
#include <mutex>
#include <condition_variable>

namespace OHOS {
namespace DistributedCollab {
class DistributedClient {
public:
    DistributedClient() = default;
    ~DistributedClient();
    
    int32_t CollabMission(int32_t sessionId, const std::string& serverSocketName,
        const AbilityConnectionSessionInfo& sessionInfo, const ConnectOption& options, const std::string& token);
    int32_t NotifyPrepareResult(const std::string& token, int32_t result, int32_t sessionId,
        const std::string& serverSocketName);
    int32_t NotifyCloseCollabSession(const std::string& token);
    int32_t NotifyRejectReason(const std::string& token, const std::string& reason);
    int32_t GetPeerVersion(int32_t sessionId, const std::string& peerDeviceId, const std::string dmsServerToken);
    bool GetWifiStatus();
    
    enum {
        COLLAB_MESSION = 330,
        NOTIFY_PREPARE_RESULT = 331,
        NOTIFY_REJECT_REASON = 332,
        BNOTIFY_CLOSE_COLLAB_SESSION = 333,
        GET_SINK_COLLAB_VERSION = 335,
        GET_WIFI_STATUS = 336,
    };
private:
    sptr<IRemoteObject> GetDmsProxy();

private:
    std::mutex mtx_;
    std::condition_variable cv_;
    bool callbackReceived_ = false;
    sptr<ISystemAbilityStatusChange> listener_ = nullptr;
    class SystemAbilityStatusChangeListener : public SystemAbilityStatusChangeStub {
    public:
        explicit SystemAbilityStatusChangeListener(DistributedClient* client): client_(client) {}
        void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
        void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
    private:
        DistributedClient* client_ = nullptr;
    };
};
}  // namespace AAFwk
}  // namespace OHOS
#endif