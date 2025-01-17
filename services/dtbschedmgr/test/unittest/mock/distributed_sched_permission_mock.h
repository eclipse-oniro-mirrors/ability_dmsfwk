/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#ifndef DISTRIBUTED_SCHED_PERMISSION_MOCK_H
#define DISTRIBUTED_SCHED_PERMISSION_MOCK_H

#include <gmock/gmock.h>

#include "distributed_sched_permission.h"

namespace OHOS {
namespace DistributedSchedule {
using namespace OHOS::AppExecFwk;
class IDistributedSchedPerm {
public:
    virtual ~IDistributedSchedPerm() = default;
    virtual int32_t GetAccountInfo(const std::string& remoteNetworkId, const CallerInfo& callerInfo,
        IDistributedSched::AccountInfo& accountInfo) = 0;
public:
    static inline std::shared_ptr<IDistributedSchedPerm> dmsPermMock = nullptr;
};

class DistributedSchedPermMock : public IDistributedSchedPerm {
public:
    MOCK_METHOD3(GetAccountInfo, int32_t(const std::string& remoteNetworkId, const CallerInfo& callerInfo,
        IDistributedSched::AccountInfo& accountInfo));
};
}
}
#endif