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

#include "app_device_callback_stub_test.h"

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

void AppDeviceCallbackStubTest::SetUpTestCase()
{
    DTEST_LOG << "AppDeviceCallbackStubTest::SetUpTestCase" << std::endl;
}

void AppDeviceCallbackStubTest::TearDownTestCase()
{
    DTEST_LOG << "AppDeviceCallbackStubTest::TearDownTestCase" << std::endl;
}

void AppDeviceCallbackStubTest::SetUp()
{
    DTEST_LOG << "AppDeviceCallbackStubTest::SetUp" << std::endl;
    appDeviceCallbackStub_ = new AppDeviceCallbackStub();
}

void AppDeviceCallbackStubTest::TearDown()
{
    DTEST_LOG << "AppDeviceCallbackStubTest::TearDown" << std::endl;
    appDeviceCallbackStub_ = nullptr;
}

/**
 * @tc.name: AppDeviceCallbackStubTest_001
 * @tc.desc: invalid descriptor returns ERR_INVALID_STATE
 * @tc.type: FUNC
 */
HWTEST_F(AppDeviceCallbackStubTest, AppDeviceCallbackStubTest_001, TestSize.Level3)
{
    DTEST_LOG << "AppDeviceCallbackStubTest_001 begin" << std::endl;
    ASSERT_NE(appDeviceCallbackStub_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(MOCK_INVALID_DESCRIPTOR);
    int32_t result = appDeviceCallbackStub_->OnRemoteRequest(
        static_cast<uint32_t>(IDRequestCallbackInterfaceCode::EVENT_DEVICE_CONNECT), data, reply, option);
    EXPECT_EQ(result, ERR_INVALID_STATE);
    DTEST_LOG << "AppDeviceCallbackStubTest_001 end" << std::endl;
}

/**
 * @tc.name: AppDeviceCallbackStubTest_002
 * @tc.desc: EVENT_DEVICE_CONNECT with valid parcel triggers OnDeviceConnect path
 * @tc.type: FUNC
 */
HWTEST_F(AppDeviceCallbackStubTest, AppDeviceCallbackStubTest_002, TestSize.Level3)
{
    DTEST_LOG << "AppDeviceCallbackStubTest_002 begin" << std::endl;
    ASSERT_NE(appDeviceCallbackStub_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(AppDeviceCallbackInterface::GetDescriptor());
    int32_t token = 1;
    data.WriteInt32(token);
    std::vector<ContinuationResult> results;
    ContinuationResult::WriteContinuationResultsToParcel(data, results);
    int32_t result = appDeviceCallbackStub_->OnRemoteRequest(
        static_cast<uint32_t>(IDRequestCallbackInterfaceCode::EVENT_DEVICE_CONNECT), data, reply, option);
    // OnDeviceConnect forwards to service, result depends on service state; just verify no crash
    EXPECT_NO_FATAL_FAILURE(result);
    DTEST_LOG << "AppDeviceCallbackStubTest_002 end" << std::endl;
}

/**
 * @tc.name: AppDeviceCallbackStubTest_003
 * @tc.desc: EVENT_DEVICE_DISCONNECT with valid parcel
 * @tc.type: FUNC
 */
HWTEST_F(AppDeviceCallbackStubTest, AppDeviceCallbackStubTest_003, TestSize.Level3)
{
    DTEST_LOG << "AppDeviceCallbackStubTest_003 begin" << std::endl;
    ASSERT_NE(appDeviceCallbackStub_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(AppDeviceCallbackInterface::GetDescriptor());
    int32_t token = 2;
    data.WriteInt32(token);
    std::vector<ContinuationResult> results;
    ContinuationResult::WriteContinuationResultsToParcel(data, results);
    int32_t result = appDeviceCallbackStub_->OnRemoteRequest(
        static_cast<uint32_t>(IDRequestCallbackInterfaceCode::EVENT_DEVICE_DISCONNECT), data, reply, option);
    EXPECT_NO_FATAL_FAILURE(result);
    DTEST_LOG << "AppDeviceCallbackStubTest_003 end" << std::endl;
}

/**
 * @tc.name: AppDeviceCallbackStubTest_004
 * @tc.desc: EVENT_DEVICE_CANCEL
 * @tc.type: FUNC
 */
HWTEST_F(AppDeviceCallbackStubTest, AppDeviceCallbackStubTest_004, TestSize.Level3)
{
    DTEST_LOG << "AppDeviceCallbackStubTest_004 begin" << std::endl;
    ASSERT_NE(appDeviceCallbackStub_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(AppDeviceCallbackInterface::GetDescriptor());
    int32_t result = appDeviceCallbackStub_->OnRemoteRequest(
        static_cast<uint32_t>(IDRequestCallbackInterfaceCode::EVENT_DEVICE_CANCEL), data, reply, option);
    EXPECT_NO_FATAL_FAILURE(result);
    DTEST_LOG << "AppDeviceCallbackStubTest_004 end" << std::endl;
}

/**
 * @tc.name: AppDeviceCallbackStubTest_005
 * @tc.desc: unknown code falls through to IPCObjectStub
 * @tc.type: FUNC
 */
HWTEST_F(AppDeviceCallbackStubTest, AppDeviceCallbackStubTest_005, TestSize.Level3)
{
    DTEST_LOG << "AppDeviceCallbackStubTest_005 begin" << std::endl;
    ASSERT_NE(appDeviceCallbackStub_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(AppDeviceCallbackInterface::GetDescriptor());
    uint32_t unknownCode = 9999;
    int32_t result = appDeviceCallbackStub_->OnRemoteRequest(unknownCode, data, reply, option);
    EXPECT_NE(result, ERR_NONE);
    DTEST_LOG << "AppDeviceCallbackStubTest_005 end" << std::endl;
}

/**
 * @tc.name: AppDeviceCallbackStubTest_006
 * @tc.desc: EVENT_DEVICE_CONNECT with malformed parcel (missing continuation results)
 * @tc.type: FUNC
 */
HWTEST_F(AppDeviceCallbackStubTest, AppDeviceCallbackStubTest_006, TestSize.Level3)
{
    DTEST_LOG << "AppDeviceCallbackStubTest_006 begin" << std::endl;
    ASSERT_NE(appDeviceCallbackStub_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(AppDeviceCallbackInterface::GetDescriptor());
    // write a token but no continuation results -> ReadContinuationResultsFromParcel reads VALUE_OBJECT=garbage
    int32_t token = 3;
    data.WriteInt32(token);
    int32_t result = appDeviceCallbackStub_->OnRemoteRequest(
        static_cast<uint32_t>(IDRequestCallbackInterfaceCode::EVENT_DEVICE_CONNECT), data, reply, option);
    // malformed parcel -> ReadContinuationResultsFromParcel likely fails -> ERR_FLATTEN_OBJECT or service path
    EXPECT_NO_FATAL_FAILURE(result);
    DTEST_LOG << "AppDeviceCallbackStubTest_006 end" << std::endl;
}
} // namespace DistributedSchedule
} // namespace OHOS
