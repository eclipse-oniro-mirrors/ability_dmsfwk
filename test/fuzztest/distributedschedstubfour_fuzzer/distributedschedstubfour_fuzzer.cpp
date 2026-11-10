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

#include "distributedschedstubfour_fuzzer.h"

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
const std::string TAG = "DistributedSchedFuzzTest";
constexpr int32_t FUZZ_BRANCH_COUNT = 5;
enum FuzzCaseIndex {
    START_FREE_INSTALL,
    START_SHARE_FORM,
    STOP_EXTENSION_ABILITY,
    REGISTER_ON_LISTENER,
    STOP_SYNC_MISSIONS,
};
namespace {
const std::u16string DMS_STUB_INTERFACE_TOKEN = u"ohos.distributedschedule.accessToken";
}

void StartRemoteFreeInstallInnerFuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr) {
        return;
    }
    FuzzUtil::MockPermission();
    MessageParcel dataParcel;
    MessageParcel reply;
    MessageOption option;
    FuzzedDataProvider fdp(data, size);
    int32_t int32Data = fdp.ConsumeIntegral<int32_t>();
    uint32_t uint32Data = fdp.ConsumeIntegral<uint32_t>();
    sptr<IRemoteObject> obj(new MockDistributedSched());
    Want want;
    std::string dstDeviceId = fdp.ConsumeRandomLengthString();

    want.SetDeviceId(dstDeviceId);
    dataParcel.WriteInterfaceToken(DMS_STUB_INTERFACE_TOKEN);
    PARCEL_WRITE_HELPER_NORET(dataParcel, Parcelable, &want);
    PARCEL_WRITE_HELPER_NORET(dataParcel, Int32, int32Data);
    PARCEL_WRITE_HELPER_NORET(dataParcel, Int32, int32Data);
    PARCEL_WRITE_HELPER_NORET(dataParcel, Uint32, uint32Data);
    PARCEL_WRITE_HELPER_NORET(dataParcel, RemoteObject, obj);
    DistributedSchedService::GetInstance().StartRemoteFreeInstallInner(dataParcel, reply);
    DistributedSchedService::GetInstance().ProcessCallResult(obj, obj);
}

void StartRemoteShareFormInnerFuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr) {
        return;
    }
    FuzzUtil::MockPermission();
    int32_t code = static_cast<uint32_t>(IDSchedInterfaceCode::START_REMOTE_SHARE_FORM);
    FuzzedDataProvider fdp(data, size);
    MessageParcel dataParcel;
    MessageParcel reply;
    MessageOption option;
    std::string str = fdp.ConsumeRandomLengthString();
    Want want;
    std::string dstDeviceId = fdp.ConsumeRandomLengthString();

    want.SetDeviceId(dstDeviceId);
    dataParcel.WriteInterfaceToken(DMS_STUB_INTERFACE_TOKEN);
    PARCEL_WRITE_HELPER_NORET(dataParcel, String, str);
    PARCEL_WRITE_HELPER_NORET(dataParcel, Parcelable, &want);
    DistributedSchedService::GetInstance().OnRemoteRequest(code, dataParcel, reply, option);
}

void StopRemoteExtensionAbilityInnerFuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr) {
        return;
    }
    FuzzUtil::MockPermission();
    MessageParcel dataParcel;
    MessageParcel reply;
    MessageOption option;
    Want want;
    FuzzedDataProvider fdp(data, size);
    int32_t callerUid = fdp.ConsumeIntegral<int32_t>();
    int32_t serviceType = fdp.ConsumeIntegral<int32_t>();
    uint32_t accessToken = fdp.ConsumeIntegral<uint32_t>();
    std::string dstDeviceId = fdp.ConsumeRandomLengthString();
    std::string bundleName = fdp.ConsumeRandomLengthString();
    std::string abilityName = fdp.ConsumeRandomLengthString();

    want.SetDeviceId(dstDeviceId);
    want.SetElementName(bundleName, abilityName);
    dataParcel.WriteParcelable(&want);
    dataParcel.WriteInt32(callerUid);
    dataParcel.WriteUint32(accessToken);
    dataParcel.WriteInt32(serviceType);
    DistributedSchedService::GetInstance().StopRemoteExtensionAbilityInner(dataParcel, reply);

    CallerInfo callerInfo;
    std::string localDeviceId = fdp.ConsumeRandomLengthString();
    DistributedSchedService::GetInstance().GetCallerInfo(localDeviceId, callerUid, accessToken, callerInfo);
    DistributedSchedService::GetInstance().CheckDeviceIdFromRemote(localDeviceId, localDeviceId, localDeviceId);
}

void RegisterOnListenerInnerFuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    FuzzUtil::MockPermission();
    MessageParcel dataParcel;
    MessageParcel reply;
    MessageOption option;
    std::string str = fdp.ConsumeRandomLengthString();
    sptr<IRemoteObject> obj(new MockDistributedSched());

    PARCEL_WRITE_HELPER_NORET(dataParcel, String, str);
    PARCEL_WRITE_HELPER_NORET(dataParcel, RemoteObject, obj);
    DistributedSchedService::GetInstance().RegisterOnListenerInner(dataParcel, reply);
    DistributedSchedService::GetInstance().HandleLocalCallerDied(obj);
    DistributedSchedService::GetInstance().RemoveCallerComponent(obj);
    DistributedSchedService::GetInstance().RemoveConnectAbilityInfo(str);
    DistributedSchedService::GetInstance().DumpConnectInfo(str);
}

void StopSyncMissionsFromRemoteInnerFuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr) {
        return;
    }
    FuzzUtil::MockPermission();
    MessageParcel dataParcel;
    MessageParcel reply;
    MessageOption option;
    FuzzedDataProvider fdp(data, size);
    int32_t int32Data = fdp.ConsumeIntegral<int32_t>();
    std::string str = fdp.ConsumeRandomLengthString();

    PARCEL_WRITE_HELPER_NORET(dataParcel, Int32, int32Data);
    PARCEL_WRITE_HELPER_NORET(dataParcel, Int32, int32Data);
    PARCEL_WRITE_HELPER_NORET(dataParcel, Int32, int32Data);
    PARCEL_WRITE_HELPER_NORET(dataParcel, String, str);
    PARCEL_WRITE_HELPER_NORET(dataParcel, Int32, int32Data);
    PARCEL_WRITE_HELPER_NORET(dataParcel, String, str);
    PARCEL_WRITE_HELPER_NORET(dataParcel, Int32, int32Data);

    DistributedSchedService::GetInstance().StopSyncMissionsFromRemoteInner(dataParcel, reply);
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
    if ((data == nullptr) || (size < sizeof(uint8_t))) {
        return 0;
    }

    static OHOS::DistributedSchedule::FuzzCleanupGuard cleanupGuard;

    uint8_t fuzzChoice = data[0];
    switch (fuzzChoice % OHOS::DistributedSchedule::FUZZ_BRANCH_COUNT) {
        case OHOS::DistributedSchedule::START_FREE_INSTALL:
            OHOS::DistributedSchedule::StartRemoteFreeInstallInnerFuzzTest(data, size);
            return 0;
        case OHOS::DistributedSchedule::START_SHARE_FORM:
            OHOS::DistributedSchedule::StartRemoteShareFormInnerFuzzTest(data, size);
            return 0;
        case OHOS::DistributedSchedule::STOP_EXTENSION_ABILITY:
            OHOS::DistributedSchedule::StopRemoteExtensionAbilityInnerFuzzTest(data, size);
            return 0;
        case OHOS::DistributedSchedule::REGISTER_ON_LISTENER:
            OHOS::DistributedSchedule::RegisterOnListenerInnerFuzzTest(data, size);
            return 0;
        case OHOS::DistributedSchedule::STOP_SYNC_MISSIONS:
            OHOS::DistributedSchedule::StopSyncMissionsFromRemoteInnerFuzzTest(data, size);
            return 0;
        default:
            return 0;
    }
}
