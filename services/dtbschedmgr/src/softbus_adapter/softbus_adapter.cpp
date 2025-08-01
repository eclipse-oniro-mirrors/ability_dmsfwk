/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "softbus_adapter/softbus_adapter.h"

#include <sys/prctl.h>

#include "dfx/distributed_radar.h"
#include "dtbschedmgr_log.h"
#include "softbus_error_code.h"
#include "mission/distributed_mission_broadcast_listener.h"

namespace OHOS {
namespace DistributedSchedule {
namespace {
const std::string TAG = "SoftbusAdapter";
const std::string SOFTBUS_ADAPTER = "softbus_adapter";
const std::string RETRY_SENT_EVENT_TASK = "retry_on_sent_event_task";
constexpr int32_t RETRY_SENT_EVENT_DELAY = 50;
constexpr int32_t RETRY_SENT_EVENT_MAX_TIME = 3;
}

IMPLEMENT_SINGLE_INSTANCE(SoftbusAdapter);

static void OnBroadCastRecvAdapt(std::string &networkId, uint8_t *data, uint32_t dataLen)
{
    SoftbusAdapter::GetInstance().OnBroadCastRecv(networkId, data, dataLen);
}

void SoftbusAdapter::Init()
{
    if (eventHandler_ != nullptr) {
        HILOGI("Already inited, end.");
        return;
    }
    eventThread_ = std::thread(&SoftbusAdapter::StartEvent, this);
    std::unique_lock<std::mutex> lock(eventMutex_);
    eventCon_.wait(lock, [this] {
        return eventHandler_ != nullptr;
    });
}

void SoftbusAdapter::StartEvent()
{
    HILOGI("StartEvent start");
    prctl(PR_SET_NAME, SOFTBUS_ADAPTER.c_str());
    auto runner = AppExecFwk::EventRunner::Create(false);
    {
        std::lock_guard<std::mutex> lock(eventMutex_);
        eventHandler_ = std::make_shared<OHOS::AppExecFwk::EventHandler>(runner);
    }
    eventCon_.notify_one();
    if (runner != nullptr) {
        runner->Run();
    } else {
        HILOGE("runner is null");
    }
    HILOGI("StartEvent end");
}

void SoftbusAdapter::UnInit()
{
    HILOGI("UnInit start");
    if (eventHandler_ != nullptr && eventHandler_->GetEventRunner() != nullptr) {
        eventHandler_->GetEventRunner()->Stop();
        if (eventThread_.joinable()) {
            eventThread_.join();
        }
        eventHandler_ = nullptr;
    } else {
        HILOGE("eventHandler_ or eventRunner is nullptr");
    }
    HILOGI("UnInit end");
}

int32_t SoftbusAdapter::SendSoftbusEvent(std::shared_ptr<DSchedDataBuffer> buffer)
{
    HILOGI("SendSoftbusEvent pkgName: %{public}s.", pkgName_.c_str());
    auto feedfunc = [this, buffer]() {
        DealSendSoftbusEvent(buffer);
    };
    if (eventHandler_ != nullptr) {
        eventHandler_->PostTask(feedfunc);
    } else {
        HILOGE("eventHandler_ is nullptr");
    }

    return SOFTBUS_OK;
}

int32_t SoftbusAdapter::DealSendSoftbusEvent(std::shared_ptr<DSchedDataBuffer> buffer, const int32_t retry)
{
    if (eventHandler_ != nullptr) {
        eventHandler_->RemoveTask(RETRY_SENT_EVENT_TASK);
    } else {
        HILOGE("eventHandler_ is nullptr");
        return INVALID_PARAMETERS_ERR;
    }

    if (buffer == nullptr) {
        HILOGE("buffer is nullptr");
        return INVALID_PARAMETERS_ERR;
    }
    if (dmsAdapetr_.SendSoftbusEvent == nullptr) {
        HILOGE("Dms interactive SendSoftbusEvent is null.");
        return INVALID_PARAMETERS_ERR;
    }
    int32_t ret = dmsAdapetr_.SendSoftbusEvent(true, buffer->Data(), buffer->Capacity());
    if (ret != SOFTBUS_OK) {
        HILOGW("SendEvent failed, ret:%{public}d.", ret);
        return RetrySendSoftbusEvent(buffer, retry);
    }
    return ret;
}

int32_t SoftbusAdapter::RetrySendSoftbusEvent(std::shared_ptr<DSchedDataBuffer> buffer, const int32_t retry)
{
    HILOGI("Retry post broadcast, current retry times %{public}d", retry);
    if (retry == RETRY_SENT_EVENT_MAX_TIME) {
        HILOGE("meet max retry time!");
        return INVALID_PARAMETERS_ERR;
    }
    auto feedfunc = [this, buffer, retry]() mutable {
        DealSendSoftbusEvent(buffer, retry + 1);
    };
    if (eventHandler_ != nullptr) {
        eventHandler_->RemoveTask(RETRY_SENT_EVENT_TASK);
        eventHandler_->PostTask(feedfunc, RETRY_SENT_EVENT_TASK, RETRY_SENT_EVENT_DELAY);
    } else {
        HILOGE("eventHandler_ is nullptr");
        return INVALID_PARAMETERS_ERR;
    }
    return ERR_OK;
}

int32_t SoftbusAdapter::StopSoftbusEvent()
{
    HILOGI("StopSoftbusEvent pkgName: %{public}s.", pkgName_.c_str());
    if (dmsAdapetr_.StopSoftbusEvent == nullptr) {
        HILOGE("Dms interactive StopSoftbusEvent is null.");
        return INVALID_PARAMETERS_ERR;
    }
    int32_t ret = dmsAdapetr_.StopSoftbusEvent();
    if (ret != SOFTBUS_OK) {
        HILOGE("StopEvent failed, ret:%{public}d.", ret);
        return ret;
    }
    return SOFTBUS_OK;
}

void SoftbusAdapter::OnBroadCastRecv(std::string& networkId, uint8_t* data, uint32_t dataLen)
{
    if (softbusAdapterListener_ != nullptr) {
        softbusAdapterListener_->OnDataRecv(networkId, data, dataLen);
    } else {
        HILOGE("softbusAdapterListener_ is nullptr");
    }
}

int32_t SoftbusAdapter::RegisterSoftbusEventListener(const std::shared_ptr<SoftbusAdapterListener>& listener)
{
    if (listener == nullptr) {
        HILOGE("Registering listener failed");
        return SOFTBUS_INVALID_PARAM;
    }
    {
        std::lock_guard<std::mutex> lock(softbusAdapterListenerMutex_);
        softbusAdapterListener_ = listener;
    }

    HILOGI("RegisterSoftbusEventListener pkgName: %s.", pkgName_.c_str());
    if (dmsAdapetr_.RegisterSoftbusEventListener == nullptr) {
        HILOGE("Dms interactive RegisterSoftbusEventListener is null.");
        return INVALID_PARAMETERS_ERR;
    }
    int32_t ret = dmsAdapetr_.RegisterSoftbusEventListener(true, (void *)OnBroadCastRecvAdapt);
    DmsRadar::GetInstance().RegisterSoftbusCallbackRes("RegisterSoftbusEventListener", ret);
    if (ret != SOFTBUS_OK) {
        HILOGE("RegisterSoftbusEventListener failed, ret: %{public}d.", ret);
        return ret;
    }
    return SOFTBUS_OK;
}

int32_t SoftbusAdapter::UnregisterSoftbusEventListener(const std::shared_ptr<SoftbusAdapterListener>& listener)
{
    if (listener == nullptr) {
        HILOGE("Unregistering listener failed");
        return SOFTBUS_INVALID_PARAM;
    }
    {
        std::lock_guard<std::mutex> lock(softbusAdapterListenerMutex_);
        softbusAdapterListener_ = listener;
    }

    HILOGI("UnregisterSoftbusEventListener pkgName: %s.", pkgName_.c_str());
    if (dmsAdapetr_.UnregisterSoftbusEventListener == nullptr) {
        HILOGE("Dms interactive UnregisterSoftbusEventListener is null.");
        return INVALID_PARAMETERS_ERR;
    }
    int32_t ret = dmsAdapetr_.UnregisterSoftbusEventListener(true, (void *)OnBroadCastRecvAdapt);
    if (ret != SOFTBUS_OK) {
        HILOGE("UnregisterSoftbusEventListener failed, ret: %{public}d.", ret);
        return ret;
    }
    return SOFTBUS_OK;
}

void SoftbusAdapter::ReRegister()
{
    HILOGI("Re register softbus listener begin.");
    StopSoftbusEvent();
    int32_t ret = UnregisterSoftbusEventListener(softbusAdapterListener_);
    if (ret != ERR_OK) {
        HILOGE("UnregisterSoftbusEventListener failed, ret: %{public}d", ret);
        return;
    }
    std::shared_ptr<SoftbusAdapterListener> missionBroadcastListener =
        std::make_shared<DistributedMissionBroadcastListener>();
    ret = RegisterSoftbusEventListener(missionBroadcastListener);
    if (ret != ERR_OK) {
        HILOGE("RegisterSoftbusEventListener failed, ret: %{public}d", ret);
        return;
    }
    HILOGI("Re register softbus listener success.");
}
} // namespace DistributedSchedule
} // namespace OHOS
