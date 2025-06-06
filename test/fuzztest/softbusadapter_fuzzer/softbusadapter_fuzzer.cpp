/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "softbusadapter_fuzzer.h"

#include <cstddef>
#include "securec.h"
#include <cstdint>
#include <unistd.h>

#include "distributed_mission_broadcast_listener.h"
#include "softbus_adapter/softbus_adapter.h"

namespace OHOS {
namespace DistributedSchedule {
#ifdef DMSFWK_INTERACTIVE_ADAPTER
namespace {
    constexpr size_t CAPACITY = 10;
    const int32_t WAITTIME = 10000;
}
#endif

void FuzzSendSoftbusEvent(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(uint32_t))) {
        return;
    }
    std::shared_ptr<DSchedDataBuffer> buffer = std::make_shared<DSchedDataBuffer>(size);
    if (memcpy_s(buffer->Data(), buffer->Capacity(), data, size) != ERR_OK) {
        return;
    }
    SoftbusAdapter::GetInstance().SendSoftbusEvent(buffer);
    SoftbusAdapter::GetInstance().StopSoftbusEvent();
}

void FuzzOnBroadCastRecv(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(uint32_t))) {
        return;
    }
    std::string networkId(reinterpret_cast<const char*>(data), size);
    uint8_t* newdata = const_cast<uint8_t*>(data);
    uint32_t dataLen = *(reinterpret_cast<const uint32_t*>(data));
    SoftbusAdapter::GetInstance().OnBroadCastRecv(networkId, newdata, dataLen);
    std::shared_ptr<SoftbusAdapterListener> listener = std::make_shared<DistributedMissionBroadcastListener>();
    SoftbusAdapter::GetInstance().RegisterSoftbusEventListener(listener);
    SoftbusAdapter::GetInstance().UnregisterSoftbusEventListener(listener);
}

#ifdef DMSFWK_INTERACTIVE_ADAPTER
void FuzzDealSendSoftbusEvent(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(uint32_t))) {
        return;
    }
    std::shared_ptr<DSchedDataBuffer> buffer = nullptr;
    int32_t retry = *(reinterpret_cast<const int32_t*>(data));
    SoftbusAdapter::GetInstance().Init();
    SoftbusAdapter::GetInstance().DealSendSoftbusEvent(buffer, retry);
    SoftbusAdapter::GetInstance().RetrySendSoftbusEvent(buffer, retry);

    buffer = std::make_shared<DSchedDataBuffer>(CAPACITY);
    SoftbusAdapter::GetInstance().DealSendSoftbusEvent(buffer, retry);
    SoftbusAdapter::GetInstance().RetrySendSoftbusEvent(buffer, retry);
    usleep(WAITTIME);
    SoftbusAdapter::GetInstance().UnInit();
}
#endif
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::DistributedSchedule::FuzzSendSoftbusEvent(data, size);
    OHOS::DistributedSchedule::FuzzOnBroadCastRecv(data, size);
#ifdef DMSFWK_INTERACTIVE_ADAPTER
    OHOS::DistributedSchedule::FuzzDealSendSoftbusEvent(data, size);
#endif
    return 0;
}
