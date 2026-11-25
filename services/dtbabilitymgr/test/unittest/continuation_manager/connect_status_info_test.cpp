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

#include "connect_status_info_test.h"

#include "test_log.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace DistributedSchedule {
namespace {
const std::string MOCK_DEVICE_ID = "123456789";
}

void ConnectStatusInfoTest::SetUpTestCase()
{
    DTEST_LOG << "ConnectStatusInfoTest::SetUpTestCase" << std::endl;
}

void ConnectStatusInfoTest::TearDownTestCase()
{
    DTEST_LOG << "ConnectStatusInfoTest::TearDownTestCase" << std::endl;
}

void ConnectStatusInfoTest::SetUp()
{
    DTEST_LOG << "ConnectStatusInfoTest::SetUp" << std::endl;
}

void ConnectStatusInfoTest::TearDown()
{
    DTEST_LOG << "ConnectStatusInfoTest::TearDown" << std::endl;
}

/**
 * @tc.name: ConnectStatusInfoTest_001
 * @tc.desc: construct with deviceId and IDLE status, verify getters
 * @tc.type: FUNC
 */
HWTEST_F(ConnectStatusInfoTest, ConnectStatusInfoTest_001, TestSize.Level3)
{
    DTEST_LOG << "ConnectStatusInfoTest ConnectStatusInfoTest_001 begin" << std::endl;
    ConnectStatusInfo info(MOCK_DEVICE_ID, DeviceConnectStatus::IDLE);
    EXPECT_EQ(info.GetDeviceId(), MOCK_DEVICE_ID);
    EXPECT_EQ(info.GetDeviceConnectStatus(), DeviceConnectStatus::IDLE);
    DTEST_LOG << "ConnectStatusInfoTest ConnectStatusInfoTest_001 end" << std::endl;
}

/**
 * @tc.name: ConnectStatusInfoTest_002
 * @tc.desc: construct with CONNECTED status and verify
 * @tc.type: FUNC
 */
HWTEST_F(ConnectStatusInfoTest, ConnectStatusInfoTest_002, TestSize.Level3)
{
    DTEST_LOG << "ConnectStatusInfoTest ConnectStatusInfoTest_002 begin" << std::endl;
    ConnectStatusInfo info("devA", DeviceConnectStatus::CONNECTED);
    EXPECT_EQ(info.GetDeviceConnectStatus(), DeviceConnectStatus::CONNECTED);
    DTEST_LOG << "ConnectStatusInfoTest ConnectStatusInfoTest_002 end" << std::endl;
}

/**
 * @tc.name: ConnectStatusInfoTest_003
 * @tc.desc: SetDeviceId updates value
 * @tc.type: FUNC
 */
HWTEST_F(ConnectStatusInfoTest, ConnectStatusInfoTest_003, TestSize.Level3)
{
    DTEST_LOG << "ConnectStatusInfoTest ConnectStatusInfoTest_003 begin" << std::endl;
    ConnectStatusInfo info("", DeviceConnectStatus::IDLE);
    info.SetDeviceId("newDevice");
    EXPECT_EQ(info.GetDeviceId(), "newDevice");
    DTEST_LOG << "ConnectStatusInfoTest ConnectStatusInfoTest_003 end" << std::endl;
}

/**
 * @tc.name: ConnectStatusInfoTest_004
 * @tc.desc: SetDeviceConnectStatus updates value across all enum members
 * @tc.type: FUNC
 */
HWTEST_F(ConnectStatusInfoTest, ConnectStatusInfoTest_004, TestSize.Level3)
{
    DTEST_LOG << "ConnectStatusInfoTest ConnectStatusInfoTest_004 begin" << std::endl;
    ConnectStatusInfo info(MOCK_DEVICE_ID, DeviceConnectStatus::IDLE);
    info.SetDeviceConnectStatus(DeviceConnectStatus::CONNECTING);
    EXPECT_EQ(info.GetDeviceConnectStatus(), DeviceConnectStatus::CONNECTING);
    info.SetDeviceConnectStatus(DeviceConnectStatus::DISCONNECTING);
    EXPECT_EQ(info.GetDeviceConnectStatus(), DeviceConnectStatus::DISCONNECTING);
    DTEST_LOG << "ConnectStatusInfoTest ConnectStatusInfoTest_004 end" << std::endl;
}

/**
 * @tc.name: ConnectStatusInfoTest_005
 * @tc.desc: SetDeviceId with empty string
 * @tc.type: FUNC
 */
HWTEST_F(ConnectStatusInfoTest, ConnectStatusInfoTest_005, TestSize.Level3)
{
    DTEST_LOG << "ConnectStatusInfoTest ConnectStatusInfoTest_005 begin" << std::endl;
    ConnectStatusInfo info(MOCK_DEVICE_ID, DeviceConnectStatus::IDLE);
    info.SetDeviceId("");
    EXPECT_EQ(info.GetDeviceId(), "");
    DTEST_LOG << "ConnectStatusInfoTest ConnectStatusInfoTest_005 end" << std::endl;
}
} // namespace DistributedSchedule
} // namespace OHOS
