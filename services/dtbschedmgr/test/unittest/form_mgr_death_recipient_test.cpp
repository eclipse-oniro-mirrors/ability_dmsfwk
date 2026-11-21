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

#include "form_mgr_death_recipient_test.h"

#include "form_mgr_death_recipient.h"
#include "mock_remote_stub.h"
#include "test_log.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace DistributedSchedule {

void FormMgrDeathRecipientTest::SetUpTestCase()
{}

void FormMgrDeathRecipientTest::TearDownTestCase()
{}

void FormMgrDeathRecipientTest::SetUp()
{}

void FormMgrDeathRecipientTest::TearDown()
{}

/**
 * @tc.name: testDestructor_001
 * @tc.desc: test FormMgrDeathRecipient destructor does not crash
 * @tc.type: FUNC
 * @tc.require: I7XVTZ
 */
HWTEST_F(FormMgrDeathRecipientTest, testDestructor_001, TestSize.Level3)
{
    DTEST_LOG << "FormMgrDeathRecipientTest testDestructor_001 begin" << std::endl;
    auto* recipient = new FormMgrDeathRecipient();
    ASSERT_NE(recipient, nullptr);
    delete recipient;
    DTEST_LOG << "FormMgrDeathRecipientTest testDestructor_001 end" << std::endl;
}

/**
 * @tc.name: testOnRemoteDied_001
 * @tc.desc: test OnRemoteDied with null remote does not crash
 * @tc.type: FUNC
 * @tc.require: I7XVTZ
 */
HWTEST_F(FormMgrDeathRecipientTest, testOnRemoteDied_001, TestSize.Level3)
{
    DTEST_LOG << "FormMgrDeathRecipientTest testOnRemoteDied_001 begin" << std::endl;
    FormMgrDeathRecipient recipient;
    wptr<IRemoteObject> nullRemote;
    EXPECT_NO_FATAL_FAILURE(recipient.OnRemoteDied(nullRemote));
    DTEST_LOG << "FormMgrDeathRecipientTest testOnRemoteDied_001 end" << std::endl;
}

/**
 * @tc.name: testOnRemoteDied_002
 * @tc.desc: test OnRemoteDied with a valid remote does not crash
 * @tc.type: FUNC
 * @tc.require: I7XVTZ
 */
HWTEST_F(FormMgrDeathRecipientTest, testOnRemoteDied_002, TestSize.Level3)
{
    DTEST_LOG << "FormMgrDeathRecipientTest testOnRemoteDied_002 begin" << std::endl;
    FormMgrDeathRecipient recipient;
    sptr<IRemoteObject> remote = new MockRemoteStub();
    wptr<IRemoteObject> weakRemote = remote;
    EXPECT_NO_FATAL_FAILURE(recipient.OnRemoteDied(weakRemote));
    DTEST_LOG << "FormMgrDeathRecipientTest testOnRemoteDied_002 end" << std::endl;
}

/**
 * @tc.name: testOnRemoteDied_003
 * @tc.desc: test OnRemoteDied with an expired weak pointer does not crash
 * @tc.type: FUNC
 * @tc.require: I7XVTZ
 */
HWTEST_F(FormMgrDeathRecipientTest, testOnRemoteDied_003, TestSize.Level3)
{
    DTEST_LOG << "FormMgrDeathRecipientTest testOnRemoteDied_003 begin" << std::endl;
    FormMgrDeathRecipient recipient;
    wptr<IRemoteObject> expiredWeak;
    {
        sptr<IRemoteObject> temp = new MockRemoteStub();
        expiredWeak = temp;
    }
    EXPECT_NO_FATAL_FAILURE(recipient.OnRemoteDied(expiredWeak));
    DTEST_LOG << "FormMgrDeathRecipientTest testOnRemoteDied_003 end" << std::endl;
}

/**
 * @tc.name: testMultipleOnRemoteDiedCalls_001
 * @tc.desc: test multiple OnRemoteDied calls do not crash
 * @tc.type: FUNC
 * @tc.require: I7XVTZ
 */
HWTEST_F(FormMgrDeathRecipientTest, testMultipleOnRemoteDiedCalls_001, TestSize.Level3)
{
    DTEST_LOG << "FormMgrDeathRecipientTest testMultipleOnRemoteDiedCalls_001 begin" << std::endl;
    FormMgrDeathRecipient recipient;
    sptr<IRemoteObject> remote = new MockRemoteStub();
    wptr<IRemoteObject> weakRemote = remote;
    for (int i = 0; i < 10; i++) {
        EXPECT_NO_FATAL_FAILURE(recipient.OnRemoteDied(weakRemote));
    }
    DTEST_LOG << "FormMgrDeathRecipientTest testMultipleOnRemoteDiedCalls_001 end" << std::endl;
}

