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

#include "device_selection_notifier_stub_test.h"

#include "continuation_result.h"
#include "message_parcel.h"
#include "test_log.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace DistributedSchedule {
namespace {
const std::u16string MOCK_INVALID_DESCRIPTOR = u"invalid descriptor";
}

void DeviceSelectionNotifierStubTest::SetUpTestCase()
{
    DTEST_LOG << "DeviceSelectionNotifierStubTest::SetUpTestCase" << std::endl;
}

void DeviceSelectionNotifierStubTest::TearDownTestCase()
{
    DTEST_LOG << "DeviceSelectionNotifierStubTest::TearDownTestCase" << std::endl;
}

void DeviceSelectionNotifierStubTest::SetUp()
{
    DTEST_LOG << "DeviceSelectionNotifierStubTest::SetUp" << std::endl;
    notifierStub_ = new MockDeviceSelectionNotifier();
}

void DeviceSelectionNotifierStubTest::TearDown()
{
    DTEST_LOG << "DeviceSelectionNotifierStubTest::TearDown" << std::endl;
    notifierStub_ = nullptr;
}

/**
 * @tc.name: DeviceSelectionNotifierStubTest_001
 * @tc.desc: invalid descriptor returns ERR_INVALID_VALUE
 * @tc.type: FUNC
 */
HWTEST_F(DeviceSelectionNotifierStubTest, DeviceSelectionNotifierStubTest_001, TestSize.Level3)
{
    DTEST_LOG << "DeviceSelectionNotifierStubTest_001 begin" << std::endl;
    ASSERT_NE(notifierStub_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(MOCK_INVALID_DESCRIPTOR);
    int32_t result = notifierStub_->OnRemoteRequest(
        static_cast<uint32_t>(IDRequestInterfaceCode::EVENT_DEVICE_CONNECT), data, reply, option);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    DTEST_LOG << "DeviceSelectionNotifierStubTest_001 end" << std::endl;
}

/**
 * @tc.name: DeviceSelectionNotifierStubTest_002
 * @tc.desc: EVENT_DEVICE_CONNECT with valid parcel dispatches OnDeviceConnect
 * @tc.type: FUNC
 */
HWTEST_F(DeviceSelectionNotifierStubTest, DeviceSelectionNotifierStubTest_002, TestSize.Level3)
{
    DTEST_LOG << "DeviceSelectionNotifierStubTest_002 begin" << std::endl;
    ASSERT_NE(notifierStub_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(IDeviceSelectionNotifier::GetDescriptor());
    std::vector<ContinuationResult> results;
    ContinuationResult r;
    r.SetDeviceId("dev1");
    results.push_back(r);
    ContinuationResult::WriteContinuationResultsToParcel(data, results);
    int32_t result = notifierStub_->OnRemoteRequest(
        static_cast<uint32_t>(IDRequestInterfaceCode::EVENT_DEVICE_CONNECT), data, reply, option);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_TRUE(notifierStub_->connectCalled_);
    EXPECT_EQ(notifierStub_->lastResults_.size(), results.size());
    DTEST_LOG << "DeviceSelectionNotifierStubTest_002 end" << std::endl;
}

/**
 * @tc.name: DeviceSelectionNotifierStubTest_003
 * @tc.desc: EVENT_DEVICE_DISCONNECT with valid parcel dispatches OnDeviceDisconnect
 * @tc.type: FUNC
 */
HWTEST_F(DeviceSelectionNotifierStubTest, DeviceSelectionNotifierStubTest_003, TestSize.Level3)
{
    DTEST_LOG << "DeviceSelectionNotifierStubTest_003 begin" << std::endl;
    ASSERT_NE(notifierStub_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(IDeviceSelectionNotifier::GetDescriptor());
    std::vector<ContinuationResult> results;
    ContinuationResult::WriteContinuationResultsToParcel(data, results);
    int32_t result = notifierStub_->OnRemoteRequest(
        static_cast<uint32_t>(IDRequestInterfaceCode::EVENT_DEVICE_DISCONNECT), data, reply, option);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_TRUE(notifierStub_->disconnectCalled_);
    DTEST_LOG << "DeviceSelectionNotifierStubTest_003 end" << std::endl;
}

/**
 * @tc.name: DeviceSelectionNotifierStubTest_004
 * @tc.desc: unknown code falls through to IPCObjectStub
 * @tc.type: FUNC
 */
HWTEST_F(DeviceSelectionNotifierStubTest, DeviceSelectionNotifierStubTest_004, TestSize.Level3)
{
    DTEST_LOG << "DeviceSelectionNotifierStubTest_004 begin" << std::endl;
    ASSERT_NE(notifierStub_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(IDeviceSelectionNotifier::GetDescriptor());
    uint32_t unknownCode = 9999;
    int32_t result = notifierStub_->OnRemoteRequest(unknownCode, data, reply, option);
    EXPECT_NE(result, ERR_OK);
    DTEST_LOG << "DeviceSelectionNotifierStubTest_004 end" << std::endl;
}

/**
 * @tc.name: DeviceSelectionNotifierStubTest_005
 * @tc.desc: EVENT_DEVICE_CONNECT with empty results
 * @tc.type: FUNC
 */
HWTEST_F(DeviceSelectionNotifierStubTest, DeviceSelectionNotifierStubTest_005, TestSize.Level3)
{
    DTEST_LOG << "DeviceSelectionNotifierStubTest_005 begin" << std::endl;
    ASSERT_NE(notifierStub_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(IDeviceSelectionNotifier::GetDescriptor());
    std::vector<ContinuationResult> emptyResults;
    ContinuationResult::WriteContinuationResultsToParcel(data, emptyResults);
    int32_t result = notifierStub_->OnRemoteRequest(
        static_cast<uint32_t>(IDRequestInterfaceCode::EVENT_DEVICE_CONNECT), data, reply, option);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_TRUE(notifierStub_->connectCalled_);
    EXPECT_TRUE(notifierStub_->lastResults_.empty());
    DTEST_LOG << "DeviceSelectionNotifierStubTest_005 end" << std::endl;
}
} // namespace DistributedSchedule
} // namespace OHOS
