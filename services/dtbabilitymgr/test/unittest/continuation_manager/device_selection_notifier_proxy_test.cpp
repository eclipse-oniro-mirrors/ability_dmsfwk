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

#include "device_selection_notifier_proxy_test.h"

#include "continuation_result.h"
#include "test_log.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace DistributedSchedule {

void DeviceSelectionNotifierProxyTest::SetUpTestCase()
{
    DTEST_LOG << "DeviceSelectionNotifierProxyTest::SetUpTestCase" << std::endl;
}

void DeviceSelectionNotifierProxyTest::TearDownTestCase()
{
    DTEST_LOG << "DeviceSelectionNotifierProxyTest::TearDownTestCase" << std::endl;
}

void DeviceSelectionNotifierProxyTest::SetUp()
{
    DTEST_LOG << "DeviceSelectionNotifierProxyTest::SetUp" << std::endl;
    remote_ = new MockRecordingRemote();
    proxy_ = std::make_shared<DeviceSelectionNotifierProxy>(remote_);
}

void DeviceSelectionNotifierProxyTest::TearDown()
{
    DTEST_LOG << "DeviceSelectionNotifierProxyTest::TearDown" << std::endl;
    proxy_ = nullptr;
    remote_ = nullptr;
}

/**
 * @tc.name: DeviceSelectionNotifierProxyTest_001
 * @tc.desc: OnDeviceConnect with empty results sends request with EVENT_DEVICE_CONNECT
 * @tc.type: FUNC
 */
HWTEST_F(DeviceSelectionNotifierProxyTest, DeviceSelectionNotifierProxyTest_001, TestSize.Level3)
{
    DTEST_LOG << "DeviceSelectionNotifierProxyTest_001 begin" << std::endl;
    ASSERT_NE(proxy_, nullptr);
    std::vector<ContinuationResult> results;
    EXPECT_NO_FATAL_FAILURE(proxy_->OnDeviceConnect(results));
    EXPECT_TRUE(remote_->called_);
    EXPECT_EQ(remote_->lastCode_, static_cast<uint32_t>(IDRequestInterfaceCode::EVENT_DEVICE_CONNECT));
    DTEST_LOG << "DeviceSelectionNotifierProxyTest_001 end" << std::endl;
}

/**
 * @tc.name: DeviceSelectionNotifierProxyTest_002
 * @tc.desc: OnDeviceDisconnect sends request with EVENT_DEVICE_DISCONNECT
 * @tc.type: FUNC
 */
HWTEST_F(DeviceSelectionNotifierProxyTest, DeviceSelectionNotifierProxyTest_002, TestSize.Level3)
{
    DTEST_LOG << "DeviceSelectionNotifierProxyTest_002 begin" << std::endl;
    ASSERT_NE(proxy_, nullptr);
    std::vector<ContinuationResult> results;
    EXPECT_NO_FATAL_FAILURE(proxy_->OnDeviceDisconnect(results));
    EXPECT_TRUE(remote_->called_);
    EXPECT_EQ(remote_->lastCode_, static_cast<uint32_t>(IDRequestInterfaceCode::EVENT_DEVICE_DISCONNECT));
    DTEST_LOG << "DeviceSelectionNotifierProxyTest_002 end" << std::endl;
}

/**
 * @tc.name: DeviceSelectionNotifierProxyTest_003
 * @tc.desc: OnDeviceConnect with non-empty results does not crash
 * @tc.type: FUNC
 */
HWTEST_F(DeviceSelectionNotifierProxyTest, DeviceSelectionNotifierProxyTest_003, TestSize.Level3)
{
    DTEST_LOG << "DeviceSelectionNotifierProxyTest_003 begin" << std::endl;
    ASSERT_NE(proxy_, nullptr);
    std::vector<ContinuationResult> results;
    ContinuationResult r;
    r.SetDeviceId("dev1");
    r.SetDeviceType("tablet");
    r.SetDeviceName("MyTablet");
    results.push_back(r);
    EXPECT_NO_FATAL_FAILURE(proxy_->OnDeviceConnect(results));
    EXPECT_TRUE(remote_->called_);
    DTEST_LOG << "DeviceSelectionNotifierProxyTest_003 end" << std::endl;
}

/**
 * @tc.name: DeviceSelectionNotifierProxyTest_004
 * @tc.desc: OnDeviceDisconnect with non-empty results does not crash
 * @tc.type: FUNC
 */
HWTEST_F(DeviceSelectionNotifierProxyTest, DeviceSelectionNotifierProxyTest_004, TestSize.Level3)
{
    DTEST_LOG << "DeviceSelectionNotifierProxyTest_004 begin" << std::endl;
    ASSERT_NE(proxy_, nullptr);
    std::vector<ContinuationResult> results;
    ContinuationResult r;
    r.SetDeviceId("dev2");
    results.push_back(r);
    EXPECT_NO_FATAL_FAILURE(proxy_->OnDeviceDisconnect(results));
    EXPECT_TRUE(remote_->called_);
    DTEST_LOG << "DeviceSelectionNotifierProxyTest_004 end" << std::endl;
}
} // namespace DistributedSchedule
} // namespace OHOS
