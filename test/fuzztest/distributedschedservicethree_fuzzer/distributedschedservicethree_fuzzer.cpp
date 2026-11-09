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

#include "distributedschedservicethree_fuzzer.h"

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
constexpr int32_t ON_DEMAND_REASON_ID_COUNT = 7;
constexpr int32_t FUZZ_BRANCH_COUNT = 4;
enum FuzzCaseIndex {
    ON_START,
    ON_ADD_ABILITY,
    CONTINUE_STATE_UNREGISTER,
    CONTINUE_MISSION,
};

std::string GetDExtensionName(std::string bundleName, int32_t userId);
std::string GetDExtensionProcess(std::string bundleName, int32_t userId);

void OnStartFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t) + sizeof(int32_t))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    std::string reasonName = fdp.ConsumeRandomLengthString();
    std::string reasonValue = fdp.ConsumeRandomLengthString();
    int32_t enumIdx = fdp.ConsumeIntegral<int32_t>() % ON_DEMAND_REASON_ID_COUNT;
    OnDemandReasonId reasonId = static_cast<OnDemandReasonId>(enumIdx);
    int32_t extraDataId = fdp.ConsumeIntegral<int32_t>();
    SystemAbilityOnDemandReason reason(reasonId, reasonName, reasonValue, extraDataId);
    DistributedSchedService::GetInstance().OnStart(reason);
}

void OnAddSystemAbilityFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    int32_t systemAbilityId = fdp.ConsumeIntegral<int32_t>();
    std::string deviceId = fdp.ConsumeRandomLengthString();
    DistributedSchedService::GetInstance().OnAddSystemAbility(systemAbilityId, deviceId);
}

void ContinueStateCallbackUnRegisterFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    int32_t missionId = fdp.ConsumeIntegral<int32_t>();
    std::string bundleName = fdp.ConsumeRandomLengthString();
    std::string moduleName = fdp.ConsumeRandomLengthString();
    std::string abilityName = fdp.ConsumeRandomLengthString();
    DistributedSchedService::GetInstance().ContinueStateCallbackUnRegister(missionId, bundleName,
        moduleName, abilityName);
}

void ContinueMissionFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    std::string srcDeviceId = fdp.ConsumeRandomLengthString();
    std::string dstDeviceId = fdp.ConsumeRandomLengthString();
    int32_t missionId = fdp.ConsumeIntegral<int32_t>();
    sptr<IRemoteObject> callback = nullptr;
    OHOS::AAFwk::WantParams wantParams;
    DistributedSchedService::GetInstance().ContinueMission(srcDeviceId, dstDeviceId, missionId, callback, wantParams);
}

constexpr int32_t STOP_SERVICE_WAIT_MS = 200;

void StopServiceForFuzzCleanup()
{
    SystemAbilityOnDemandReason stopReason;
    DistributedSchedService::GetInstance().OnStop(stopReason);
    std::this_thread::sleep_for(std::chrono::milliseconds(STOP_SERVICE_WAIT_MS));
}
}
}
/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(uint8_t))) {
        return 0;
    }
    uint8_t fuzzChoice = data[0];
    switch (fuzzChoice % OHOS::DistributedSchedule::FUZZ_BRANCH_COUNT) {
        case OHOS::DistributedSchedule::ON_START:
            OHOS::DistributedSchedule::OnStartFuzzTest(data, size);
            return 0;
        case OHOS::DistributedSchedule::ON_ADD_ABILITY:
            OHOS::DistributedSchedule::OnAddSystemAbilityFuzzTest(data, size);
            break;
        case OHOS::DistributedSchedule::CONTINUE_STATE_UNREGISTER:
            OHOS::DistributedSchedule::ContinueStateCallbackUnRegisterFuzzTest(data, size);
            break;
        case OHOS::DistributedSchedule::CONTINUE_MISSION:
            OHOS::DistributedSchedule::ContinueMissionFuzzTest(data, size);
            break;
        default:
            break;
    }
    // Stop the service to clean up async threads (MissionManagerHandler, DmsExCmptChange, etc.)
    OHOS::DistributedSchedule::StopServiceForFuzzCleanup();
    return 0;
}
