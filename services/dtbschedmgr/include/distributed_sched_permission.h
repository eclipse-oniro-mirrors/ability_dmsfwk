/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef DISTRIBUTED_SCHED_PERMISSION_H
#define DISTRIBUTED_SCHED_PERMISSION_H

#include <stdint.h>
#include <string>

#include "distributed_sched_interface.h"
#include "nlohmann/json.hpp"
#include "single_instance.h"

#include "app_mgr_interface.h"
#include "app_mgr_proxy.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace DistributedSchedule {
namespace {
const std::string DMS_IS_CALLER_FOREGROUND = "dmsIsCallerForeGround";
}

struct GroupInfo {
    std::string groupName;
    std::string groupId;
    std::string groupOwner;
    int32_t groupType;
    int32_t groupVisibility;

    GroupInfo() : groupName(""), groupId(""), groupOwner(""), groupType(0), groupVisibility(0) {}
};

void from_json(const nlohmann::json& jsonObject, GroupInfo& groupInfo);

class DistributedSchedPermission {
    DECLARE_SINGLE_INSTANCE(DistributedSchedPermission);

public:
    using AccountInfo = IDistributedSched::AccountInfo;

    int32_t CheckSendResultPermission(const AAFwk::Want& want, const CallerInfo& callerInfo,
        const AccountInfo& accountInfo, AppExecFwk::AbilityInfo& targetAbility);
    int32_t CheckStartPermission(const AAFwk::Want& want, const CallerInfo& callerInfo,
        const AccountInfo& accountInfo, AppExecFwk::AbilityInfo& targetAbility, bool isSameBundle = true);
    int32_t CheckCollabStartPermission(const AAFwk::Want& want, const CallerInfo& callerInfo,
        const AccountInfo& accountInfo, AppExecFwk::AbilityInfo& targetAbility);
    int32_t CheckGetCallerPermission(const AAFwk::Want& want, const CallerInfo& callerInfo,
        const AccountInfo& accountInfo, AppExecFwk::AbilityInfo& targetAbility);
    bool IsFoundationCall() const;
    bool IsSceneBoardCall() const;
    int32_t CheckPermission(uint32_t accessToken, const std::string& permissionName) const;
    int32_t CheckPermissionAll(uint32_t accessToken, const std::string& permissionName) const;
    int32_t GetAccountInfo(const std::string& remoteNetworkId, const CallerInfo& callerInfo,
        AccountInfo& accountInfo);
    bool GetTargetAbility(const AAFwk::Want& want, AppExecFwk::AbilityInfo& targetAbility,
        bool needQueryExtension = false) const;
    void MarkUriPermission(OHOS::AAFwk::Want& want, uint32_t accessToken);
    void RemoveRemoteObjectFromWant(std::shared_ptr<AAFwk::Want> want) const;
    bool IsHigherAclVersion(const CallerInfo& callerInfo);
    bool CheckSrcBackgroundPermission(uint32_t accessTokenId);
    bool IsAbilityForeground(uint32_t accessTokenId);

private:
    bool GetOsAccountData(AccountInfo& dmsAccountInfo);
    bool CheckSameAccount(const std::string& dstNetworkId, const AccountInfo& dmsAccountInfo,
        const CallerInfo& callerInfo, bool isSrc);
    bool CheckDstSameAccount(const std::string& dstNetworkId, const AccountInfo& dmsAccountInfo,
        const CallerInfo& callerInfo, bool isSrc);
    bool CheckLowVersionSameAccount(const std::string& dstNetworkId, const AccountInfo& dmsAccountInfo,
        const CallerInfo& callerInfo, bool isSrc);
    bool CheckAclList(const std::string& dstNetworkId, const AccountInfo& dmsAccountInfo,
        const CallerInfo& callerInfo, bool isSrc, const std::string& targetBundleName = "");
    bool CheckNewAclList(const std::string& dstNetworkId, const AccountInfo& dmsAccountInfo,
        const CallerInfo& callerInfo, bool isSrc, const std::string& targetBundleName = "");
    bool CheckLowVersionAclList(const std::string& dstNetworkId, const AccountInfo& dmsAccountInfo,
        const CallerInfo& callerInfo, bool isSrc, const std::string& targetBundleName = "");
    bool GetRelatedGroups(const std::string& udid, const std::vector<std::string>& bundleNames,
        AccountInfo& accountInfo);
    bool ParseGroupInfos(const std::string& returnGroupStr, std::vector<GroupInfo>& groupInfos);
    bool VerifyPermission(uint32_t accessToken, const std::string& permissionName) const;
    bool CheckAccountAccessPermission(const CallerInfo& callerInfo,
        const AccountInfo& accountInfo, const std::string& targetBundleName, bool isNewCollab = false);
    bool CheckComponentAccessPermission(const AppExecFwk::AbilityInfo& targetAbility,
        const CallerInfo& callerInfo, const AccountInfo& accountInfo, const AAFwk::Want& want) const;
    bool CheckMigrateStartCtrlPer(const AppExecFwk::AbilityInfo& targetAbility,
        const CallerInfo& callerInfo, const AAFwk::Want& want, bool isSameBundle = true);
    bool CheckCollaborateStartCtrlPer(const AppExecFwk::AbilityInfo& targetAbility,
        const CallerInfo& callerInfo, const AAFwk::Want& want) const;
    bool CheckNewCollabStartControlPermission(const AppExecFwk::AbilityInfo& targetAbility,
        const CallerInfo& callerInfo, const AAFwk::Want& want);
    bool CheckStartControlPermission(const AppExecFwk::AbilityInfo& targetAbility,
        const CallerInfo& callerInfo, const AAFwk::Want& want, bool isSameBundle = true);
    bool CheckNewCollabBackgroundPermission(const CallerInfo& callerInfo, const AAFwk::Want& want);
    bool CheckBackgroundPermission(const AppExecFwk::AbilityInfo& targetAbility,
        const CallerInfo& callerInfo, const AAFwk::Want& want, bool needCheckApiVersion) const;
    bool CheckMinApiVersion(const AppExecFwk::AbilityInfo& targetAbility, int32_t apiVersion) const;
    bool CheckDeviceSecurityLevel(const std::string& srcDeviceId, const std::string& dstDeviceId) const;
    int32_t GetDeviceSecurityLevel(const std::string& udid) const;
    bool CheckTargetAbilityVisible(const AppExecFwk::AbilityInfo& targetAbility, const CallerInfo& callerInfo) const;
    bool IsDistributedFile(const std::string& path) const;
    sptr<AppExecFwk::IAppMgr> GetAppManager() const;
};
} // namespace DistributedSchedule
} // namespace OHOS
#endif // DISTRIBUTED_SCHED_PERMISSION_H