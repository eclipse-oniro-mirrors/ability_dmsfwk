/*
* Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "channel_manager_test.h"
#include "channel_manager.h"
#include "dtbcollabmgr_log.h"

namespace OHOS {
namespace DistributedCollab {
namespace {
    static const std::string TAG = "DSchedCollabChannelManagerTest";
    using namespace testing;
    using namespace testing::ext;
}

void ChannelManagerTest::SetUpTestCase()
{
    HILOGI("ChannelManagerTest::SetUpTestCase");
}

void ChannelManagerTest::TearDownTestCase()
{
    HILOGI("ChannelManagerTest::TearDownTestCase");
}

void ChannelManagerTest::SetUp()
{
    HILOGI("ChannelManagerTest::SetUp");
}

void ChannelManagerTest::TearDown()
{
    HILOGI("ChannelManagerTest::TearDown");
    ChannelManager::GetInstance().DeInit();
}

/**
 * @tc.name: TestInit_Success
 * @tc.desc: Test for Init when all conditions are successful
 * @tc.type: FUNC
 */
HWTEST_F(ChannelManagerTest, TestInit_Success, TestSize.Level1)
{
    EXPECT_CALL(mockSoftbus, Socket(testing::_))
        .WillOnce(testing::Return(1234));

    EXPECT_CALL(mockSoftbus, Listen(testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(ERR_OK));

    int32_t result = ChannelManager::GetInstance().Init(ownerName);

    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: TestInit_SocketFailure
 * @tc.desc: Test for Init when CreateServerSocket fails
 * @tc.type: FUNC
 */
HWTEST_F(ChannelManagerTest, TestInit_SocketFailure, TestSize.Level1)
{
    EXPECT_CALL(mockSoftbus, Socket(testing::_))
        .WillOnce(testing::Return(-1));

    int32_t result = ChannelManager::GetInstance().Init(ownerName);

    EXPECT_EQ(result, CREATE_SOCKET_FAILED);
}

/**
 * @tc.name: TestInit_ListenFailure
 * @tc.desc: Test for Init when Listen fails
 * @tc.type: FUNC
 */
HWTEST_F(ChannelManagerTest, TestInit_ListenFailure, TestSize.Level1)
{
    EXPECT_CALL(mockSoftbus, Socket(testing::_))
        .WillOnce(testing::Return(1234));

    EXPECT_CALL(mockSoftbus, Listen(testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(-2));

    int32_t result = ChannelManager::GetInstance().Init(ownerName);

    EXPECT_EQ(result, LISTEN_SOCKET_FAILED);
}

/**
 * @tc.name: CreateServerChannel_Success
 * @tc.desc: Test for CreateServerChannel when the base channel is successfully created
 * @tc.type: FUNC
 */
HWTEST_F(ChannelManagerTest, CreateServerChannel_Success, TestSize.Level1)
{
    std::string channelName = "TestChannel";
    ChannelDataType dataType = ChannelDataType::MESSAGE;
    ChannelPeerInfo peerInfo = { "peerName", "networkId" };

    int32_t channelId = ChannelManager::GetInstance().CreateServerChannel(channelName, dataType, peerInfo);
    EXPECT_EQ(channelId, 1001);
}

/**
 * @tc.name: CreateServerChannel_ExceedMax_Failed
 * @tc.desc: Test for CreateServerChannel when the base channel is ExceedMax_Failed
 * @tc.type: FUNC
 */
HWTEST_F(ChannelManagerTest, CreateServerChannel_ExceedMax_Failed, TestSize.Level1)
{
    std::string channelName = "TestChannel";
    ChannelDataType dataType = ChannelDataType::MESSAGE;
    ChannelPeerInfo peerInfo = { "peerName", "networkId" };

    int32_t channelId = 0;
    for (int32_t i = 0; i < 1001; i++)
    {
        channelId = ChannelManager::GetInstance().CreateServerChannel(channelName, dataType, peerInfo);
    }
    EXPECT_EQ(channelId, CREATE_SERVER_CHANNEL_FAILED);
}
} // namespace DistributedCollab
} // namespace OHOS