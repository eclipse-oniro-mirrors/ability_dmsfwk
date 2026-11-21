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

#include "notifier_info_test.h"

#include "test_log.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace DistributedSchedule {
namespace {
const std::string CB_TYPE_CONNECT = "deviceSelected";
const std::string CB_TYPE_DISCONNECT = "deviceUnselected";
}

void NotifierInfoTest::SetUpTestCase()
{
    DTEST_LOG << "NotifierInfoTest::SetUpTestCase" << std::endl;
}

void NotifierInfoTest::TearDownTestCase()
{
    DTEST_LOG << "NotifierInfoTest::TearDownTestCase" << std::endl;
}

void NotifierInfoTest::SetUp()
{
    DTEST_LOG << "NotifierInfoTest::SetUp" << std::endl;
}

void NotifierInfoTest::TearDown()
{
    DTEST_LOG << "NotifierInfoTest::TearDown" << std::endl;
}

/**
 * @tc.name: NotifierInfoTest_001
 * @tc.desc: empty map IsNotifierMapEmpty returns true
 * @tc.type: FUNC
 */
HWTEST_F(NotifierInfoTest, NotifierInfoTest_001, TestSize.Level3)
{
    DTEST_LOG << "NotifierInfoTest_001 begin" << std::endl;
    NotifierInfo info;
    EXPECT_TRUE(info.IsNotifierMapEmpty());
    EXPECT_EQ(info.GetNotifier(CB_TYPE_CONNECT), nullptr);
    DTEST_LOG << "NotifierInfoTest_001 end" << std::endl;
}

/**
 * @tc.name: NotifierInfoTest_002
 * @tc.desc: SetNotifier/GetNotifier round trip
 * @tc.type: FUNC
 */
HWTEST_F(NotifierInfoTest, NotifierInfoTest_002, TestSize.Level3)
{
    DTEST_LOG << "NotifierInfoTest_002 begin" << std::endl;
    NotifierInfo info;
    sptr<IRemoteObject> notifier = new MockRemoteStub();
    info.SetNotifier(CB_TYPE_CONNECT, notifier);
    EXPECT_FALSE(info.IsNotifierMapEmpty());
    EXPECT_EQ(info.GetNotifier(CB_TYPE_CONNECT), notifier);
    DTEST_LOG << "NotifierInfoTest_002 end" << std::endl;
}

/**
 * @tc.name: NotifierInfoTest_003
 * @tc.desc: GetNotifier returns nullptr for unknown cbType
 * @tc.type: FUNC
 */
HWTEST_F(NotifierInfoTest, NotifierInfoTest_003, TestSize.Level3)
{
    DTEST_LOG << "NotifierInfoTest_003 begin" << std::endl;
    NotifierInfo info;
    sptr<IRemoteObject> notifier = new MockRemoteStub();
    info.SetNotifier(CB_TYPE_CONNECT, notifier);
    EXPECT_EQ(info.GetNotifier(CB_TYPE_DISCONNECT), nullptr);
    DTEST_LOG << "NotifierInfoTest_003 end" << std::endl;
}

/**
 * @tc.name: NotifierInfoTest_004
 * @tc.desc: DeleteNotifier removes entry
 * @tc.type: FUNC
 */
HWTEST_F(NotifierInfoTest, NotifierInfoTest_004, TestSize.Level3)
{
    DTEST_LOG << "NotifierInfoTest_004 begin" << std::endl;
    NotifierInfo info;
    sptr<IRemoteObject> notifier = new MockRemoteStub();
    info.SetNotifier(CB_TYPE_CONNECT, notifier);
    info.DeleteNotifier(CB_TYPE_CONNECT);
    EXPECT_EQ(info.GetNotifier(CB_TYPE_CONNECT), nullptr);
    EXPECT_TRUE(info.IsNotifierMapEmpty());
    DTEST_LOG << "NotifierInfoTest_004 end" << std::endl;
}

/**
 * @tc.name: NotifierInfoTest_005
 * @tc.desc: DeleteNotifier on unknown cbType does nothing
 * @tc.type: FUNC
 */
HWTEST_F(NotifierInfoTest, NotifierInfoTest_005, TestSize.Level3)
{
    DTEST_LOG << "NotifierInfoTest_005 begin" << std::endl;
    NotifierInfo info;
    sptr<IRemoteObject> notifier = new MockRemoteStub();
    info.SetNotifier(CB_TYPE_CONNECT, notifier);
    info.DeleteNotifier(CB_TYPE_DISCONNECT);
    EXPECT_FALSE(info.IsNotifierMapEmpty());
    DTEST_LOG << "NotifierInfoTest_005 end" << std::endl;
}

