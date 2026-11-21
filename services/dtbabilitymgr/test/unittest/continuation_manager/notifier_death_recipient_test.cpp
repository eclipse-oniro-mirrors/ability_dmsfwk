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

#include "notifier_death_recipient_test.h"

#include "mock_remote_stub.h"
#include "test_log.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace DistributedSchedule {

void NotifierDeathRecipientTest::SetUpTestCase()
{
    DTEST_LOG << "NotifierDeathRecipientTest::SetUpTestCase" << std::endl;
}

void NotifierDeathRecipientTest::TearDownTestCase()
{
    DTEST_LOG << "NotifierDeathRecipientTest::TearDownTestCase" << std::endl;
}

void NotifierDeathRecipientTest::SetUp()
{
    DTEST_LOG << "NotifierDeathRecipientTest::SetUp" << std::endl;
}

void NotifierDeathRecipientTest::TearDown()
{
    DTEST_LOG << "NotifierDeathRecipientTest::TearDown" << std::endl;
}

/**
 * @tc.name: NotifierDeathRecipientTest_001
 * @tc.desc: construct and destruct does not crash
 * @tc.type: FUNC
 */
HWTEST_F(NotifierDeathRecipientTest, NotifierDeathRecipientTest_001, TestSize.Level3)
{
    DTEST_LOG << "NotifierDeathRecipientTest_001 begin" << std::endl;
    auto* recipient = new NotifierDeathRecipient();
    ASSERT_NE(recipient, nullptr);
    delete recipient;
    DTEST_LOG << "NotifierDeathRecipientTest_001 end" << std::endl;
}

/**
 * @tc.name: NotifierDeathRecipientTest_002
 * @tc.desc: OnRemoteDied with null remote (promote returns null) does not crash
 * @tc.type: FUNC
 */
HWTEST_F(NotifierDeathRecipientTest, NotifierDeathRecipientTest_002, TestSize.Level3)
{
    DTEST_LOG << "NotifierDeathRecipientTest_002 begin" << std::endl;
    NotifierDeathRecipient recipient;
    wptr<IRemoteObject> nullRemote;
    EXPECT_NO_FATAL_FAILURE(recipient.OnRemoteDied(nullRemote));
    DTEST_LOG << "NotifierDeathRecipientTest_002 end" << std::endl;
}

/**
 * @tc.name: NotifierDeathRecipientTest_003
 * @tc.desc: OnRemoteDied with a valid remote object does not crash
 * @tc.type: FUNC
 */
HWTEST_F(NotifierDeathRecipientTest, NotifierDeathRecipientTest_003, TestSize.Level3)
{
    DTEST_LOG << "NotifierDeathRecipientTest_003 begin" << std::endl;
    NotifierDeathRecipient recipient;
    sptr<IRemoteObject> remote = new MockRemoteStub();
    wptr<IRemoteObject> weakRemote = remote;
    EXPECT_NO_FATAL_FAILURE(recipient.OnRemoteDied(weakRemote));
    DTEST_LOG << "NotifierDeathRecipientTest_003 end" << std::endl;
}
} // namespace DistributedSchedule
} // namespace OHOS
