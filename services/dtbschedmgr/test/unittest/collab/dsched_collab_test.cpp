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

#include "dsched_collab_test.h"

#include "mock_distributed_sched.h"
#include "mock_remote_sup_stub.h"
#include "test_log.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace DistributedSchedule {
namespace {
    const int32_t WAITTIME = 2000;
    constexpr int32_t TEST_CALLER_PID = 1000;
    constexpr int32_t TEST_CALLER_UID = 10000;
    constexpr int32_t TEST_CALLEE_PID = 2000;
    constexpr int32_t TEST_CALLEE_UID = 20000;
    constexpr int32_t DISTRIBUTED_COMPONENT_ADD = 1;
    constexpr int32_t DISTRIBUTED_COMPONENT_REMOVE = 2;
}
void DSchedCollabTest::SetUpTestCase()
{
    DTEST_LOG << "DSchedCollabTest::SetUpTestCase" << std::endl;
}

void DSchedCollabTest::TearDownTestCase()
{
    DTEST_LOG << "DSchedCollabTest::TearDownTestCase" << std::endl;
}

void DSchedCollabTest::TearDown()
{
    DTEST_LOG << "DSchedCollabTest::TearDown" << std::endl;
    MessageParcelMock::messageParcel = nullptr;
    messageParcelMock_ = nullptr;
    DSchedTransportSoftbusAdapterMock::adapterMock = nullptr;
    adapterMock_ = nullptr;
    BundleManagerInternalMock::bundleMgrMock = nullptr;
    bundleMgrMock_ = nullptr;
    DistributedSchedPermMock::dmsPermMock = nullptr;
    dmsPermMock_ = nullptr;
    DistributedSchedServiceMock::dmsSrvMock = nullptr;
    dmsSrvMock_ = nullptr;
    dSchedCollab_ = nullptr;
}

void DSchedCollabTest::SetUp()
{
    DTEST_LOG << "DSchedCollabTest::SetUp" << std::endl;
    messageParcelMock_ = std::make_shared<MessageParcelMock>();
    MessageParcelMock::messageParcel = messageParcelMock_;
    adapterMock_ = std::make_shared<DSchedTransportSoftbusAdapterMock>();
    DSchedTransportSoftbusAdapterMock::adapterMock = adapterMock_;
    bundleMgrMock_ = std::make_shared<BundleManagerInternalMock>();
    BundleManagerInternalMock::bundleMgrMock = bundleMgrMock_;
    dmsPermMock_ = std::make_shared<DistributedSchedPermMock>();
    DistributedSchedPermMock::dmsPermMock = dmsPermMock_;
    dmsSrvMock_ = std::make_shared<DistributedSchedServiceMock>();
    DistributedSchedServiceMock::dmsSrvMock = dmsSrvMock_;
    std::string collabToken;
    DSchedCollabInfo info;
    dSchedCollab_ = std::make_shared<DSchedCollab>(collabToken, info);
}

