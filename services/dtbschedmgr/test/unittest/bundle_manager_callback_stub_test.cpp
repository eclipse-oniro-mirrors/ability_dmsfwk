/*
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

#include "bundle_manager_callback_stub_test.h"

#include "message_parcel.h"
#include "test_log.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace DistributedSchedule {
namespace {
const std::u16string MOCK_INVALID_DESCRIPTOR = u"invalid descriptor";
const uint32_t CODE_QUERY_DONE =
    static_cast<uint32_t>(IDBundleManagerCallbackInterfaceCod::ON_QUERY_INSTALLATION_DONE);
const uint32_t CODE_UNKNOWN_HIGH = 9999;
const uint32_t CODE_ZERO = 0;
const int32_t RESULT_CODE_FAILURE = 1;
const int32_t RESULT_CODE_OK = 0;
const uint32_t VERSION_CODE = 100;
const int32_t MISSION_ID = 1;
}

void DmsBundleManagerCallbackStubTest::SetUpTestCase()
{
    DTEST_LOG << "DmsBundleManagerCallbackStubTest::SetUpTestCase" << std::endl;
}

void DmsBundleManagerCallbackStubTest::TearDownTestCase()
{
    DTEST_LOG << "DmsBundleManagerCallbackStubTest::TearDownTestCase" << std::endl;
}

void DmsBundleManagerCallbackStubTest::SetUp()
{
    DTEST_LOG << "DmsBundleManagerCallbackStubTest::SetUp" << std::endl;
    stub_ = new DmsBundleManagerCallbackStub();
}

void DmsBundleManagerCallbackStubTest::TearDown()
{
    DTEST_LOG << "DmsBundleManagerCallbackStubTest::TearDown" << std::endl;
    stub_ = nullptr;
}

/**
 * @tc.name: testConstructor_001
 * @tc.desc: verify stub constructs and registers the ON_QUERY_INSTALLATION_DONE handler
 * @tc.type: FUNC
 */
HWTEST_F(DmsBundleManagerCallbackStubTest, testConstructor_001, TestSize.Level3)
{
    DTEST_LOG << "DmsBundleManagerCallbackStubTest testConstructor_001 begin" << std::endl;
    ASSERT_NE(stub_, nullptr);
    EXPECT_EQ(stub_->memberFuncMap_.size(), static_cast<size_t>(1));
    EXPECT_EQ(stub_->memberFuncMap_.count(CODE_QUERY_DONE), static_cast<size_t>(1));
    DTEST_LOG << "DmsBundleManagerCallbackStubTest testConstructor_001 end" << std::endl;
}

/**
 * @tc.name: testConstructor_002
 * @tc.desc: verify a second independent stub also has exactly one handler registered
 * @tc.type: FUNC
 */
HWTEST_F(DmsBundleManagerCallbackStubTest, testConstructor_002, TestSize.Level3)
{
    DTEST_LOG << "DmsBundleManagerCallbackStubTest testConstructor_002 begin" << std::endl;
    sptr<DmsBundleManagerCallbackStub> another = new DmsBundleManagerCallbackStub();
    ASSERT_NE(another, nullptr);
    EXPECT_EQ(another->memberFuncMap_.size(), static_cast<size_t>(1));
    EXPECT_EQ(another->memberFuncMap_.count(CODE_QUERY_DONE), static_cast<size_t>(1));
    DTEST_LOG << "DmsBundleManagerCallbackStubTest testConstructor_002 end" << std::endl;
}

/**
 * @tc.name: testOnRemoteRequest_TokenMismatch_001
 * @tc.desc: OnRemoteRequest with an invalid interface token returns ERR_INVALID_STATE
 * @tc.type: FUNC
 */
