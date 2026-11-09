/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "distributedschedserviceone_fuzzer.h"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>
#include <singleton.h>
#include <thread>

#include "distributed_sched_interface.h"
#include "distributed_sched_service.h"
#include "distributed_sched_stub.h"
#include "distributedWant/distributed_want.h"
#include "mock_fuzz_util.h"
#include "mock_distributed_sched.h"
#include "parcel_helper.h"
#include "dms_continue_time_dumper.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace DistributedSchedule {
constexpr int32_t FUZZ_BRANCH_COUNT = 2;
enum FuzzCaseIndex {
    DEVICE_OFFLINE_NOTIFY,
    DEVICE_OFFLINE_AFTER_DELETE,
};

std::string GetDExtensionName(std::string bundleName, int32_t userId);
std::string GetDExtensionProcess(std::string bundleName, int32_t userId);

void DeviceOfflineNotifyFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }

    FuzzedDataProvider fdp(data, size);
    std::string networkId = fdp.ConsumeRandomLengthString();
    DistributedSchedService::GetInstance().DeviceOfflineNotify(networkId);
}

void DeviceOfflineNotifyAfterDeleteFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }

    FuzzedDataProvider fdp(data, size);
    std::string networkId = fdp.ConsumeRandomLengthString();
    DistributedSchedService::GetInstance().DeviceOfflineNotifyAfterDelete(networkId);
}

constexpr int32_t STOP_SERVICE_WAIT_MS = 200;

void StopServiceForFuzzCleanup()
{
    SystemAbilityOnDemandReason stopReason;
    DistributedSchedService::GetInstance().OnStop(stopReason);
    std::this_thread::sleep_for(std::chrono::milliseconds(STOP_SERVICE_WAIT_MS));
}

class FuzzCleanupGuard {
public:
    FuzzCleanupGuard() = default;
    ~FuzzCleanupGuard() { StopServiceForFuzzCleanup(); }
};
}
}
/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    static OHOS::DistributedSchedule::FuzzCleanupGuard cleanupGuard;

    if ((data == nullptr) || (size < sizeof(uint8_t))) {
        return 0;
    }
    uint8_t fuzzChoice = data[0];
    switch (fuzzChoice % OHOS::DistributedSchedule::FUZZ_BRANCH_COUNT) {
        case OHOS::DistributedSchedule::DEVICE_OFFLINE_NOTIFY:
            OHOS::DistributedSchedule::DeviceOfflineNotifyFuzzTest(data, size);
            return 0;
        case OHOS::DistributedSchedule::DEVICE_OFFLINE_AFTER_DELETE:
            OHOS::DistributedSchedule::DeviceOfflineNotifyAfterDeleteFuzzTest(data, size);
            return 0;
        default:
            break;
    }
    return 0;
}
