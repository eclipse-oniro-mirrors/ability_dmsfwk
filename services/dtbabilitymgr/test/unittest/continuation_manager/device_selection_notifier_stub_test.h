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

#ifndef DEVICE_SELECTION_NOTIFIER_STUB_TEST_H
#define DEVICE_SELECTION_NOTIFIER_STUB_TEST_H

#include "gtest/gtest.h"

#include "device_selection_notifier_stub.h"

namespace OHOS {
namespace DistributedSchedule {
// Concrete subclass for testing the stub's OnRemoteRequest dispatch
class MockDeviceSelectionNotifier : public DeviceSelectionNotifierStub {
public:
    void OnDeviceConnect(const std::vector<ContinuationResult>& continuationResults) override
    {
        connectCalled_ = true;
        lastResults_ = continuationResults;
    }
    void OnDeviceDisconnect(const std::vector<ContinuationResult>& continuationResults) override
    {
        disconnectCalled_ = true;
        lastResults_ = continuationResults;
    }
    bool connectCalled_ = false;
    bool disconnectCalled_ = false;
    std::vector<ContinuationResult> lastResults_;
};

class DeviceSelectionNotifierStubTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    sptr<MockDeviceSelectionNotifier> notifierStub_;
};
} // namespace DistributedSchedule
} // namespace OHOS
#endif // DEVICE_SELECTION_NOTIFIER_STUB_TEST_H