/**
 * @tc.name: DSchedCollab_001
 * @tc.desc: call DSchedCollab
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(DSchedCollabTest, DSchedCollab_001, TestSize.Level3)
{
    DTEST_LOG << "DSchedCollabTest DSchedCollab_001 begin" << std::endl;
    int32_t softbusSessionId = 0;
    auto getSinkCollabVersionCmd = std::make_shared<GetSinkCollabVersionCmd>();
    auto newCollab = std::make_shared<DSchedCollab>(getSinkCollabVersionCmd, softbusSessionId);
    EXPECT_EQ(newCollab->softbusSessionId_, softbusSessionId);
    DTEST_LOG << "DSchedCollabTest DSchedCollab_001 end" << std::endl;
}

/**
 * @tc.name: DSchedCollab_002
 * @tc.desc: call DSchedCollab
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(DSchedCollabTest, DSchedCollab_002, TestSize.Level3)
{
    DTEST_LOG << "DSchedCollabTest DSchedCollab_002 begin" << std::endl;
    int32_t softbusSessionId = 0;
    auto getSinkCollabVersionCmd = std::make_shared<GetSinkCollabVersionCmd>();
    auto newCollab = std::make_shared<DSchedCollab>(getSinkCollabVersionCmd, softbusSessionId);
    EXPECT_EQ(newCollab->softbusSessionId_, softbusSessionId);
    newCollab->UnInit();
    DTEST_LOG << "DSchedCollabTest DSchedCollab_002 end" << std::endl;
}

/**
 * @tc.name: PostSrcStartTask_001
 * @tc.desc: call PostSrcStartTask
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(DSchedCollabTest, PostSrcStartTask_001, TestSize.Level3)
{
    DTEST_LOG << "DSchedCollabTest PostSrcStartTask_002 begin" << std::endl;
    ASSERT_NE(dSchedCollab_, nullptr);
    ASSERT_EQ(dSchedCollab_->eventHandler_, nullptr);
    EXPECT_EQ(dSchedCollab_->PostSrcStartTask(), INVALID_PARAMETERS_ERR);
    DTEST_LOG << "DSchedCollabTest PostSrcStartTask_001 end" << std::endl;
}

/**
 * @tc.name: PostSinkStartTask_001
 * @tc.desc: call PostSinkStartTask
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(DSchedCollabTest, PostSinkStartTask_001, TestSize.Level3)
{
    DTEST_LOG << "DSchedCollabTest PostSinkStartTask_001 begin" << std::endl;
    ASSERT_NE(dSchedCollab_, nullptr);
    ASSERT_EQ(dSchedCollab_->eventHandler_, nullptr);
    EXPECT_EQ(dSchedCollab_->PostSinkStartTask(""), INVALID_PARAMETERS_ERR);
    DTEST_LOG << "DSchedCollabTest PostSinkStartTask_001 end" << std::endl;
}

/**
 * @tc.name: PostSinkPrepareResultTask_001
 * @tc.desc: call PostSinkPrepareResultTask
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(DSchedCollabTest, PostSinkPrepareResultTask_001, TestSize.Level3)
{
    DTEST_LOG << "DSchedCollabTest PostSinkPrepareResultTask_001 begin" << std::endl;
    ASSERT_NE(dSchedCollab_, nullptr);
    ASSERT_EQ(dSchedCollab_->eventHandler_, nullptr);
    int32_t result = 100;
    DSchedCollabInfo dSchedCollabInfo;
    EXPECT_EQ(dSchedCollab_->PostSinkPrepareResultTask(
        result, dSchedCollabInfo), INVALID_PARAMETERS_ERR);
    DTEST_LOG << "DSchedCollabTest PostSinkPrepareResultTask_001 end" << std::endl;
}

/**
 * @tc.name: PostSrcResultTask_001
 * @tc.desc: call PostSrcResultTask
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(DSchedCollabTest, PostSrcResultTask_001, TestSize.Level3)
{
    DTEST_LOG << "DSchedCollabTest PostSrcResultTask_001 begin" << std::endl;
    ASSERT_NE(dSchedCollab_, nullptr);
    ASSERT_EQ(dSchedCollab_->eventHandler_, nullptr);
    std::shared_ptr<NotifyResultCmd> cmd = std::make_shared<NotifyResultCmd>();
    EXPECT_EQ(dSchedCollab_->PostSrcResultTask(cmd), INVALID_PARAMETERS_ERR);
    DTEST_LOG << "DSchedCollabTest PostSrcResultTask_001 end" << std::endl;
}

/**
 * @tc.name: PostErrEndTask_001
 * @tc.desc: call PostErrEndTask
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(DSchedCollabTest, PostErrEndTask_001, TestSize.Level3)
{
    DTEST_LOG << "DSchedCollabTest PostErrEndTask_001 begin" << std::endl;
    ASSERT_NE(dSchedCollab_, nullptr);
    ASSERT_EQ(dSchedCollab_->eventHandler_, nullptr);
    int32_t result = 100;
    EXPECT_EQ(dSchedCollab_->PostErrEndTask(result), INVALID_PARAMETERS_ERR);
    DTEST_LOG << "DSchedCollabTest PostErrEndTask_001 end" << std::endl;
}

/**
 * @tc.name: PostAbilityRejectTask_001
 * @tc.desc: call PostAbilityRejectTask
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(DSchedCollabTest, PostAbilityRejectTask_001, TestSize.Level3)
{
    DTEST_LOG << "DSchedCollabTest PostAbilityRejectTask_001 begin" << std::endl;
    ASSERT_NE(dSchedCollab_, nullptr);
    ASSERT_EQ(dSchedCollab_->eventHandler_, nullptr);
    std::string reason = "test";
    EXPECT_EQ(dSchedCollab_->PostAbilityRejectTask(reason), INVALID_PARAMETERS_ERR);
    DTEST_LOG << "DSchedCollabTest PostAbilityRejectTask_001 end" << std::endl;
}

/**
 * @tc.name: PostEndTask_001
 * @tc.desc: call PostEndTask
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(DSchedCollabTest, PostEndTask_001, TestSize.Level3)
{
    DTEST_LOG << "DSchedCollabTest PostEndTask_001 begin" << std::endl;
    ASSERT_NE(dSchedCollab_, nullptr);
    ASSERT_EQ(dSchedCollab_->eventHandler_, nullptr);
    EXPECT_EQ(dSchedCollab_->PostEndTask(), INVALID_PARAMETERS_ERR);
    DTEST_LOG << "DSchedCollabTest PostEndTask_001 end" << std::endl;
}

/**
 * @tc.name: ExeSrcClientNotify_001
 * @tc.desc: call ExeSrcClientNotify
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(DSchedCollabTest, ExeSrcClientNotify_001, TestSize.Level3)
{
    DTEST_LOG << "DSchedCollabTest ExeSrcClientNotify_001 begin" << std::endl;
    ASSERT_NE(dSchedCollab_, nullptr);
    int32_t result = 0;
    std::string reason = "test";
    auto ret = dSchedCollab_->ExeSrcClientNotify(result, reason);
    EXPECT_EQ(ret, INVALID_PARAMETERS_ERR);

    dSchedCollab_->collabInfo_.srcClientCB_ = sptr<DistributedSchedService>(new DistributedSchedService());
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(false));
    ret = dSchedCollab_->ExeSrcClientNotify(result, reason);
    EXPECT_EQ(ret, INVALID_PARAMETERS_ERR);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(false));
    ret = dSchedCollab_->ExeSrcClientNotify(result, reason);
    EXPECT_EQ(ret, ERR_FLATTEN_OBJECT);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true)).WillOnce(Return(false));
    ret = dSchedCollab_->ExeSrcClientNotify(result, reason);
    EXPECT_EQ(ret, ERR_FLATTEN_OBJECT);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(false));
    ret = dSchedCollab_->ExeSrcClientNotify(result, reason);
    EXPECT_EQ(ret, ERR_FLATTEN_OBJECT);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true)).WillOnce(Return(false));
    ret = dSchedCollab_->ExeSrcClientNotify(result, reason);
    EXPECT_EQ(ret, ERR_FLATTEN_OBJECT);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true))
        .WillOnce(Return(true)).WillOnce(Return(false));
    ret = dSchedCollab_->ExeSrcClientNotify(result, reason);
    EXPECT_EQ(ret, ERR_FLATTEN_OBJECT);
    DTEST_LOG << "DSchedCollabTest ExeSrcClientNotify_001 end" << std::endl;
}

/**
 * @tc.name: ExeSrcClientNotify_002
 * @tc.desc: call ExeSrcClientNotify
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(DSchedCollabTest, ExeSrcClientNotify_002, TestSize.Level3)
{
    DTEST_LOG << "DSchedCollabTest ExeSrcClientNotify_002 begin" << std::endl;
    ASSERT_NE(dSchedCollab_, nullptr);
    int32_t result = 0;
    std::string reason = "test";
    auto mock_ = sptr<MockRemoteSupStub>(new MockRemoteSupStub());
    dSchedCollab_->collabInfo_.srcClientCB_ = mock_;
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true))
        .WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(ERR_OK));
    EXPECT_EQ(dSchedCollab_->ExeSrcClientNotify(result, reason), ERR_OK);
    DTEST_LOG << "DSchedCollabTest ExeSrcClientNotify_002 end" << std::endl;
}

/**
 * @tc.name: ExeSrcClientNotify_003
 * @tc.desc: call ExeSrcClientNotify
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(DSchedCollabTest, ExeSrcClientNotify_003, TestSize.Level3)
{
    DTEST_LOG << "DSchedCollabTest ExeSrcClientNotify_003 begin" << std::endl;
    ASSERT_NE(dSchedCollab_, nullptr);
    int32_t result = 0;
    std::string reason = "test";
    auto mock_ = sptr<MockRemoteSupStub>(new MockRemoteSupStub());
    dSchedCollab_->collabInfo_.srcClientCB_ = mock_;
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true))
        .WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(-1));
    EXPECT_EQ(dSchedCollab_->ExeSrcClientNotify(result, reason), -1);
    DTEST_LOG << "DSchedCollabTest ExeSrcClientNotify_003 end" << std::endl;
}

/**
 * @tc.name: ExeClientDisconnectNotify_001
 * @tc.desc: call ExeClientDisconnectNotify
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(DSchedCollabTest, ExeClientDisconnectNotify_001, TestSize.Level3)
{
    DTEST_LOG << "DSchedCollabTest ExeClientDisconnectNotify_001 begin" << std::endl;
    ASSERT_NE(dSchedCollab_, nullptr);
    dSchedCollab_->collabInfo_.direction_ = COLLAB_SOURCE;
    dSchedCollab_->collabInfo_.srcClientCB_ = nullptr;
    auto ret = dSchedCollab_->ExeClientDisconnectNotify();
    EXPECT_EQ(ret, ERR_NULL_OBJECT);

    auto mock_ = sptr<MockRemoteSupStub>(new MockRemoteSupStub());
    dSchedCollab_->collabInfo_.srcClientCB_ = mock_;
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(false));
    EXPECT_EQ(dSchedCollab_->ExeClientDisconnectNotify(), SEND_REQUEST_DEF_FAIL);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(false));
    EXPECT_EQ(dSchedCollab_->ExeClientDisconnectNotify(), ERR_FLATTEN_OBJECT);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(-1));
    EXPECT_EQ(dSchedCollab_->ExeClientDisconnectNotify(), SEND_REQUEST_DEF_FAIL);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(ERR_OK));
    EXPECT_EQ(dSchedCollab_->ExeClientDisconnectNotify(), ERR_OK);
    DTEST_LOG << "DSchedCollabTest ExeClientDisconnectNotify_001 end" << std::endl;
}

/**
 * @tc.name: ExeClientDisconnectNotify_002
 * @tc.desc: call ExeClientDisconnectNotify
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(DSchedCollabTest, ExeClientDisconnectNotify_002, TestSize.Level3)
{
    DTEST_LOG << "DSchedCollabTest ExeClientDisconnectNotify_002 begin" << std::endl;
    ASSERT_NE(dSchedCollab_, nullptr);
    dSchedCollab_->collabInfo_.direction_ = COLLAB_SINK;
    dSchedCollab_->collabInfo_.sinkClientCB_ = nullptr;
    auto ret = dSchedCollab_->ExeClientDisconnectNotify();
    EXPECT_EQ(ret, ERR_NULL_OBJECT);

    auto mock_ = sptr<MockRemoteSupStub>(new MockRemoteSupStub());
    dSchedCollab_->collabInfo_.sinkClientCB_ = mock_;
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(false));
    EXPECT_EQ(dSchedCollab_->ExeClientDisconnectNotify(), SEND_REQUEST_DEF_FAIL);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(false));
    EXPECT_EQ(dSchedCollab_->ExeClientDisconnectNotify(), ERR_FLATTEN_OBJECT);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(-1));
    EXPECT_EQ(dSchedCollab_->ExeClientDisconnectNotify(), SEND_REQUEST_DEF_FAIL);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(ERR_OK));
    EXPECT_EQ(dSchedCollab_->ExeClientDisconnectNotify(), ERR_OK);
    DTEST_LOG << "DSchedCollabTest ExeClientDisconnectNotify_002 end" << std::endl;
}

/**
 * @tc.name: NotifyWifiOpen_001
 * @tc.desc: call ExeClientDisconnectNotify
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(DSchedCollabTest, NotifyWifiOpen_001, TestSize.Level3)
{
    DTEST_LOG << "DSchedCollabTest NotifyWifiOpen_001 begin" << std::endl;
    ASSERT_NE(dSchedCollab_, nullptr);
    dSchedCollab_->collabInfo_.direction_ = COLLAB_SOURCE;
    dSchedCollab_->collabInfo_.srcClientCB_ = nullptr;
    auto ret = dSchedCollab_->NotifyWifiOpen();
    EXPECT_EQ(ret, ERR_NULL_OBJECT);

    auto mock_ = sptr<MockRemoteSupStub>(new MockRemoteSupStub());
    dSchedCollab_->collabInfo_.srcClientCB_ = mock_;
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(false));
    EXPECT_EQ(dSchedCollab_->NotifyWifiOpen(), SEND_REQUEST_DEF_FAIL);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(false));
    EXPECT_EQ(dSchedCollab_->NotifyWifiOpen(), ERR_FLATTEN_OBJECT);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(-1));
    EXPECT_EQ(dSchedCollab_->NotifyWifiOpen(), SEND_REQUEST_DEF_FAIL);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(ERR_OK));
    EXPECT_EQ(dSchedCollab_->NotifyWifiOpen(), ERR_OK);
    DTEST_LOG << "DSchedCollabTest NotifyWifiOpen_001 end" << std::endl;
}

/**
 * @tc.name: NotifyWifiOpen_002
 * @tc.desc: call NotifyWifiOpen
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(DSchedCollabTest, NotifyWifiOpen_002, TestSize.Level3)
{
    DTEST_LOG << "DSchedCollabTest NotifyWifiOpen_002 begin" << std::endl;
    ASSERT_NE(dSchedCollab_, nullptr);
    dSchedCollab_->collabInfo_.direction_ = COLLAB_SINK;
    dSchedCollab_->collabInfo_.sinkClientCB_ = nullptr;
    auto ret = dSchedCollab_->NotifyWifiOpen();
    EXPECT_EQ(ret, ERR_NULL_OBJECT);

    auto mock_ = sptr<MockRemoteSupStub>(new MockRemoteSupStub());
    dSchedCollab_->collabInfo_.sinkClientCB_ = mock_;
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(false));
    EXPECT_EQ(dSchedCollab_->NotifyWifiOpen(), SEND_REQUEST_DEF_FAIL);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(false));
    EXPECT_EQ(dSchedCollab_->NotifyWifiOpen(), ERR_FLATTEN_OBJECT);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(-1));
    EXPECT_EQ(dSchedCollab_->NotifyWifiOpen(), SEND_REQUEST_DEF_FAIL);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(ERR_OK));
    EXPECT_EQ(dSchedCollab_->NotifyWifiOpen(), ERR_OK);
    DTEST_LOG << "DSchedCollabTest NotifyWifiOpen_002 end" << std::endl;
}

/**
 * @tc.name: CleanUpSession_001
 * @tc.desc: call CleanUpSession
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(DSchedCollabTest, CleanUpSession_001, TestSize.Level3)
{
    DTEST_LOG << "DSchedCollabTest CleanUpSession_001 begin" << std::endl;
    ASSERT_NE(dSchedCollab_, nullptr);
    dSchedCollab_->collabInfo_.direction_ = COLLAB_SINK;
    dSchedCollab_->collabInfo_.srcClientCB_ = nullptr;
    dSchedCollab_->collabInfo_.sinkClientCB_ = nullptr;
    EXPECT_NE(dSchedCollab_->CleanUpSession(), ERR_OK);

    dSchedCollab_->collabInfo_.direction_ = COLLAB_SOURCE;
    EXPECT_NE(dSchedCollab_->CleanUpSession(), ERR_OK);
    DTEST_LOG << "DSchedCollabTest CleanUpSession_001 end" << std::endl;
}

/**
 * @tc.name: SendCommand_001
 * @tc.desc: call SendCommand
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(DSchedCollabTest, SendCommand_001, TestSize.Level3)
{
    DTEST_LOG << "DSchedCollabTest SendCommand_001 begin" << std::endl;
    ASSERT_NE(dSchedCollab_, nullptr);
    std::shared_ptr<DisconnectCmd> cmd = nullptr;
    EXPECT_EQ(dSchedCollab_->PackDisconnectCmd(cmd), INVALID_PARAMETERS_ERR);
    EXPECT_EQ(dSchedCollab_->SendCommand(cmd), INVALID_PARAMETERS_ERR);

    cmd = std::make_shared<DisconnectCmd>();
    EXPECT_CALL(*adapterMock_, SendData(_, _, _)).WillOnce(Return(-1));
    EXPECT_EQ(dSchedCollab_->SendCommand(cmd), -1);

    EXPECT_CALL(*adapterMock_, SendData(_, _, _)).WillOnce(Return(0));
    EXPECT_EQ(dSchedCollab_->SendCommand(cmd), ERR_OK);
    DTEST_LOG << "DSchedCollabTest SendCommand_001 end" << std::endl;
}

/**
 * @tc.name: ExeSinkPrepareResult_001
 * @tc.desc: call ExeSinkPrepareResult
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(DSchedCollabTest, ExeSinkPrepareResult_001, TestSize.Level3)
{
    DTEST_LOG << "DSchedCollabTest ExeSinkPrepareResult_001 begin" << std::endl;
    ASSERT_NE(dSchedCollab_, nullptr);
    int32_t result = COLLAB_ABILITY_TIMEOUT_ERR;
    EXPECT_EQ(dSchedCollab_->ExeSinkPrepareResult(result), INVALID_PARAMETERS_ERR);

    result = ERR_OK;
    EXPECT_CALL(*adapterMock_, SendData(_, _, _)).WillOnce(Return(-1));
    EXPECT_EQ(dSchedCollab_->ExeSinkPrepareResult(result), -1);

    EXPECT_CALL(*adapterMock_, SendData(_, _, _)).WillOnce(Return(0));
    EXPECT_EQ(dSchedCollab_->ExeSinkPrepareResult(result), ERR_OK);

    result = COLLAB_ABILITY_REJECT_ERR;
    EXPECT_CALL(*adapterMock_, SendData(_, _, _)).WillOnce(Return(0));
    EXPECT_EQ(dSchedCollab_->ExeSinkPrepareResult(result), ERR_OK);
    DTEST_LOG << "DSchedCollabTest ExeSinkPrepareResult_001 end" << std::endl;
}

/**
 * @tc.name: ExeSrcCollabResult_001
 * @tc.desc: call ExeSrcCollabResult
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(DSchedCollabTest, ExeSrcCollabResult_001, TestSize.Level3)
{
    DTEST_LOG << "DSchedCollabTest ExeSrcCollabResult_001 begin" << std::endl;
    ASSERT_NE(dSchedCollab_, nullptr);
    int32_t result = COLLAB_ABILITY_TIMEOUT_ERR;
    std::string reason;
    EXPECT_EQ(dSchedCollab_->ExeSrcCollabResult(result, reason), INVALID_PARAMETERS_ERR);

    result = ERR_OK;
    dSchedCollab_->collabInfo_.srcClientCB_ = nullptr;
    dSchedCollab_->collabInfo_.sinkClientCB_ = nullptr;
    EXPECT_EQ(dSchedCollab_->ExeSrcCollabResult(result, reason), INVALID_PARAMETERS_ERR);

    result = COLLAB_ABILITY_REJECT_ERR;
    auto mock_ = sptr<MockRemoteSupStub>(new MockRemoteSupStub());
    dSchedCollab_->collabInfo_.srcClientCB_ = mock_;
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true))
        .WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(ERR_OK));
    EXPECT_EQ(dSchedCollab_->ExeSrcCollabResult(result, reason), ERR_OK);

    dSchedCollab_->collabInfo_.srcClientCB_ = nullptr;
    reason = "test";
    EXPECT_NE(dSchedCollab_->ExeSrcCollabResult(result, reason), ERR_OK);
    DTEST_LOG << "DSchedCollabTest ExeSinkPrepareResult_001 end" << std::endl;
}

/**
 * @tc.name: PackStartCmd_001
 * @tc.desc: call PackStartCmd
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(DSchedCollabTest, PackStartCmd_001, TestSize.Level3)
{
    DTEST_LOG << "DSchedCollabTest PackStartCmd_001 begin" << std::endl;
    ASSERT_NE(dSchedCollab_, nullptr);
    std::shared_ptr<SinkStartCmd> cmd = nullptr;
    EXPECT_EQ(dSchedCollab_->PackStartCmd(cmd), INVALID_PARAMETERS_ERR);
    DTEST_LOG << "DSchedCollabTest PackStartCmd_001 end" << std::endl;
}

/**
 * @tc.name: ExeSrcStart_001
 * @tc.desc: call ExeSrcStart
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(DSchedCollabTest, ExeSrcStart_001, TestSize.Level3)
{
    DTEST_LOG << "DSchedCollabTest ExeSrcStart_001 begin" << std::endl;
    ASSERT_NE(dSchedCollab_, nullptr);
    EXPECT_NE(dSchedCollab_->ExeSrcStart(), ERR_OK);
    DTEST_LOG << "DSchedCollabTest ExeSrcStart_001 end" << std::endl;
}

/**
 * @tc.name: ExeStartAbility_001
 * @tc.desc: call ExeStartAbility
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(DSchedCollabTest, ExeStartAbility_001, TestSize.Level3)
{
    DTEST_LOG << "DSchedCollabTest ExeStartAbility_001 begin" << std::endl;
    ASSERT_NE(dSchedCollab_, nullptr);
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*dmsSrvMock_, CheckCollabStartPermission(_, _, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    EXPECT_EQ(dSchedCollab_->ExeStartAbility(""), INVALID_PARAMETERS_ERR);

    EXPECT_CALL(*dmsSrvMock_, CheckCollabStartPermission(_, _, _, _)).WillOnce(Return(ERR_OK));
    EXPECT_NE(dSchedCollab_->ExeStartAbility(""), ERR_OK);
    DTEST_LOG << "DSchedCollabTest ExeStartAbility_001 end" << std::endl;
}

/**
 * @tc.name: ExeSrcGetPeerVersion_001
 * @tc.desc: call ExeSrcGetPeerVersion
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(DSchedCollabTest, ExeSrcGetPeerVersion_001, TestSize.Level3)
{
    DTEST_LOG << "DSchedCollabTest ExeSrcGetPeerVersion_001 begin" << std::endl;
    ASSERT_NE(dSchedCollab_, nullptr);
    EXPECT_CALL(*adapterMock_, ConnectDevice(_, _, _)).WillOnce(Return(INVALID_PARAMETERS_ERR));
    EXPECT_EQ(dSchedCollab_->ExeSrcGetPeerVersion(), INVALID_PARAMETERS_ERR);

    EXPECT_CALL(*adapterMock_, ConnectDevice(_, _, _)).WillOnce(Return(ERR_OK));
    EXPECT_CALL(*adapterMock_, SendData(_, _, _)).WillOnce(Return(-1));
    EXPECT_EQ(dSchedCollab_->ExeSrcGetPeerVersion(), -1);

    EXPECT_CALL(*adapterMock_, ConnectDevice(_, _, _)).WillOnce(Return(ERR_OK));
    EXPECT_CALL(*adapterMock_, SendData(_, _, _)).WillOnce(Return(0));
    EXPECT_EQ(dSchedCollab_->ExeSrcGetPeerVersion(), ERR_OK);
    DTEST_LOG << "DSchedCollabTest ExeSrcGetPeerVersion_001 end" << std::endl;
}

/**
 * @tc.name: PostSinkGetVersionTask_001
 * @tc.desc: call PostSinkGetVersionTask
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(DSchedCollabTest, PostSinkGetVersionTask_001, TestSize.Level3)
{
    DTEST_LOG << "DSchedCollabTest PostSinkGetVersionTask_001 begin" << std::endl;
    ASSERT_NE(dSchedCollab_, nullptr);
    ASSERT_EQ(dSchedCollab_->eventHandler_, nullptr);
    EXPECT_EQ(dSchedCollab_->PostSinkGetVersionTask(), INVALID_PARAMETERS_ERR);
    DTEST_LOG << "DSchedCollabTest PostSinkGetVersionTask_001 end" << std::endl;
}

/**
 * @tc.name: PostSrcGetVersionTask_001
 * @tc.desc: call PostSrcGetVersionTask
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(DSchedCollabTest, PostSrcGetVersionTask_001, TestSize.Level3)
{
    DTEST_LOG << "DSchedCollabTest PostSrcGetVersionTask_001 begin" << std::endl;
    ASSERT_NE(dSchedCollab_, nullptr);
    ASSERT_EQ(dSchedCollab_->eventHandler_, nullptr);
    EXPECT_EQ(dSchedCollab_->PostSrcGetVersionTask(), INVALID_PARAMETERS_ERR);
    DTEST_LOG << "DSchedCollabTest PostSrcGetVersionTask_001 end" << std::endl;
}

/**
 * @tc.name: PostSrcGetPeerVersionTask_001
 * @tc.desc: call PostSrcGetPeerVersionTask
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(DSchedCollabTest, PostSrcGetPeerVersionTask_001, TestSize.Level3)
{
    DTEST_LOG << "DSchedCollabTest PostSrcGetPeerVersionTask_001 begin" << std::endl;
    ASSERT_NE(dSchedCollab_, nullptr);
    ASSERT_EQ(dSchedCollab_->eventHandler_, nullptr);
    EXPECT_EQ(dSchedCollab_->PostSrcGetPeerVersionTask(), INVALID_PARAMETERS_ERR);
    DTEST_LOG << "DSchedCollabTest PostSrcGetPeerVersionTask_001 end" << std::endl;
}

/**
 * @tc.name: PackGetPeerVersionCmd_001
 * @tc.desc: call PackGetPeerVersionCmd
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(DSchedCollabTest, PackGetPeerVersionCmd_001, TestSize.Level3)
{
    DTEST_LOG << "DSchedCollabTest PackGetPeerVersionCmd_001 begin" << std::endl;
    ASSERT_NE(dSchedCollab_, nullptr);
    ASSERT_EQ(dSchedCollab_->eventHandler_, nullptr);
    std::shared_ptr<GetSinkCollabVersionCmd> cmd = nullptr;
    EXPECT_EQ(dSchedCollab_->PackGetPeerVersionCmd(cmd), INVALID_PARAMETERS_ERR);
    DTEST_LOG << "DSchedCollabTest PackGetPeerVersionCmd_001 end" << std::endl;
}

/**
 * @tc.name: PackSinkCollabVersionCmd_001
 * @tc.desc: call PackSinkCollabVersionCmd
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(DSchedCollabTest, PackSinkCollabVersionCmd_001, TestSize.Level3)
{
    DTEST_LOG << "DSchedCollabTest PackSinkCollabVersionCmd_001 begin" << std::endl;
    ASSERT_NE(dSchedCollab_, nullptr);
    ASSERT_EQ(dSchedCollab_->eventHandler_, nullptr);
    std::shared_ptr<GetSinkCollabVersionCmd> cmd = nullptr;
    EXPECT_EQ(dSchedCollab_->PackSinkCollabVersionCmd(cmd), INVALID_PARAMETERS_ERR);
    DTEST_LOG << "DSchedCollabTest PackSinkCollabVersionCmd_001 end" << std::endl;
}

/**
 * @tc.name: ExeStartAbility_002
 * @tc.desc: call ExeStartAbility
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(DSchedCollabTest, ExeStartAbility_002, TestSize.Level3)
{
    DTEST_LOG << "DSchedCollabTest ExeStartAbility_002 begin" << std::endl;
    ASSERT_NE(dSchedCollab_, nullptr);
    dSchedCollab_->collabInfo_.callerInfo_.sourceDeviceId = "sourceDeviceId";
    EXPECT_EQ(dSchedCollab_->ExeStartAbility(""), INVALID_PARAMETERS_ERR);
    DTEST_LOG << "DSchedCollabTest ExeStartAbility_002 end" << std::endl;
}

/**
 * @tc.name: GetCollabInfo_001
 * @tc.desc: verify GetCollabInfo returns a consistent snapshot of collabInfo_ fields
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(DSchedCollabTest, GetCollabInfo_001, TestSize.Level3)
{
    DTEST_LOG << "DSchedCollabTest GetCollabInfo_001 begin" << std::endl;
    ASSERT_NE(dSchedCollab_, nullptr);
    dSchedCollab_->collabInfo_.collabToken_ = "token_snapshot";
    dSchedCollab_->collabInfo_.srcInfo_.pid_ = TEST_CALLER_PID;
    dSchedCollab_->collabInfo_.srcInfo_.bundleName_ = "snapshot.src.bundle";
    dSchedCollab_->collabInfo_.sinkInfo_.pid_ = TEST_CALLEE_PID;
    dSchedCollab_->collabInfo_.sinkInfo_.accessToken_ = TEST_CALLEE_UID;
    DSchedCollabInfo collabInfo = dSchedCollab_->GetCollabInfo();
    EXPECT_EQ(collabInfo.collabToken_, "token_snapshot");
    EXPECT_EQ(collabInfo.srcInfo_.pid_, TEST_CALLER_PID);
    EXPECT_EQ(collabInfo.srcInfo_.bundleName_, "snapshot.src.bundle");
    EXPECT_EQ(collabInfo.sinkInfo_.pid_, TEST_CALLEE_PID);
    EXPECT_EQ(collabInfo.sinkInfo_.accessToken_, TEST_CALLEE_UID);
    EXPECT_EQ(dSchedCollab_->collabInfo_.collabToken_, "token_snapshot");
    DTEST_LOG << "DSchedCollabTest GetCollabInfo_001 end" << std::endl;
}

/**
 * @tc.name: GetCollabInfo_002
 * @tc.desc: verify GetCollabInfo on a newly created object returns default values
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(DSchedCollabTest, GetCollabInfo_002, TestSize.Level3)
{
    DTEST_LOG << "DSchedCollabTest GetCollabInfo_002 begin" << std::endl;
    ASSERT_NE(dSchedCollab_, nullptr);
    DSchedCollabInfo collabInfo = dSchedCollab_->GetCollabInfo();
    EXPECT_TRUE(collabInfo.collabToken_.empty());
    EXPECT_EQ(collabInfo.srcCollabSessionId_, -1);
    EXPECT_EQ(collabInfo.sinkInfo_.pid_, -1);
    EXPECT_EQ(collabInfo.sinkInfo_.uid_, -1);
    DTEST_LOG << "DSchedCollabTest GetCollabInfo_002 end" << std::endl;
}

/**
 * @tc.name: SetSrcCollabInfo_001
 * @tc.desc: verify SetSrcCollabInfo updates collabInfo_ and preserves srcClientCB_
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(DSchedCollabTest, SetSrcCollabInfo_001, TestSize.Level3)
{
    DTEST_LOG << "DSchedCollabTest SetSrcCollabInfo_001 begin" << std::endl;
    ASSERT_NE(dSchedCollab_, nullptr);
    dSchedCollab_->collabInfo_.srcClientCB_ = sptr<DistributedSchedService>(new DistributedSchedService());
    ASSERT_NE(dSchedCollab_->collabInfo_.srcClientCB_, nullptr);
    DSchedCollabInfo info;
    info.collabToken_ = "set.src.token";
    info.srcInfo_.pid_ = TEST_CALLER_PID;
    info.srcInfo_.bundleName_ = "set.src.bundle";
    info.srcClientCB_ = nullptr;
    dSchedCollab_->SetSrcCollabInfo(info);
    EXPECT_EQ(dSchedCollab_->collabInfo_.collabToken_, "set.src.token");
    EXPECT_EQ(dSchedCollab_->collabInfo_.srcInfo_.pid_, TEST_CALLER_PID);
    EXPECT_EQ(dSchedCollab_->collabInfo_.srcInfo_.bundleName_, "set.src.bundle");
    EXPECT_NE(dSchedCollab_->collabInfo_.srcClientCB_, nullptr);
    DTEST_LOG << "DSchedCollabTest SetSrcCollabInfo_001 end" << std::endl;
}

/**
 * @tc.name: SetSinkCollabInfo_001
 * @tc.desc: verify SetSinkCollabInfo with null cmd keeps collabInfo_ unchanged
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(DSchedCollabTest, SetSinkCollabInfo_001, TestSize.Level3)
{
    DTEST_LOG << "DSchedCollabTest SetSinkCollabInfo_001 begin" << std::endl;
    ASSERT_NE(dSchedCollab_, nullptr);
    dSchedCollab_->collabInfo_.srcInfo_.pid_ = TEST_CALLER_PID;
    dSchedCollab_->SetSinkCollabInfo(nullptr);
    EXPECT_EQ(dSchedCollab_->collabInfo_.srcInfo_.pid_, TEST_CALLER_PID);
    DTEST_LOG << "DSchedCollabTest SetSinkCollabInfo_001 end" << std::endl;
}

/**
 * @tc.name: SetSinkCollabInfo_002
 * @tc.desc: verify SetSinkCollabInfo copies SinkStartCmd fields into collabInfo_
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(DSchedCollabTest, SetSinkCollabInfo_002, TestSize.Level3)
{
    DTEST_LOG << "DSchedCollabTest SetSinkCollabInfo_002 begin" << std::endl;
    ASSERT_NE(dSchedCollab_, nullptr);
    auto startCmd = std::make_shared<SinkStartCmd>();
    ASSERT_NE(startCmd, nullptr);
    startCmd->srcPid_ = TEST_CALLER_PID;
    startCmd->srcUid_ = TEST_CALLER_UID;
    startCmd->srcDeviceId_ = "sink.src.device";
    startCmd->srcBundleName_ = "sink.src.bundle";
    startCmd->sinkDeviceId_ = "sink.peer.device";
    startCmd->sinkBundleName_ = "sink.peer.bundle";
    startCmd->needSendBigData_ = true;
    dSchedCollab_->SetSinkCollabInfo(startCmd);
    EXPECT_EQ(dSchedCollab_->collabInfo_.srcInfo_.pid_, TEST_CALLER_PID);
    EXPECT_EQ(dSchedCollab_->collabInfo_.srcInfo_.uid_, TEST_CALLER_UID);
    EXPECT_EQ(dSchedCollab_->collabInfo_.srcInfo_.deviceId_, "sink.src.device");
    EXPECT_EQ(dSchedCollab_->collabInfo_.srcInfo_.bundleName_, "sink.src.bundle");
    EXPECT_EQ(dSchedCollab_->collabInfo_.sinkInfo_.deviceId_, "sink.peer.device");
    EXPECT_EQ(dSchedCollab_->collabInfo_.sinkInfo_.bundleName_, "sink.peer.bundle");
    EXPECT_TRUE(dSchedCollab_->collabInfo_.srcOpt_.needSendBigData_);
    DTEST_LOG << "DSchedCollabTest SetSinkCollabInfo_002 end" << std::endl;
}

/**
 * @tc.name: SaveSinkAbilityData_001
 * @tc.desc: verify SaveSinkAbilityData writes pid/uid/accessToken into collabInfo_
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(DSchedCollabTest, SaveSinkAbilityData_001, TestSize.Level3)
{
    DTEST_LOG << "DSchedCollabTest SaveSinkAbilityData_001 begin" << std::endl;
    ASSERT_NE(dSchedCollab_, nullptr);
    dSchedCollab_->collabInfo_.sinkInfo_.pid_ = -1;
    dSchedCollab_->collabInfo_.sinkInfo_.uid_ = -1;
    dSchedCollab_->collabInfo_.sinkInfo_.accessToken_ = -1;
    const std::string collabToken = "save.sink.token";
    int32_t result = 0;
    EXPECT_EQ(dSchedCollab_->SaveSinkAbilityData(collabToken, result, TEST_CALLEE_PID,
        TEST_CALLEE_UID, TEST_CALLEE_UID + 1), ERR_OK);
    EXPECT_EQ(dSchedCollab_->collabInfo_.sinkInfo_.pid_, TEST_CALLEE_PID);
    EXPECT_EQ(dSchedCollab_->collabInfo_.sinkInfo_.uid_, TEST_CALLEE_UID);
    EXPECT_EQ(dSchedCollab_->collabInfo_.sinkInfo_.accessToken_, TEST_CALLEE_UID + 1);
    DTEST_LOG << "DSchedCollabTest SaveSinkAbilityData_001 end" << std::endl;
}

/**
 * @tc.name: HandleSourceGetVersionCmd_001
 * @tc.desc: verify HandleSourceGetVersionCmd with invalid json keeps sinkCollabVersion_ unchanged
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(DSchedCollabTest, HandleSourceGetVersionCmd_001, TestSize.Level3)
{
    DTEST_LOG << "DSchedCollabTest HandleSourceGetVersionCmd_001 begin" << std::endl;
    ASSERT_NE(dSchedCollab_, nullptr);
    dSchedCollab_->collabInfo_.sinkCollabVersion_ = 0;
    EXPECT_NO_FATAL_FAILURE(dSchedCollab_->HandleSourceGetVersionCmd(""));
    EXPECT_EQ(dSchedCollab_->collabInfo_.sinkCollabVersion_, 0);
    DTEST_LOG << "DSchedCollabTest HandleSourceGetVersionCmd_001 end" << std::endl;
}

/**
 * @tc.name: HandleSourceGetVersionCmd_002
 * @tc.desc: verify HandleSourceGetVersionCmd writes sinkCollabVersion_ from valid cmd json
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(DSchedCollabTest, HandleSourceGetVersionCmd_002, TestSize.Level3)
{
    DTEST_LOG << "DSchedCollabTest HandleSourceGetVersionCmd_002 begin" << std::endl;
    ASSERT_NE(dSchedCollab_, nullptr);
    auto cmd = std::make_shared<GetSinkCollabVersionCmd>();
    ASSERT_NE(cmd, nullptr);
    cmd->sinkCollabVersion_ = 5;
    std::string jsonStr;
    ASSERT_EQ(cmd->Marshal(jsonStr), ERR_OK);
    dSchedCollab_->collabInfo_.sinkCollabVersion_ = 0;
    EXPECT_NO_FATAL_FAILURE(dSchedCollab_->HandleSourceGetVersionCmd(jsonStr));
    EXPECT_EQ(dSchedCollab_->collabInfo_.sinkCollabVersion_, 5);
    DTEST_LOG << "DSchedCollabTest HandleSourceGetVersionCmd_002 end" << std::endl;
}

/**
 * @tc.name: PackNotifyResultCmd_002
 * @tc.desc: verify PackNotifyResultCmd packs fields from the collabInfo_ snapshot
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(DSchedCollabTest, PackNotifyResultCmd_002, TestSize.Level3)
{
    DTEST_LOG << "DSchedCollabTest PackNotifyResultCmd_002 begin" << std::endl;
    ASSERT_NE(dSchedCollab_, nullptr);
    dSchedCollab_->collabInfo_.collabToken_ = "pack.notify.token";
    dSchedCollab_->collabInfo_.sinkCollabSessionId_ = 3;
    dSchedCollab_->collabInfo_.sinkInfo_.pid_ = TEST_CALLEE_PID;
    dSchedCollab_->collabInfo_.sinkInfo_.accessToken_ = TEST_CALLEE_UID;
    dSchedCollab_->collabInfo_.sinkInfo_.socketName_ = "pack.notify.socket";
    dSchedCollab_->collabInfo_.sinkUserId_ = 100;
    dSchedCollab_->collabInfo_.sinkAccountId_ = 200;
    auto cmd = std::make_shared<NotifyResultCmd>();
    ASSERT_NE(cmd, nullptr);
    int32_t result = 0;
    EXPECT_EQ(dSchedCollab_->PackNotifyResultCmd(cmd, result), ERR_OK);
    EXPECT_EQ(cmd->command_, NOTIFY_RESULT_CMD);
    EXPECT_EQ(cmd->collabToken_, "pack.notify.token");
    EXPECT_EQ(cmd->sinkCollabSessionId_, 3);
    EXPECT_EQ(cmd->sinkPid_, TEST_CALLEE_PID);
    EXPECT_EQ(cmd->sinkAccessToken_, TEST_CALLEE_UID);
    EXPECT_EQ(cmd->sinkSocketName_, "pack.notify.socket");
    EXPECT_EQ(cmd->sinkUserId_, 100);
    EXPECT_EQ(cmd->sinkAccountId_, 200);
    DTEST_LOG << "DSchedCollabTest PackNotifyResultCmd_002 end" << std::endl;
}

#ifdef EFFICIENCY_MANAGER_ENABLE
/**
 * @tc.name: ReportDistributedComponentChange_Caller_Add_001
 * @tc.desc: test ReportDistributedComponentChange for Caller ADD event
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(DSchedCollabTest, ReportDistributedComponentChange_Caller_Add_001, TestSize.Level3)
{
    DTEST_LOG << "DSchedCollabTest ReportDistributedComponentChange_Caller_Add_001 begin" << std::endl;
    ASSERT_NE(dSchedCollab_, nullptr);

    dSchedCollab_->collabInfo_.srcInfo_.pid_ = TEST_CALLER_PID;
    dSchedCollab_->collabInfo_.srcInfo_.uid_ = TEST_CALLER_UID;
    dSchedCollab_->collabInfo_.srcInfo_.bundleName_ = "test.bundle";
    dSchedCollab_->collabInfo_.direction_ = COLLAB_SOURCE;

    dSchedCollab_->ReportDistributedComponentChange(DISTRIBUTED_COMPONENT_ADD, IDistributedSched::CALLER);
    DTEST_LOG << "DSchedCollabTest ReportDistributedComponentChange_Caller_Add_001 end" << std::endl;
}

/**
 * @tc.name: ReportDistributedComponentChange_Callee_Add_001
 * @tc.desc: test ReportDistributedComponentChange for Callee ADD event
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(DSchedCollabTest, ReportDistributedComponentChange_Callee_Add_001, TestSize.Level3)
{
    DTEST_LOG << "DSchedCollabTest ReportDistributedComponentChange_Callee_Add_001 begin" << std::endl;
    ASSERT_NE(dSchedCollab_, nullptr);

    dSchedCollab_->collabInfo_.sinkInfo_.pid_ = TEST_CALLEE_PID;
    dSchedCollab_->collabInfo_.sinkInfo_.uid_ = TEST_CALLEE_UID;
    dSchedCollab_->collabInfo_.sinkInfo_.bundleName_ = "test.bundle.sink";
    dSchedCollab_->collabInfo_.direction_ = COLLAB_SINK;

    dSchedCollab_->ReportDistributedComponentChange(DISTRIBUTED_COMPONENT_ADD, IDistributedSched::CALLEE);
    DTEST_LOG << "DSchedCollabTest ReportDistributedComponentChange_Callee_Add_001 end" << std::endl;
}

/**
 * @tc.name: ReportDistributedComponentChange_Remove_001
 * @tc.desc: test ReportDistributedComponentChange for REMOVE event
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(DSchedCollabTest, ReportDistributedComponentChange_Remove_001, TestSize.Level3)
{
    DTEST_LOG << "DSchedCollabTest ReportDistributedComponentChange_Remove_001 begin" << std::endl;
    ASSERT_NE(dSchedCollab_, nullptr);

    dSchedCollab_->collabInfo_.srcInfo_.pid_ = TEST_CALLER_PID;
    dSchedCollab_->collabInfo_.srcInfo_.uid_ = TEST_CALLER_UID;
    dSchedCollab_->collabInfo_.srcInfo_.bundleName_ = "test.bundle";
    dSchedCollab_->collabInfo_.direction_ = COLLAB_SOURCE;

    dSchedCollab_->ReportDistributedComponentChange(DISTRIBUTED_COMPONENT_REMOVE, IDistributedSched::CALLER);
    DTEST_LOG << "DSchedCollabTest ReportDistributedComponentChange_Remove_001 end" << std::endl;
}

/**
 * @tc.name: ReportDistributedComponentChange_Invalid_Info_001
 * @tc.desc: test ReportDistributedComponentChange with invalid info
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(DSchedCollabTest, ReportDistributedComponentChange_Invalid_Info_001, TestSize.Level3)
{
    DTEST_LOG << "DSchedCollabTest ReportDistributedComponentChange_Invalid_Info_001 begin" << std::endl;
    ASSERT_NE(dSchedCollab_, nullptr);

    dSchedCollab_->collabInfo_.srcInfo_.uid_ = -1;
    dSchedCollab_->collabInfo_.srcInfo_.bundleName_ = "";

    dSchedCollab_->ReportDistributedComponentChange(DISTRIBUTED_COMPONENT_ADD, IDistributedSched::CALLER);
    DTEST_LOG << "DSchedCollabTest ReportDistributedComponentChange_Invalid_Info_001 end" << std::endl;
}
#endif
}
}