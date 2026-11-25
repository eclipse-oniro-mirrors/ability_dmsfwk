/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef DEVICE_SELECTION_NOTIFIER_PROXY_TEST_H
#define DEVICE_SELECTION_NOTIFIER_PROXY_TEST_H

#include "gtest/gtest.h"

#include "continuation_manager/device_selection_notifier_proxy.h"
#include "mock_remote_stub.h"

namespace OHOS {
namespace DistributedSchedule {
// Mock remote that records the last request code
class MockRecordingRemote : public MockRemoteStub {
public:
    int32_t OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply,
        MessageOption& option) override
    {
        lastCode_ = code;
        called_ = true;
        return ERR_OK;
    }
    uint32_t lastCode_ = 0;
    bool called_ = false;
};

class DeviceSelectionNotifierProxyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    sptr<MockRecordingRemote> remote_;
    std::shared_ptr<DeviceSelectionNotifierProxy> proxy_;
};
} // namespace DistributedSchedule
} // namespace OHOS
#endif // DEVICE_SELECTION_NOTIFIER_PROXY_TEST_H
