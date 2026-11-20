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

#include "kvstore_death_recipient_test.h"

#include "mission/kvstore_death_recipient.h"
#include "mock_remote_stub.h"
#include "test_log.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace DistributedSchedule {

void KvStoreDeathRecipientTest::SetUpTestCase()
{}

void KvStoreDeathRecipientTest::TearDownTestCase()
{}

void KvStoreDeathRecipientTest::SetUp()
{}

void KvStoreDeathRecipientTest::TearDown()
{}

/**
 * @tc.name: testConstructor_001
 * @tc.desc: test KvStoreDeathRecipient construction and destruction
 * @tc.type: FUNC
 */
HWTEST_F(KvStoreDeathRecipientTest, testConstructor_001, TestSize.Level3)
{
    DTEST_LOG << "KvStoreDeathRecipientTest testConstructor_001 begin" << std::endl;
    auto* recipient = new KvStoreDeathRecipient();
    ASSERT_NE(recipient, nullptr);
    delete recipient;
    DTEST_LOG << "KvStoreDeathRecipientTest testConstructor_001 end" << std::endl;
}

/**
 * @tc.name: testOnRemoteDied_ValidRemote_001
 * @tc.desc: test OnRemoteDied with a valid remote does not crash
 * @tc.type: FUNC
 */
HWTEST_F(KvStoreDeathRecipientTest, testOnRemoteDied_ValidRemote_001, TestSize.Level3)
{
    DTEST_LOG << "KvStoreDeathRecipientTest testOnRemoteDied_ValidRemote_001 begin" << std::endl;
    KvStoreDeathRecipient recipient;
    sptr<IRemoteObject> remote = new MockRemoteStub();
    wptr<IRemoteObject> weakRemote = remote;
    EXPECT_NO_FATAL_FAILURE(recipient.OnRemoteDied(weakRemote));
    DTEST_LOG << "KvStoreDeathRecipientTest testOnRemoteDied_ValidRemote_001 end" << std::endl;
}

/**
 * @tc.name: testOnRemoteDied_Repeated_001
 * @tc.desc: test repeated OnRemoteDied calls with the same remote
 * @tc.type: FUNC
 */
HWTEST_F(KvStoreDeathRecipientTest, testOnRemoteDied_Repeated_001, TestSize.Level3)
{
    DTEST_LOG << "KvStoreDeathRecipientTest testOnRemoteDied_Repeated_001 begin" << std::endl;
    KvStoreDeathRecipient recipient;
    sptr<IRemoteObject> remote = new MockRemoteStub();
    wptr<IRemoteObject> weakRemote = remote;
    for (int i = 0; i < 10; i++) {
        EXPECT_NO_FATAL_FAILURE(recipient.OnRemoteDied(weakRemote));
    }
    DTEST_LOG << "KvStoreDeathRecipientTest testOnRemoteDied_Repeated_001 end" << std::endl;
}

/**
 * @tc.name: testOnRemoteDied_MultipleRemotes_001
 * @tc.desc: test OnRemoteDied with multiple distinct remote objects
 * @tc.type: FUNC
 */
HWTEST_F(KvStoreDeathRecipientTest, testOnRemoteDied_MultipleRemotes_001, TestSize.Level3)
{
    DTEST_LOG << "KvStoreDeathRecipientTest testOnRemoteDied_MultipleRemotes_001 begin" << std::endl;
    KvStoreDeathRecipient recipient;
    sptr<IRemoteObject> remote1 = new MockRemoteStub();
    sptr<IRemoteObject> remote2 = new MockRemoteStub();
    sptr<IRemoteObject> remote3 = new MockRemoteStub();
    EXPECT_NO_FATAL_FAILURE(recipient.OnRemoteDied(wptr<IRemoteObject>(remote1)));
    EXPECT_NO_FATAL_FAILURE(recipient.OnRemoteDied(wptr<IRemoteObject>(remote2)));
    EXPECT_NO_FATAL_FAILURE(recipient.OnRemoteDied(wptr<IRemoteObject>(remote3)));
    DTEST_LOG << "KvStoreDeathRecipientTest testOnRemoteDied_MultipleRemotes_001 end" << std::endl;
}

/**
 * @tc.name: testOnRemoteDied_SharedPtrRecipient_001
 * @tc.desc: test OnRemoteDied on a shared_ptr-managed recipient
 * @tc.type: FUNC
 */
HWTEST_F(KvStoreDeathRecipientTest, testOnRemoteDied_SharedPtrRecipient_001, TestSize.Level3)
{
    DTEST_LOG << "KvStoreDeathRecipientTest testOnRemoteDied_SharedPtrRecipient_001 begin" << std::endl;
    auto recipient = std::make_shared<KvStoreDeathRecipient>();
    ASSERT_NE(recipient, nullptr);
    sptr<IRemoteObject> remote = new MockRemoteStub();
    wptr<IRemoteObject> weakRemote = remote;
    EXPECT_NO_FATAL_FAILURE(recipient->OnRemoteDied(weakRemote));
    DTEST_LOG << "KvStoreDeathRecipientTest testOnRemoteDied_SharedPtrRecipient_001 end" << std::endl;
}

/**
 * @tc.name: testOnRemoteDied_LoopOnShared_001
 * @tc.desc: test OnRemoteDied invoked several times on a shared recipient
 * @tc.type: FUNC
 */
HWTEST_F(KvStoreDeathRecipientTest, testOnRemoteDied_LoopOnShared_001, TestSize.Level3)
{
    DTEST_LOG << "KvStoreDeathRecipientTest testOnRemoteDied_LoopOnShared_001 begin" << std::endl;
    auto recipient = std::make_shared<KvStoreDeathRecipient>();
    sptr<IRemoteObject> remote = new MockRemoteStub();
    wptr<IRemoteObject> weakRemote = remote;
    for (int i = 0; i < 5; i++) {
        EXPECT_NO_FATAL_FAILURE(recipient->OnRemoteDied(weakRemote));
    }
    DTEST_LOG << "KvStoreDeathRecipientTest testOnRemoteDied_LoopOnShared_001 end" << std::endl;
}
} // namespace DistributedSchedule
} // namespace OHOS