HWTEST_F(DmsBundleManagerCallbackStubTest, testOnRemoteRequest_TokenMismatch_001, TestSize.Level3)
{
    DTEST_LOG << "DmsBundleManagerCallbackStubTest testOnRemoteRequest_TokenMismatch_001 begin" << std::endl;
    ASSERT_NE(stub_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(MOCK_INVALID_DESCRIPTOR);
    int32_t result = stub_->OnRemoteRequest(CODE_QUERY_DONE, data, reply, option);
    EXPECT_EQ(result, ERR_INVALID_STATE);
    DTEST_LOG << "DmsBundleManagerCallbackStubTest testOnRemoteRequest_TokenMismatch_001 end" << std::endl;
}

/**
 * @tc.name: testOnRemoteRequest_TokenMismatch_002
 * @tc.desc: OnRemoteRequest with an empty interface token returns ERR_INVALID_STATE
 * @tc.type: FUNC
 */
HWTEST_F(DmsBundleManagerCallbackStubTest, testOnRemoteRequest_TokenMismatch_002, TestSize.Level3)
{
    DTEST_LOG << "DmsBundleManagerCallbackStubTest testOnRemoteRequest_TokenMismatch_002 begin" << std::endl;
    ASSERT_NE(stub_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(u"");
    int32_t result = stub_->OnRemoteRequest(CODE_QUERY_DONE, data, reply, option);
    EXPECT_EQ(result, ERR_INVALID_STATE);
    DTEST_LOG << "DmsBundleManagerCallbackStubTest testOnRemoteRequest_TokenMismatch_002 end" << std::endl;
}

/**
 * @tc.name: testOnRemoteRequest_UnknownCode_001
 * @tc.desc: OnRemoteRequest with a valid token but unknown code returns ERR_INVALID_STATE
 * @tc.type: FUNC
 */
HWTEST_F(DmsBundleManagerCallbackStubTest, testOnRemoteRequest_UnknownCode_001, TestSize.Level3)
{
    DTEST_LOG << "DmsBundleManagerCallbackStubTest testOnRemoteRequest_UnknownCode_001 begin" << std::endl;
    ASSERT_NE(stub_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(DmsBundleManagerCallbackStub::GetDescriptor());
    int32_t result = stub_->OnRemoteRequest(CODE_UNKNOWN_HIGH, data, reply, option);
    EXPECT_EQ(result, ERR_INVALID_STATE);
    DTEST_LOG << "DmsBundleManagerCallbackStubTest testOnRemoteRequest_UnknownCode_001 end" << std::endl;
}

/**
 * @tc.name: testOnRemoteRequest_UnknownCode_002
 * @tc.desc: OnRemoteRequest with code 0 (not registered) returns ERR_INVALID_STATE
 * @tc.type: FUNC
 */
HWTEST_F(DmsBundleManagerCallbackStubTest, testOnRemoteRequest_UnknownCode_002, TestSize.Level3)
{
    DTEST_LOG << "DmsBundleManagerCallbackStubTest testOnRemoteRequest_UnknownCode_002 begin" << std::endl;
    ASSERT_NE(stub_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(DmsBundleManagerCallbackStub::GetDescriptor());
    int32_t result = stub_->OnRemoteRequest(CODE_ZERO, data, reply, option);
    EXPECT_EQ(result, ERR_INVALID_STATE);
    DTEST_LOG << "DmsBundleManagerCallbackStubTest testOnRemoteRequest_UnknownCode_002 end" << std::endl;
}

/**
 * @tc.name: testOnRemoteRequest_ValidDispatch_FailureResult_001
 * @tc.desc: OnRemoteRequest dispatches with a non-ERR_OK resultCode without crashing
 * @tc.type: FUNC
 */
HWTEST_F(DmsBundleManagerCallbackStubTest, testOnRemoteRequest_ValidDispatch_FailureResult_001, TestSize.Level3)
{
    DTEST_LOG << "DmsBundleManagerCallbackStubTest "
                 "testOnRemoteRequest_ValidDispatch_FailureResult_001 begin" << std::endl;
    ASSERT_NE(stub_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(DmsBundleManagerCallbackStub::GetDescriptor());
    data.WriteInt32(RESULT_CODE_FAILURE);
    data.WriteUint32(VERSION_CODE);
    data.WriteInt32(MISSION_ID);
    EXPECT_NO_FATAL_FAILURE({
        stub_->OnRemoteRequest(CODE_QUERY_DONE, data, reply, option);
    });
    DTEST_LOG << "DmsBundleManagerCallbackStubTest "
                 "testOnRemoteRequest_ValidDispatch_FailureResult_001 end" << std::endl;
}

/**
 * @tc.name: testOnRemoteRequest_ValidDispatch_SuccessResult_001
 * @tc.desc: OnRemoteRequest dispatches with ERR_OK resultCode without crashing
 * @tc.type: FUNC
 */
HWTEST_F(DmsBundleManagerCallbackStubTest, testOnRemoteRequest_ValidDispatch_SuccessResult_001, TestSize.Level3)
{
    DTEST_LOG << "DmsBundleManagerCallbackStubTest "
                 "testOnRemoteRequest_ValidDispatch_SuccessResult_001 begin" << std::endl;
    ASSERT_NE(stub_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(DmsBundleManagerCallbackStub::GetDescriptor());
    data.WriteInt32(RESULT_CODE_OK);
    data.WriteUint32(VERSION_CODE);
    data.WriteInt32(MISSION_ID);
    EXPECT_NO_FATAL_FAILURE({
        stub_->OnRemoteRequest(CODE_QUERY_DONE, data, reply, option);
    });
    DTEST_LOG << "DmsBundleManagerCallbackStubTest "
                 "testOnRemoteRequest_ValidDispatch_SuccessResult_001 end" << std::endl;
}

/**
 * @tc.name: testOnRemoteRequest_ValidDispatch_NoExtraParcel_001
 * @tc.desc: OnRemoteRequest with valid token/code but missing parcel payload does not crash
 * @tc.type: FUNC
 */
HWTEST_F(DmsBundleManagerCallbackStubTest, testOnRemoteRequest_ValidDispatch_NoExtraParcel_001, TestSize.Level3)
{
    DTEST_LOG << "DmsBundleManagerCallbackStubTest "
                 "testOnRemoteRequest_ValidDispatch_NoExtraParcel_001 begin" << std::endl;
    ASSERT_NE(stub_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(DmsBundleManagerCallbackStub::GetDescriptor());
    EXPECT_NO_FATAL_FAILURE({
        stub_->OnRemoteRequest(CODE_QUERY_DONE, data, reply, option);
    });
    DTEST_LOG << "DmsBundleManagerCallbackStubTest "
                 "testOnRemoteRequest_ValidDispatch_NoExtraParcel_001 end" << std::endl;
}

/**
 * @tc.name: testOnRemoteRequest_DifferentMissionIds_001
 * @tc.desc: OnRemoteRequest accepts varying missionId/versionCode values without crashing
 * @tc.type: FUNC
 */
HWTEST_F(DmsBundleManagerCallbackStubTest, testOnRemoteRequest_DifferentMissionIds_001, TestSize.Level3)
{
    DTEST_LOG << "DmsBundleManagerCallbackStubTest testOnRemoteRequest_DifferentMissionIds_001 begin" << std::endl;
    ASSERT_NE(stub_, nullptr);
    const int32_t missionIds[] = { 0, 1, 100, 2147483647 };
    const uint32_t versionCodes[] = { 0, 1, 100, 4294967295 };
    for (size_t i = 0; i < sizeof(missionIds) / sizeof(missionIds[0]); i++) {
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        data.WriteInterfaceToken(DmsBundleManagerCallbackStub::GetDescriptor());
        data.WriteInt32(RESULT_CODE_FAILURE);
        data.WriteUint32(versionCodes[i]);
        data.WriteInt32(missionIds[i]);
        EXPECT_NO_FATAL_FAILURE({
            stub_->OnRemoteRequest(CODE_QUERY_DONE, data, reply, option);
        });
    }
    DTEST_LOG << "DmsBundleManagerCallbackStubTest testOnRemoteRequest_DifferentMissionIds_001 end" << std::endl;
}

/**
 * @tc.name: testOnRemoteRequest_RepeatedCalls_001
 * @tc.desc: repeated OnRemoteRequest calls on the same stub do not crash
 * @tc.type: FUNC
 */
HWTEST_F(DmsBundleManagerCallbackStubTest, testOnRemoteRequest_RepeatedCalls_001, TestSize.Level3)
{
    DTEST_LOG << "DmsBundleManagerCallbackStubTest testOnRemoteRequest_RepeatedCalls_001 begin" << std::endl;
    ASSERT_NE(stub_, nullptr);
    for (int i = 0; i < 5; i++) {
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        data.WriteInterfaceToken(MOCK_INVALID_DESCRIPTOR);
        int32_t result = stub_->OnRemoteRequest(CODE_QUERY_DONE, data, reply, option);
        EXPECT_EQ(result, ERR_INVALID_STATE);
    }
    DTEST_LOG << "DmsBundleManagerCallbackStubTest testOnRemoteRequest_RepeatedCalls_001 end" << std::endl;
}

/**
 * @tc.name: testOnRemoteRequest_MultipleStubs_001
 * @tc.desc: multiple stubs independently reject an invalid token
 * @tc.type: FUNC
 */
HWTEST_F(DmsBundleManagerCallbackStubTest, testOnRemoteRequest_MultipleStubs_001, TestSize.Level3)
{
    DTEST_LOG << "DmsBundleManagerCallbackStubTest testOnRemoteRequest_MultipleStubs_001 begin" << std::endl;
    sptr<DmsBundleManagerCallbackStub> stubA = new DmsBundleManagerCallbackStub();
    sptr<DmsBundleManagerCallbackStub> stubB = new DmsBundleManagerCallbackStub();
    ASSERT_NE(stubA, nullptr);
    ASSERT_NE(stubB, nullptr);
    MessageParcel dataA;
    MessageParcel replyA;
    MessageOption optionA;
    dataA.WriteInterfaceToken(MOCK_INVALID_DESCRIPTOR);
    EXPECT_EQ(stubA->OnRemoteRequest(CODE_QUERY_DONE, dataA, replyA, optionA), ERR_INVALID_STATE);

    MessageParcel dataB;
    MessageParcel replyB;
    MessageOption optionB;
    dataB.WriteInterfaceToken(MOCK_INVALID_DESCRIPTOR);
    EXPECT_EQ(stubB->OnRemoteRequest(CODE_QUERY_DONE, dataB, replyB, optionB), ERR_INVALID_STATE);
    DTEST_LOG << "DmsBundleManagerCallbackStubTest testOnRemoteRequest_MultipleStubs_001 end" << std::endl;
}
} // namespace DistributedSchedule
} // namespace OHOS
