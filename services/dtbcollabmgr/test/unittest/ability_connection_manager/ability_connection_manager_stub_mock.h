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
#ifndef OHOS_DISTRIBUTED_ABILITY_CONNECTION_MANAGER_STUB_MOCK_H
#define OHOS_DISTRIBUTED_ABILITY_CONNECTION_MANAGER_STUB_MOCK_H

#include "gmock/gmock.h"

#include "ability_connection_manager_stub.h"

namespace OHOS {
namespace DistributedCollab {
class AbilityConnectionManagerStubMock : public AbilityConnectionManagerStub {
public:
    MOCK_METHOD5(NotifyCollabResult, int32_t(int32_t, int32_t, const std::string&,
        const std::string&, const std::string&));

    MOCK_METHOD1(NotifyDisconnect, int32_t(int32_t sessionId));
    MOCK_METHOD1(NotifyWifiOpen, int32_t(int32_t sessionId));
    MOCK_METHOD2(NotifyPeerVersion, int32_t(int32_t sessionId, int32_t version));
};
}  // namespace DistributedCollab
}  // namespace OHOS
#endif /* OHOS_DISTRIBUTED_ABILITY_CONNECTION_MANAGER_STUB_MOCK_H */