/**
 * @tc.name: testOnRemoteDiedWithDifferentRemotes_001
 * @tc.desc: test OnRemoteDied with several different remote objects
 * @tc.type: FUNC
 * @tc.require: I7XVTZ
 */
HWTEST_F(FormMgrDeathRecipientTest, testOnRemoteDiedWithDifferentRemotes_001, TestSize.Level3)
{
    DTEST_LOG << "FormMgrDeathRecipientTest testOnRemoteDiedWithDifferentRemotes_001 begin" << std::endl;
    FormMgrDeathRecipient recipient;
    sptr<IRemoteObject> remote1 = new MockRemoteStub();
    sptr<IRemoteObject> remote2 = new MockRemoteStub();
    sptr<IRemoteObject> remote3 = new MockRemoteStub();
    wptr<IRemoteObject> weakRemote1 = remote1;
    wptr<IRemoteObject> weakRemote2 = remote2;
    wptr<IRemoteObject> weakRemote3 = remote3;
    EXPECT_NO_FATAL_FAILURE(recipient.OnRemoteDied(weakRemote1));
    EXPECT_NO_FATAL_FAILURE(recipient.OnRemoteDied(weakRemote2));
    EXPECT_NO_FATAL_FAILURE(recipient.OnRemoteDied(weakRemote3));
    DTEST_LOG << "FormMgrDeathRecipientTest testOnRemoteDiedWithDifferentRemotes_001 end" << std::endl;
}

/**
 * @tc.name: testSharedPtrBehavior_001
 * @tc.desc: test FormMgrDeathRecipient managed by shared_ptr works correctly
 * @tc.type: FUNC
 * @tc.require: I7XVTZ
 */
HWTEST_F(FormMgrDeathRecipientTest, testSharedPtrBehavior_001, TestSize.Level3)
{
    DTEST_LOG << "FormMgrDeathRecipientTest testSharedPtrBehavior_001 begin" << std::endl;
    auto recipient = std::make_shared<FormMgrDeathRecipient>();
    ASSERT_NE(recipient, nullptr);
    sptr<IRemoteObject> remote = new MockRemoteStub();
    wptr<IRemoteObject> weakRemote = remote;
    EXPECT_NO_FATAL_FAILURE(recipient->OnRemoteDied(weakRemote));
    DTEST_LOG << "FormMgrDeathRecipientTest testSharedPtrBehavior_001 end" << std::endl;
}

/**
 * @tc.name: testNullptrPromote_001
 * @tc.desc: test OnRemoteDied when promote returns nullptr
 * @tc.type: FUNC
 * @tc.require: I7XVTZ
 */
HWTEST_F(FormMgrDeathRecipientTest, testNullptrPromote_001, TestSize.Level3)
{
    DTEST_LOG << "FormMgrDeathRecipientTest testNullptrPromote_001 begin" << std::endl;
    FormMgrDeathRecipient recipient;
    sptr<IRemoteObject> nullRemote;
    wptr<IRemoteObject> weakNull = nullRemote;
    EXPECT_NO_FATAL_FAILURE(recipient.OnRemoteDied(weakNull));
    DTEST_LOG << "FormMgrDeathRecipientTest testNullptrPromote_001 end" << std::endl;
}

/**
 * @tc.name: testConcurrentOnRemoteDied_001
 * @tc.desc: test repeated OnRemoteDied calls on a shared recipient
 * @tc.type: FUNC
 * @tc.require: I7XVTZ
 */
HWTEST_F(FormMgrDeathRecipientTest, testConcurrentOnRemoteDied_001, TestSize.Level3)
{
    DTEST_LOG << "FormMgrDeathRecipientTest testConcurrentOnRemoteDied_001 begin" << std::endl;
    auto recipient = std::make_shared<FormMgrDeathRecipient>();
    sptr<IRemoteObject> remote = new MockRemoteStub();
    wptr<IRemoteObject> weakRemote = remote;
    for (int i = 0; i < 5; i++) {
        EXPECT_NO_FATAL_FAILURE(recipient->OnRemoteDied(weakRemote));
    }
    DTEST_LOG << "FormMgrDeathRecipientTest testConcurrentOnRemoteDied_001 end" << std::endl;
}
} // namespace DistributedSchedule
} // namespace OHOS