/**
 * @tc.name: NotifierInfoTest_006
 * @tc.desc: QueryNotifier returns true for existing, false for unknown
 * @tc.type: FUNC
 */
HWTEST_F(NotifierInfoTest, NotifierInfoTest_006, TestSize.Level3)
{
    DTEST_LOG << "NotifierInfoTest_006 begin" << std::endl;
    NotifierInfo info;
    sptr<IRemoteObject> notifier = new MockRemoteStub();
    info.SetNotifier(CB_TYPE_CONNECT, notifier);
    EXPECT_TRUE(info.QueryNotifier(notifier));
    sptr<IRemoteObject> other = new MockRemoteStub();
    EXPECT_FALSE(info.QueryNotifier(other));
    DTEST_LOG << "NotifierInfoTest_006 end" << std::endl;
}

/**
 * @tc.name: NotifierInfoTest_007
 * @tc.desc: QueryNotifier on empty map returns false
 * @tc.type: FUNC
 */
HWTEST_F(NotifierInfoTest, NotifierInfoTest_007, TestSize.Level3)
{
    DTEST_LOG << "NotifierInfoTest_007 begin" << std::endl;
    NotifierInfo info;
    sptr<IRemoteObject> notifier = new MockRemoteStub();
    EXPECT_FALSE(info.QueryNotifier(notifier));
    DTEST_LOG << "NotifierInfoTest_007 end" << std::endl;
}

/**
 * @tc.name: NotifierInfoTest_008
 * @tc.desc: SetConnectStatusInfo/GetConnectStatusInfo round trip
 * @tc.type: FUNC
 */
HWTEST_F(NotifierInfoTest, NotifierInfoTest_008, TestSize.Level3)
{
    DTEST_LOG << "NotifierInfoTest_008 begin" << std::endl;
    NotifierInfo info;
    EXPECT_EQ(info.GetConnectStatusInfo(), nullptr);
    auto statusInfo = std::make_shared<ConnectStatusInfo>("dev", DeviceConnectStatus::CONNECTED);
    info.SetConnectStatusInfo(statusInfo);
    EXPECT_EQ(info.GetConnectStatusInfo(), statusInfo);
    EXPECT_EQ(info.GetConnectStatusInfo()->GetDeviceId(), "dev");
    DTEST_LOG << "NotifierInfoTest_008 end" << std::endl;
}

/**
 * @tc.name: NotifierInfoTest_009
 * @tc.desc: RemoveDeathRecipient with empty cbType removes from all notifiers
 * @tc.type: FUNC
 */
HWTEST_F(NotifierInfoTest, NotifierInfoTest_009, TestSize.Level3)
{
    DTEST_LOG << "NotifierInfoTest_009 begin" << std::endl;
    NotifierInfo info;
    sptr<IRemoteObject> notifier = new MockRemoteStub();
    info.SetNotifier(CB_TYPE_CONNECT, notifier);
    sptr<IRemoteObject::DeathRecipient> recipient = new NotifierDeathRecipient();
    EXPECT_NO_FATAL_FAILURE(info.RemoveDeathRecipient(recipient, ""));
    DTEST_LOG << "NotifierInfoTest_009 end" << std::endl;
}

/**
 * @tc.name: NotifierInfoTest_010
 * @tc.desc: RemoveDeathRecipient with specific cbType removes only that notifier's recipient
 * @tc.type: FUNC
 */
HWTEST_F(NotifierInfoTest, NotifierInfoTest_010, TestSize.Level3)
{
    DTEST_LOG << "NotifierInfoTest_010 begin" << std::endl;
    NotifierInfo info;
    sptr<IRemoteObject> notifier = new MockRemoteStub();
    info.SetNotifier(CB_TYPE_CONNECT, notifier);
    sptr<IRemoteObject::DeathRecipient> recipient = new NotifierDeathRecipient();
    EXPECT_NO_FATAL_FAILURE(info.RemoveDeathRecipient(recipient, CB_TYPE_CONNECT));
    EXPECT_NO_FATAL_FAILURE(info.RemoveDeathRecipient(recipient, CB_TYPE_DISCONNECT));
    DTEST_LOG << "NotifierInfoTest_010 end" << std::endl;
}
} // namespace DistributedSchedule
} // namespace OHOS
