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

#include "datashare_manager.h"

#include "distributed_sched_utils.h"
#include "dsched_continue_manager.h"
#include "dtbschedmgr_log.h"
#include "mission/dms_continue_condition_manager.h"
#include "mission/notification/dms_continue_send_manager.h"
#include "os_account_manager.h"
#include "switch_status_dependency.h"

namespace OHOS {
namespace DistributedSchedule {
IMPLEMENT_SINGLE_INSTANCE(DataShareManager);
namespace {
const std::string TAG = "DMSDataShareManager";
constexpr static int32_t INVALID_ACCOUNT_ID = -1;
}
SettingObserver::SettingObserver() = default;
SettingObserver::~SettingObserver() = default;

void SettingObserver::OnChange()
{
    HILOGI("DataShareRegisterObserver OnChange start");
    if (observerCallback_ != nullptr) {
        observerCallback_();
    }
    HILOGI("DataShareRegisterObserver OnChange done");
}

void SettingObserver::SetObserverCallback(ObserverCallback &observerCallback)
{
    observerCallback_ = observerCallback;
}

void DataShareManager::RegisterObserver(SettingObserver::ObserverCallback &observerCallback)
{
    std::lock_guard<std::mutex> lock(observerMutex_);
    if (observer_ != nullptr) {
        HILOGI("observer_ has been created.");
        return;
    }
    observer_ = new SettingObserver();
    if (observer_ == nullptr) {
        HILOGE("Register observer failed, observer is null");
        return;
    }
    observer_->SetObserverCallback(observerCallback);
    HILOGI("observer_ create success.");
}

std::shared_ptr<DataShare::DataShareHelper> DataShareManager::CreateDataShareHelper()
{
    HILOGI("DataShareManager CreateDataShareHelper start");
    DataShare::CreateOptions options;
    options.isProxy_ = true;
    return DataShare::DataShareHelper::Creator(SwitchStatusDependency::SETTINGS_USER_SECURE_URI, options);
}

void DataShareManager::RegisterObserver(const std::string &key, SettingObserver::ObserverCallback &observerCallback)
{
    HILOGI("DataShareManager RegisterObserver start");
    RegisterObserver(observerCallback);
    if (observer_ != nullptr) {
        HILOGI("Observer is already registered with key is %{public}s", key.c_str());
        UnregisterObserver(key);
    }
    std::shared_ptr<DataShare::DataShareHelper> dataShareHelper = CreateDataShareHelper();
    if (dataShareHelper == nullptr) {
        HILOGE("Register observer failed, dataShareHelper is null");
        return;
    }
    int32_t userId = GetLocalAccountId();
    Uri uri(AssembleUserSecureUri(userId, key));
    dataShareHelper->RegisterObserver(uri, observer_);
    dataShareHelper->Release();
    HILOGI("DataShareManager RegisterObserver success with key is %{public}s", key.c_str());
}

void DataShareManager::UnregisterObserver(const std::string &key)
{
    HILOGI("DataShareManager UnregisterObserver start");
    if (observer_ == nullptr) {
        HILOGI("UnregisterObserver, observer is nullptr with key is %{public}s", key.c_str());
        return;
    }
    std::shared_ptr<DataShare::DataShareHelper> dataShareHelper = CreateDataShareHelper();
    if (dataShareHelper == nullptr) {
        HILOGE("Unregister observer failed with key is %{public}s", key.c_str());
        return;
    }
    int32_t userId = GetLocalAccountId();
    Uri uri(AssembleUserSecureUri(userId, key));
    dataShareHelper->UnregisterObserver(uri, observer_);
    dataShareHelper->Release();
    HILOGI("DataShareManager UnregisterObserver success with key is %{public}s", key.c_str());
}

void DataShareManager::UnInit()
{
    HILOGI("DataShareManager UnInit start");
    UnregisterObserver(SwitchStatusDependency::GetInstance().CONTINUE_SWITCH_STATUS_KEY);
    observer_ = nullptr;
    HILOGI("DataShareManager UnInit end");
}

Uri DataShareManager::AssembleUserSecureUri(int userId, const std::string &key)
{
    Uri uri(SwitchStatusDependency::SETTINGS_USER_SECURE_URI + "_" + std::to_string(userId) + "?Proxy=true&key=" + key);
    return uri;
}

int32_t DataShareManager::GetLocalAccountId()
{
    int32_t id = INVALID_ACCOUNT_ID;
    ErrCode err = AccountSA::OsAccountManager::GetForegroundOsAccountLocalId(id);
    if (err != ERR_OK || id == INVALID_ACCOUNT_ID) {
        HILOGE("GetLocalAccountId passing param invalid or return error!, err : %{public}d", err);
        return INVALID_PARAMETERS_ERR;
    }
    return id;
}

bool DataShareManager::IsCurrentContinueSwitchOn()
{
    HILOGD("IsCurrentContinueSwitchOn start");
    return isCurrentContinueSwitchOn_.load();
}

void DataShareManager::SetCurrentContinueSwitch(bool status)
{
    HILOGD("SetCurrentContinueSwitch start, status : %{public}d", status);
    isCurrentContinueSwitchOn_.store(status);
    DmsContinueConditionMgr::GetInstance().UpdateSystemStatus(SYS_EVENT_CONTINUE_SWITCH, status);
}
} // namespace DistributedSchedule
} // namespace OHOS
