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

#include "ability_connection_session_test.h"

#include "ability_connection_info.h"
#include "ability_connection_session_listener.h"
#include "av_sender_filter.h"
#include "dtbschedmgr_log.h"
#include "dtbcollabmgr_log.h"
#include "softbus_error_code.h"
#include "test_log.h"
#include "tokenid_kit_mock.h"

#include "message_data_header.h"
#include "pixel_map.h"

#include <chrono>
#include <future>

using namespace testing;
using namespace testing::ext;

namespace {
    const int32_t WAITTIME = 10000;
    // Constants for the detach-thread UAF regression tests.
    const int32_t TEST_SESSION_ID = 2;
    const int32_t TEST_CHANNEL_ID = 31;
    const int32_t ASYNC_HANDLER_SLEEP_MS = 100;
    const int32_t FUTURE_WAIT_TIMEOUT_S = 2;
    const int32_t RELEASE_SETTLE_MS = 300;
    const int32_t STRESS_LOOP_COUNT = 50;
    const int32_t STRESS_SESSION_ID_BASE = 100;
    const int32_t STRESS_CHANNEL_ID_BASE = 200;
    const int32_t STREAM_SESSION_ID_BASE = 300;
    const int32_t STRESS_SETTLE_MS = 200;
}

namespace OHOS {
namespace DistributedCollab {
void AbilityConnectionSessionTest::SetUpTestCase()
{
    DTEST_LOG << "AbilityConnectionSessionTest::SetUpTestCase" << std::endl;
    PeerInfo peerInfo = {"", "bundleName", "moduleName", "abilityName", "serverId"};
    PeerInfo localInfo = {"", "bundleName1", "moduleName1", "abilityName1", "serverId1"};
    ConnectOption options;
    std::string serverId = "test";
    AbilityConnectionSessionInfo info{serverId, localInfo, peerInfo};
    connectionSesion_ = std::make_shared<AbilityConnectionSession>(1, "serverSocketName",
        info, options);
}

void AbilityConnectionSessionTest::TearDownTestCase()
{
    DTEST_LOG << "AbilityConnectionSessionTest::TearDownTestCase" << std::endl;
}

void AbilityConnectionSessionTest::TearDown()
{
    DTEST_LOG << "AbilityConnectionSessionTest::TearDown" << std::endl;
    usleep(WAITTIME);
}

void AbilityConnectionSessionTest::SetUp()
{
    DTEST_LOG << "AbilityConnectionSessionTest::SetUp" << std::endl;
}

/**
 * @tc.name: IsVaildChannel_Test_001
 * @tc.desc: call IsVaildChannel
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(AbilityConnectionSessionTest, IsVaildChannel_Test_001, TestSize.Level3)
{
    DTEST_LOG << "AbilityConnectionSessionTest IsVaildPeerInfo_Test_001 begin" << std::endl;
    ASSERT_NE(connectionSesion_, nullptr);
    int32_t channelId = 30;
    connectionSesion_->transChannels_.clear();
    EXPECT_FALSE(connectionSesion_->IsVaildChannel(channelId));

    TransChannelInfo info;
    info.channelId = 31;
    connectionSesion_->transChannels_[TransChannelType::MESSAGE] = info;
    EXPECT_FALSE(connectionSesion_->IsVaildChannel(channelId));

    info.channelId = channelId;
    connectionSesion_->transChannels_[TransChannelType::DATA] = info;
    EXPECT_TRUE(connectionSesion_->IsVaildChannel(channelId));
    connectionSesion_->transChannels_.clear();
    DTEST_LOG << "AbilityConnectionSessionTest IsVaildChannel_Test_001 end" << std::endl;
}

/**
 * @tc.name: GetStreamTransChannel_Test_001
 * @tc.desc: call GetStreamTransChannel
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(AbilityConnectionSessionTest, GetStreamTransChannel_Test_001, TestSize.Level3)
{
    DTEST_LOG << "AbilityConnectionSessionTest GetStreamTransChannel_Test_001 begin" << std::endl;
    ASSERT_NE(connectionSesion_, nullptr);
    TransChannelInfo info;
    connectionSesion_->transChannels_.clear();
    auto rlt = connectionSesion_->GetStreamTransChannel(info);
    EXPECT_EQ(rlt, INVALID_PARAMETERS_ERR);

    TransChannelInfo info1;
    info1.channelId = 31;
    connectionSesion_->transChannels_[TransChannelType::STREAM_BYTES] = info1;
    rlt = connectionSesion_->GetStreamTransChannel(info);
    EXPECT_EQ(rlt, INVALID_PARAMETERS_ERR);

    connectionSesion_->transChannels_.erase(TransChannelType::STREAM_BYTES);
    info1.isConnected = true;
    connectionSesion_->transChannels_[TransChannelType::STREAM_BYTES] = info1;
    rlt = connectionSesion_->GetStreamTransChannel(info);
    EXPECT_EQ(rlt, ERR_OK);
    EXPECT_EQ(info.channelId, info1.channelId);

    TransChannelInfo info2;
    info2.channelId = 30;
    connectionSesion_->transChannels_[TransChannelType::STREAM] = info2;
    rlt = connectionSesion_->GetStreamTransChannel(info);
    EXPECT_EQ(rlt, ERR_OK);
    EXPECT_EQ(info.channelId, info1.channelId);

    connectionSesion_->transChannels_.erase(TransChannelType::STREAM);
    info2.isConnected = true;
    connectionSesion_->transChannels_[TransChannelType::STREAM] = info2;
    rlt = connectionSesion_->GetStreamTransChannel(info);
    EXPECT_EQ(rlt, ERR_OK);
    EXPECT_EQ(info.channelId, info2.channelId);
    connectionSesion_->transChannels_.clear();
    DTEST_LOG << "AbilityConnectionSessionTest GetStreamTransChannel_Test_001 end" << std::endl;
}

/**
 * @tc.name: ConvertToSurfaceParam_Test_001
 * @tc.desc: call ConvertToSurfaceParam
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(AbilityConnectionSessionTest, ConvertToSurfaceParam_Test_001, TestSize.Level3)
{
    DTEST_LOG << "AbilityConnectionSessionTest ConvertToSurfaceParam_Test_001 begin" << std::endl;
    ASSERT_NE(connectionSesion_, nullptr);
    SurfaceParams param;
    param.rotation = SURFACE_ROTATE_NONE;
    param.flip = FlipOptions::HORIZONTAL;
    auto outParam = connectionSesion_->ConvertToSurfaceParam(param);
    EXPECT_EQ(outParam.rotate, SurfaceRotate::ROTATE_NONE);
    EXPECT_EQ(outParam.filp, SurfaceFilp::FLIP_H);

    param.rotation = SURFACE_ROTATE_90;
    param.flip = FlipOptions::VERTICAL;
    outParam = connectionSesion_->ConvertToSurfaceParam(param);
    EXPECT_EQ(outParam.rotate, SurfaceRotate::ROTATE_90);
    EXPECT_EQ(outParam.filp, SurfaceFilp::FLIP_V);

    param.rotation = SURFACE_ROTATE_180;
    param.flip = FlipOptions::UNKNOWN;
    outParam = connectionSesion_->ConvertToSurfaceParam(param);
    EXPECT_EQ(outParam.rotate, SurfaceRotate::ROTATE_180);
    EXPECT_EQ(outParam.filp, SurfaceFilp::FLIP_NONE);

    param.rotation = SURFACE_ROTATE_270;
    param.flip = FlipOptions::HORIZONTAL;
    outParam = connectionSesion_->ConvertToSurfaceParam(param);
    EXPECT_EQ(outParam.rotate, SurfaceRotate::ROTATE_270);
    EXPECT_EQ(outParam.filp, SurfaceFilp::FLIP_H);

    param.rotation = static_cast<SurfaceRotateParams>(1);
    param.flip = FlipOptions::HORIZONTAL;
    outParam = connectionSesion_->ConvertToSurfaceParam(param);
    EXPECT_EQ(outParam.rotate, SurfaceRotate::ROTATE_NONE);
    EXPECT_EQ(outParam.filp, SurfaceFilp::FLIP_H);
    DTEST_LOG << "AbilityConnectionSessionTest ConvertToSurfaceParam_Test_001 end" << std::endl;
}

/**
 * @tc.name: OnMessageReceived_Test_001
 * @tc.desc: call OnMessageReceived
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(AbilityConnectionSessionTest, OnMessageReceived_Test_001, TestSize.Level3)
{
    DTEST_LOG << "AbilityConnectionSessionTest OnMessageReceived_Test_001 begin" << std::endl;
    ASSERT_NE(connectionSesion_, nullptr);
    int32_t channelId = 30;
    size_t capacity = 20;
    std::shared_ptr<AVTransDataBuffer> dataBuffer = std::make_shared<AVTransDataBuffer>(capacity);
    connectionSesion_->transChannels_.clear();
    EXPECT_NO_FATAL_FAILURE(connectionSesion_->OnMessageReceived(channelId, dataBuffer));

    TransChannelInfo info;
    info.channelId = channelId;
    connectionSesion_->transChannels_[TransChannelType::MESSAGE] = info;
    EXPECT_NO_FATAL_FAILURE(connectionSesion_->OnMessageReceived(channelId, dataBuffer));

    MessageDataHeader data;
    dataBuffer = data.Serialize();
    EXPECT_NO_FATAL_FAILURE(connectionSesion_->OnMessageReceived(channelId, dataBuffer));
    connectionSesion_->transChannels_.clear();
    DTEST_LOG << "AbilityConnectionSessionTest GetStreamTransChannel_Test_001 end" << std::endl;
}

/**
 * @tc.name: OnSendFile_Test_001
 * @tc.desc: call OnSendFile
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(AbilityConnectionSessionTest, OnSendFile_Test_001, TestSize.Level3)
{
    DTEST_LOG << "AbilityConnectionSessionTest OnSendFileTest_001 begin" << std::endl;
    ASSERT_NE(connectionSesion_, nullptr);
    int32_t channelId = 30;
    FileInfo fileInfo;
    connectionSesion_->transChannels_.clear();
    EXPECT_NO_FATAL_FAILURE(connectionSesion_->OnSendFile(channelId, fileInfo));

    TransChannelInfo info;
    info.channelId = channelId;
    connectionSesion_->transChannels_[TransChannelType::MESSAGE] = info;
    EXPECT_NO_FATAL_FAILURE(connectionSesion_->OnSendFile(channelId, fileInfo));
    connectionSesion_->transChannels_.clear();
    DTEST_LOG << "AbilityConnectionSessionTest OnSendFileTest_001 end" << std::endl;
}

/**
 * @tc.name: OnRecvFile_Test_001
 * @tc.desc: call OnRecvFile
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(AbilityConnectionSessionTest, OnRecvFile_Test_001, TestSize.Level3)
{
    DTEST_LOG << "AbilityConnectionSessionTest OnRecvFile_Test_001 begin" << std::endl;
    ASSERT_NE(connectionSesion_, nullptr);
    int32_t channelId = 30;
    FileInfo fileInfo;
    connectionSesion_->transChannels_.clear();
    EXPECT_NO_FATAL_FAILURE(connectionSesion_->OnRecvFile(channelId, fileInfo));

    TransChannelInfo info;
    info.channelId = channelId;
    connectionSesion_->transChannels_[TransChannelType::MESSAGE] = info;
    EXPECT_NO_FATAL_FAILURE(connectionSesion_->OnRecvFile(channelId, fileInfo));
    connectionSesion_->transChannels_.clear();
    DTEST_LOG << "AbilityConnectionSessionTest OnRecvFile_Test_001 end" << std::endl;
}

/**
 * @tc.name: GetRecvPath_Test_001
 * @tc.desc: call GetRecvPath
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(AbilityConnectionSessionTest, GetRecvPath_Test_001, TestSize.Level3)
{
    DTEST_LOG << "AbilityConnectionSessionTest GetRecvPath_Test_001 begin" << std::endl;
    ASSERT_NE(connectionSesion_, nullptr);
    int32_t channelId = 30;
    FileInfo fileInfo;
    connectionSesion_->transChannels_.clear();
    EXPECT_NO_FATAL_FAILURE(connectionSesion_->GetRecvPath(channelId));

    TransChannelInfo info;
    info.channelId = channelId;
    connectionSesion_->transChannels_[TransChannelType::MESSAGE] = info;
    EXPECT_NO_FATAL_FAILURE(connectionSesion_->GetRecvPath(channelId));
    connectionSesion_->transChannels_.clear();
    DTEST_LOG << "AbilityConnectionSessionTest GetRecvPath_Test_001 end" << std::endl;
}

/**
 * @tc.name: ExeuteMessageEventCallback_Test_001
 * @tc.desc: call ExeuteMessageEventCallback
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(AbilityConnectionSessionTest, ExeuteMessageEventCallback_Test_001, TestSize.Level3)
{
    DTEST_LOG << "AbilityConnectionSessionTest ExeuteMessageEventCallback_Test_001 begin" << std::endl;
    ASSERT_NE(connectionSesion_, nullptr);
    std::string msg = "this is a test msg";
    EXPECT_NO_FATAL_FAILURE(connectionSesion_->ExeuteMessageEventCallback(msg));
    DTEST_LOG << "AbilityConnectionSessionTest GetRecvPath_Test_001 end" << std::endl;
}

/**
 * @tc.name: OnChannelClosed_Test_001
 * @tc.desc: call OnChannelClosed
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(AbilityConnectionSessionTest, OnChannelClosed_Test_001, TestSize.Level3)
{
    DTEST_LOG << "AbilityConnectionSessionTest OnChannelClosed_Test_001 begin" << std::endl;
    ASSERT_NE(connectionSesion_, nullptr);
    int32_t channelId = 30;
    FileInfo fileInfo;
    connectionSesion_->transChannels_.clear();
    EXPECT_NO_FATAL_FAILURE(connectionSesion_->OnChannelClosed(channelId,
        ShutdownReason::SHUTDOWN_REASON_LNN_OFFLINE));

    connectionSesion_->sessionStatus_ = SessionStatus::CONNECTING;
    TransChannelInfo info;
    info.channelId = channelId;
    connectionSesion_->transChannels_[TransChannelType::MESSAGE] = info;
    EXPECT_NO_FATAL_FAILURE(connectionSesion_->OnChannelClosed(channelId,
        ShutdownReason::SHUTDOWN_REASON_LNN_OFFLINE));

    connectionSesion_->sessionStatus_ = SessionStatus::CONNECTED;
    EXPECT_NO_FATAL_FAILURE(connectionSesion_->OnChannelClosed(channelId,
        ShutdownReason::SHUTDOWN_REASON_LNN_OFFLINE));
    connectionSesion_->transChannels_.clear();
    DTEST_LOG << "AbilityConnectionSessionTest OnChannelClosed_Test_001 end" << std::endl;
}

/**
 * @tc.name: IsAllChannelConnected_Test_001
 * @tc.desc: call IsAllChannelConnected
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(AbilityConnectionSessionTest, IsAllChannelConnected_Test_001, TestSize.Level3)
{
    DTEST_LOG << "AbilityConnectionSessionTest IsAllChannelConnected_Test_001 begin" << std::endl;
    ASSERT_NE(connectionSesion_, nullptr);
    int32_t channelId = 30;
    connectionSesion_->transChannels_.clear();
    TransChannelInfo info;
    info.channelId = channelId;
    info.isConnected = true;
    connectionSesion_->transChannels_[TransChannelType::MESSAGE] = info;
    EXPECT_TRUE(connectionSesion_->IsAllChannelConnected());

    info.isConnected = false;
    info.transType = TransChannelType::STREAM;
    connectionSesion_->transChannels_[TransChannelType::MESSAGE] = info;
    EXPECT_TRUE(connectionSesion_->IsAllChannelConnected());

    info.transType = TransChannelType::DATA;
    connectionSesion_->transChannels_[TransChannelType::MESSAGE] = info;
    EXPECT_FALSE(connectionSesion_->IsAllChannelConnected());
    DTEST_LOG << "AbilityConnectionSessionTest OnChannelClosed_Test_001 end" << std::endl;
}

/**
 * @tc.name: AcceptConnect_Test_001
 * @tc.desc: call AcceptConnect
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(AbilityConnectionSessionTest, AcceptConnect_Test_001, TestSize.Level3)
{
    DTEST_LOG << "AbilityConnectionSessionTest AcceptConnect_Test_001 begin" << std::endl;
    ASSERT_NE(connectionSesion_, nullptr);
    int32_t version = 0;
    connectionSesion_->HandlePeerVersion(version);

    std::string token = "token";
    connectionSesion_->sessionStatus_ = SessionStatus::CONNECTED;
    MockIsSystemAppByFullTokenID(true);
    auto ret = connectionSesion_->AcceptConnect(token);
    EXPECT_EQ(ret, INVALID_PARAMETERS_ERR);
    DTEST_LOG << "AbilityConnectionSessionTest AcceptConnect_Test_001 end" << std::endl;
}

/**
 * @tc.name: RequestReceiveFileChannelConnection_Test_001
 * @tc.desc: call RequestReceiveFileChannelConnection
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(AbilityConnectionSessionTest, RequestReceiveFileChannelConnection_Test_001, TestSize.Level3)
{
    DTEST_LOG << "AbilityConnectionSessionTest RequestReceiveFileChannelConnection_Test_001 begin" << std::endl;
    ASSERT_NE(connectionSesion_, nullptr);
    connectionSesion_->RequestReceiveFileChannelConnection();

    connectionSesion_->connectOption_.needSendFile = true;
    connectionSesion_->NotifyPeerSessionConnected();

    connectionSesion_->connectOption_.needSendFile = false;
    connectionSesion_->connectOption_.needReceiveFile = false;
    EXPECT_NO_FATAL_FAILURE(connectionSesion_->NotifyPeerSessionConnected());

    bool isConnected = true;
    EXPECT_NO_FATAL_FAILURE(connectionSesion_->NotifyAppConnectResult(isConnected));
    
    connectionSesion_->sessionStatus_ = SessionStatus::CONNECTING;
    auto ret = connectionSesion_->HandleDisconnect();
    EXPECT_EQ(ret, ERR_OK);
    DTEST_LOG << "AbilityConnectionSessionTest RequestReceiveFileChannelConnection_Test_001 end" << std::endl;
}

/**
 * @tc.name: CreateStream_Test_001
 * @tc.desc: call CreateStream
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(AbilityConnectionSessionTest, CreateStream_Test_001, TestSize.Level3)
{
    DTEST_LOG << "AbilityConnectionSessionTest CreateStream_Test_001 begin" << std::endl;
    ASSERT_NE(connectionSesion_, nullptr);
    StreamParams param;
    param.role = StreamRole::SOURCE;
    connectionSesion_->CreateStream(0, param);

    param.role = StreamRole::SINK;
    connectionSesion_->CreateStream(0, param);

    param.role = static_cast<StreamRole>(-1);
    auto ret = connectionSesion_->CreateStream(0, param);
    EXPECT_EQ(ret, INVALID_PARAMETERS_ERR);
    DTEST_LOG << "AbilityConnectionSessionTest CreateStream_Test_001 end" << std::endl;
}

/**
 * @tc.name: InitSenderEngine_Test_001
 * @tc.desc: call InitSenderEngine
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(AbilityConnectionSessionTest, InitSenderEngine_Test_001, TestSize.Level3)
{
    DTEST_LOG << "AbilityConnectionSessionTest InitSenderEngine_Test_001 begin" << std::endl;
    ASSERT_NE(connectionSesion_, nullptr);
    connectionSesion_->connectOption_.needSendStream = false;
    auto ret = connectionSesion_->InitSenderEngine();
    EXPECT_EQ(ret, INVALID_PARAMETERS_ERR);

    connectionSesion_->connectOption_.needSendStream = true;
    connectionSesion_->senderEngine_ = nullptr;
    ret = connectionSesion_->InitSenderEngine();
    EXPECT_EQ(ret, ERR_OK);

    ret = connectionSesion_->InitSenderEngine();
    EXPECT_EQ(ret, ONLY_SUPPORT_ONE_STREAM);
    DTEST_LOG << "AbilityConnectionSessionTest InitSenderEngine_Test_001 end" << std::endl;
}

/**
 * @tc.name: InitRecvEngine_Test_001
 * @tc.desc: call InitRecvEngine
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(AbilityConnectionSessionTest, InitRecvEngine_Test_001, TestSize.Level3)
{
    DTEST_LOG << "AbilityConnectionSessionTest InitRecvEngine_Test_001 begin" << std::endl;
    ASSERT_NE(connectionSesion_, nullptr);
    connectionSesion_->connectOption_.needReceiveStream = false;
    auto ret = connectionSesion_->InitRecvEngine();
    EXPECT_EQ(ret, INVALID_PARAMETERS_ERR);

    connectionSesion_->connectOption_.needReceiveStream = true;
    connectionSesion_->recvEngine_ = nullptr;
    ret = connectionSesion_->InitRecvEngine();
    EXPECT_EQ(ret, ERR_OK);

    ret = connectionSesion_->InitRecvEngine();
    EXPECT_EQ(ret, ONLY_SUPPORT_ONE_STREAM);
    DTEST_LOG << "AbilityConnectionSessionTest InitRecvEngine_Test_001 end" << std::endl;
}

/**
 * @tc.name: GetSurfaceId_Test_001
 * @tc.desc: call GetSurfaceId
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(AbilityConnectionSessionTest, GetSurfaceId_Test_001, TestSize.Level3)
{
    DTEST_LOG << "AbilityConnectionSessionTest GetSurfaceId_Test_001 begin" << std::endl;
    ASSERT_NE(connectionSesion_, nullptr);
    SurfaceParams param;
    std::string surfaceId = "surfaceId";
    connectionSesion_->senderEngine_ = nullptr;
    connectionSesion_->GetSurfaceId(param, surfaceId);

    connectionSesion_->connectOption_.needSendStream = true;
    connectionSesion_->InitSenderEngine();
    auto ret = connectionSesion_->GetSurfaceId(param, surfaceId);
    EXPECT_NE(ret, ERR_OK);

    ret = connectionSesion_->UpdateSurfaceParam(param);
    EXPECT_EQ(ret, ERR_OK);
    DTEST_LOG << "AbilityConnectionSessionTest GetSurfaceId_Test_001 end" << std::endl;
}

/**
 * @tc.name: SetSurfaceId_Test_001
 * @tc.desc: call SetSurfaceId
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(AbilityConnectionSessionTest, SetSurfaceId_Test_001, TestSize.Level3)
{
    DTEST_LOG << "AbilityConnectionSessionTest SetSurfaceId_Test_001 begin" << std::endl;
    ASSERT_NE(connectionSesion_, nullptr);
    SurfaceParams param;
    std::string surfaceId = "123";
    connectionSesion_->recvEngine_ = nullptr;
    connectionSesion_->SetSurfaceId(surfaceId, param);

    connectionSesion_->connectOption_.needReceiveStream = true;
    connectionSesion_->InitRecvEngine();
    auto ret = connectionSesion_->SetSurfaceId(surfaceId, param);
    EXPECT_NE(ret, ERR_OK);

    connectionSesion_->senderEngine_ = nullptr;
    ret = connectionSesion_->UpdateSurfaceParam(param);
    EXPECT_EQ(ret, ERR_OK);

    connectionSesion_->recvEngine_ = nullptr;
    ret = connectionSesion_->UpdateSurfaceParam(param);
    EXPECT_EQ(ret, INVALID_PARAMETERS_ERR);
    DTEST_LOG << "AbilityConnectionSessionTest SetSurfaceId_Test_001 end" << std::endl;
}

/**
 * @tc.name: StartStream_Test_001
 * @tc.desc: call StartStream
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(AbilityConnectionSessionTest, StartStream_Test_001, TestSize.Level3)
{
    DTEST_LOG << "AbilityConnectionSessionTest StartStream_Test_001 begin" << std::endl;
    ASSERT_NE(connectionSesion_, nullptr);
    int32_t streamId = 0;
    connectionSesion_->connectOption_.needSendStream = true;
    connectionSesion_->InitSenderEngine();
    connectionSesion_->StartStream(streamId);

    connectionSesion_->senderEngine_ = nullptr;
    connectionSesion_->connectOption_.needReceiveStream = true;
    connectionSesion_->InitRecvEngine();
    connectionSesion_->StartStream(streamId);
    
    connectionSesion_->connectOption_.needSendStream = false;
    connectionSesion_->recvEngine_ = nullptr;
    connectionSesion_->StartStream(streamId);

    connectionSesion_->connectOption_.needReceiveStream = false;
    auto ret = connectionSesion_->StartStream(streamId);
    EXPECT_EQ(ret, INVALID_PARAMETERS_ERR);
    DTEST_LOG << "AbilityConnectionSessionTest StartStream_Test_001 end" << std::endl;
}

/**
 * @tc.name: RegisterEventCallback_Test_001
 * @tc.desc: call RegisterEventCallback
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(AbilityConnectionSessionTest, RegisterEventCallback_Test_001, TestSize.Level3)
{
    DTEST_LOG << "AbilityConnectionSessionTest RegisterEventCallback_Test_001 begin" << std::endl;
    ASSERT_NE(connectionSesion_, nullptr);
    auto ret = connectionSesion_->RegisterEventCallback(nullptr);
    EXPECT_EQ(ret, INVALID_LISTENER);

    ret = connectionSesion_->CreateStreamChannel("channelName", false);
    EXPECT_NE(ret, ERR_OK);
    DTEST_LOG << "AbilityConnectionSessionTest RegisterEventCallback_Test_001 end" << std::endl;
}

/**
 * @tc.name: ConnectStreamChannel_Test_001
 * @tc.desc: call ConnectStreamChannel
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(AbilityConnectionSessionTest, ConnectStreamChannel_Test_001, TestSize.Level3)
{
    DTEST_LOG << "AbilityConnectionSessionTest ConnectStreamChannel_Test_001 begin" << std::endl;
    ASSERT_NE(connectionSesion_, nullptr);
    connectionSesion_->connectOption_.needSendStream = false;
    connectionSesion_->connectOption_.needReceiveStream = false;
    auto ret = connectionSesion_->ConnectStreamChannel();
    EXPECT_EQ(ret, ERR_OK);

    connectionSesion_->connectOption_.needReceiveStream = true;
    connectionSesion_->transChannels_.clear();
    ret = connectionSesion_->ConnectStreamChannel();
    EXPECT_EQ(ret, INVALID_PARAMETERS_ERR);

    TransChannelInfo info;
    info.isConnected = true;
    connectionSesion_->connectOption_.needSendStream = true;
    connectionSesion_->transChannels_[TransChannelType::STREAM] = info;
    ret = connectionSesion_->ConnectStreamChannel();
    EXPECT_EQ(ret, ERR_OK);

    connectionSesion_->transChannels_.clear();
    info.isConnected = false;
    connectionSesion_->transChannels_[TransChannelType::STREAM] = info;
    ret = connectionSesion_->ConnectStreamChannel();
    EXPECT_EQ(ret, ERR_OK);

    connectionSesion_->direction_ = CollabrateDirection::COLLABRATE_SOURCE;
    ret = connectionSesion_->ConnectStreamChannel();
    EXPECT_EQ(ret, ERR_OK);
    DTEST_LOG << "AbilityConnectionSessionTest ConnectStreamChannel_Test_001 end" << std::endl;
}

/**
 * @tc.name: DoConnectStreamChannel_Test_001
 * @tc.desc: call DoConnectStreamChannel
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(AbilityConnectionSessionTest, DoConnectStreamChannel_Test_001, TestSize.Level3)
{
    DTEST_LOG << "AbilityConnectionSessionTest DoConnectStreamChannel_Test_001 begin" << std::endl;
    ASSERT_NE(connectionSesion_, nullptr);
    int32_t channelId = 0;
    auto ret = connectionSesion_->DoConnectStreamChannel(channelId);
    EXPECT_NE(ret, ERR_OK);
    DTEST_LOG << "AbilityConnectionSessionTest DoConnectStreamChannel_Test_001 end" << std::endl;
}

/**
 * @tc.name: OnChannelConnect_Test_001
 * @tc.desc: call OnChannelConnect
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(AbilityConnectionSessionTest, OnChannelConnect_Test_001, TestSize.Level3)
{
    DTEST_LOG << "AbilityConnectionSessionTest OnChannelConnect_Test_001 begin" << std::endl;
    ASSERT_NE(connectionSesion_, nullptr);
    int32_t channelId = 0;
    connectionSesion_->transChannels_.clear();
    EXPECT_NO_FATAL_FAILURE(connectionSesion_->OnChannelConnect(channelId));

    TransChannelInfo info;
    info.channelId = channelId;
    connectionSesion_->connectOption_.needSendStream = true;
    connectionSesion_->transChannels_[TransChannelType::STREAM] = info;
    EXPECT_NO_FATAL_FAILURE(connectionSesion_->OnChannelConnect(channelId));

    EXPECT_NO_FATAL_FAILURE(connectionSesion_->UpdateTransChannelStatus(channelId, false));
    EXPECT_NO_FATAL_FAILURE(connectionSesion_->UpdateTransChannelStatus(1, false));
    DTEST_LOG << "AbilityConnectionSessionTest OnChannelConnect_Test_001 end" << std::endl;
}

/**
 * @tc.name: UpdateRecvEngineTransChannel_Test_001
 * @tc.desc: call UpdateRecvEngineTransChannel
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(AbilityConnectionSessionTest, UpdateRecvEngineTransChannel_Test_001, TestSize.Level3)
{
    DTEST_LOG << "AbilityConnectionSessionTest UpdateRecvEngineTransChannel_Test_001 begin" << std::endl;
    ASSERT_NE(connectionSesion_, nullptr);
    connectionSesion_->recvEngine_ = nullptr;
    EXPECT_NO_FATAL_FAILURE(connectionSesion_->UpdateRecvEngineTransChannel());

    connectionSesion_->transChannels_.clear();
    connectionSesion_->InitRecvEngine();
    EXPECT_NO_FATAL_FAILURE(connectionSesion_->UpdateRecvEngineTransChannel());

    TransChannelInfo info;
    connectionSesion_->connectOption_.needSendStream = true;
    connectionSesion_->transChannels_[TransChannelType::STREAM] = info;
    EXPECT_NO_FATAL_FAILURE(connectionSesion_->UpdateRecvEngineTransChannel());
    DTEST_LOG << "AbilityConnectionSessionTest UpdateRecvEngineTransChannel_Test_001 end" << std::endl;
}

/**
 * @tc.name: UpdateSenderEngineTransChannel_Test_001
 * @tc.desc: call UpdateSenderEngineTransChannel
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(AbilityConnectionSessionTest, UpdateSenderEngineTransChannel_Test_001, TestSize.Level3)
{
    DTEST_LOG << "AbilityConnectionSessionTest UpdateSenderEngineTransChannel_Test_001 begin" << std::endl;
    ASSERT_NE(connectionSesion_, nullptr);
    connectionSesion_->senderEngine_ = nullptr;
    EXPECT_NO_FATAL_FAILURE(connectionSesion_->UpdateSenderEngineTransChannel());

    connectionSesion_->transChannels_.clear();
    connectionSesion_->InitSenderEngine();
    EXPECT_NO_FATAL_FAILURE(connectionSesion_->UpdateSenderEngineTransChannel());

    TransChannelInfo info;
    connectionSesion_->connectOption_.needSendStream = true;
    connectionSesion_->transChannels_[TransChannelType::STREAM] = info;
    EXPECT_NO_FATAL_FAILURE(connectionSesion_->UpdateSenderEngineTransChannel());
    DTEST_LOG << "AbilityConnectionSessionTest UpdateSenderEngineTransChannel_Test_001 end" << std::endl;
}
/**
 * @tc.name: ConnectFileChannel_Test_001
 * @tc.desc: call ConnectFileChannel
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(AbilityConnectionSessionTest, ConnectFileChannel_Test_001, TestSize.Level3)
{
    DTEST_LOG << "AbilityConnectionSessionTest ConnectFileChannel_Test_001 begin" << std::endl;
    ASSERT_NE(connectionSesion_, nullptr);
    std::string peerSocketName = "peerSocketName";
    EXPECT_NO_FATAL_FAILURE(connectionSesion_->ConnectFileChannel(peerSocketName));

    EXPECT_NO_FATAL_FAILURE(connectionSesion_->OnRecvPixelMap(nullptr));

    int32_t channelId = 0;
    size_t capacity = 20;
    std::shared_ptr<AVTransDataBuffer> dataBuffer = std::make_shared<AVTransDataBuffer>(capacity);
    connectionSesion_->transChannels_.clear();
    EXPECT_NO_FATAL_FAILURE(connectionSesion_->OnBytesReceived(channelId, dataBuffer));

    TransChannelInfo info;
    info.channelId = channelId;
    connectionSesion_->transChannels_[TransChannelType::STREAM] = info;
    EXPECT_NO_FATAL_FAILURE(connectionSesion_->OnBytesReceived(channelId, dataBuffer));

    connectionSesion_->transChannels_.clear();
    EXPECT_NO_FATAL_FAILURE(connectionSesion_->OnBytesReceived(channelId, dataBuffer));

    connectionSesion_->transChannels_[TransChannelType::STREAM_BYTES] = info;
    EXPECT_NO_FATAL_FAILURE(connectionSesion_->OnBytesReceived(channelId, dataBuffer));

    connectionSesion_->sessionListener_ = nullptr;
    EXPECT_NO_FATAL_FAILURE(connectionSesion_->OnBytesReceived(channelId, dataBuffer));
    DTEST_LOG << "AbilityConnectionSessionTest ConnectFileChannel_Test_001 end" << std::endl;
}

/**
 * @tc.name: SetTimeOut_Test_001
 * @tc.desc: call SetTimeOut
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(AbilityConnectionSessionTest, SetTimeOut_Test_001, TestSize.Level3)
{
    DTEST_LOG << "AbilityConnectionSessionTest SetTimeOut_Test_001 begin" << std::endl;
    ASSERT_NE(connectionSesion_, nullptr);
    connectionSesion_->eventHandler_ = nullptr;
    EXPECT_NO_FATAL_FAILURE(connectionSesion_->SetTimeOut(1));
    EXPECT_NO_FATAL_FAILURE(connectionSesion_->RemoveTimeout());
    DTEST_LOG << "AbilityConnectionSessionTest SetTimeOut_Test_001 end" << std::endl;
}

/**
 * @tc.name: SendData_Test_001
 * @tc.desc: call SendData
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(AbilityConnectionSessionTest, SendData_Test_001, TestSize.Level3)
{
    DTEST_LOG << "AbilityConnectionSessionTest SendData_Test_001 begin" << std::endl;
    ASSERT_NE(connectionSesion_, nullptr);
    size_t capacity = 20;
    std::shared_ptr<AVTransDataBuffer> dataBuffer = std::make_shared<AVTransDataBuffer>(capacity);
    EXPECT_NO_FATAL_FAILURE(connectionSesion_->SendData(dataBuffer));
    DTEST_LOG << "AbilityConnectionSessionTest SendData_Test_001 end" << std::endl;
}

/**
 * @tc.name: SendImage_Test_001
 * @tc.desc: call SendImage
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(AbilityConnectionSessionTest, SendImage_Test_001, TestSize.Level3)
{
    DTEST_LOG << "AbilityConnectionSessionTest SendImage_Test_001 begin" << std::endl;
    ASSERT_NE(connectionSesion_, nullptr);
    connectionSesion_->InitSenderEngine();
    int32_t imageQuality = 30;
    EXPECT_NO_FATAL_FAILURE(connectionSesion_->SendImage(nullptr, imageQuality));
    DTEST_LOG << "AbilityConnectionSessionTest SendImage_Test_001 end" << std::endl;
}

/**
 * @tc.name: StopStream_Test_001
 * @tc.desc: call StopStream
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(AbilityConnectionSessionTest, StopStream_Test_001, TestSize.Level3)
{
    DTEST_LOG << "AbilityConnectionSessionTest StopStream_Test_001 begin" << std::endl;
    ASSERT_NE(connectionSesion_, nullptr);
    connectionSesion_->connectOption_.needSendStream = true;
    connectionSesion_->connectOption_.needReceiveStream = true;
    connectionSesion_->senderEngine_ = nullptr;
    connectionSesion_->recvEngine_ = nullptr;
    EXPECT_NO_FATAL_FAILURE(connectionSesion_->StopStream(0));

    connectionSesion_->InitRecvEngine();
    EXPECT_NO_FATAL_FAILURE(connectionSesion_->StopStream(0));

    connectionSesion_->InitSenderEngine();
    EXPECT_NO_FATAL_FAILURE(connectionSesion_->StopStream(0));
    DTEST_LOG << "AbilityConnectionSessionTest StopStream_Test_001 end" << std::endl;
}

/**
 * @tc.name: ExeuteEventCallback_Test_001
 * @tc.desc: call ExeuteEventCallback
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(AbilityConnectionSessionTest, ExeuteEventCallback_Test_001, TestSize.Level3)
{
    DTEST_LOG << "AbilityConnectionSessionTest ExeuteEventCallback_Test_001 begin" << std::endl;
    ASSERT_NE(connectionSesion_, nullptr);
    connectionSesion_->listeners_["connect"] = nullptr;
    EventCallbackInfo callbackInfo;
    auto ret = connectionSesion_->ExeuteEventCallback("connect1", callbackInfo);
    EXPECT_EQ(ret, INVALID_PARAMETERS_ERR);

    ret = connectionSesion_->ExeuteEventCallback("connect", callbackInfo);
    EXPECT_EQ(ret, INVALID_PARAMETERS_ERR);
    DTEST_LOG << "AbilityConnectionSessionTest ExeuteEventCallback_Test_001 end" << std::endl;
}

/**
 * @tc.name: CollabChannelListener_Test_001
 * @tc.desc: call CollabChannelListener
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(AbilityConnectionSessionTest, CollabChannelListener_Test_001, TestSize.Level3)
{
    DTEST_LOG << "AbilityConnectionSessionTest CollabChannelListener_Test_001 begin" << std::endl;
    ASSERT_NE(connectionSesion_, nullptr);
    connectionSesion_->channelListener_ = std::make_shared<AbilityConnectionSession::CollabChannelListener>(nullptr);
    EXPECT_NO_FATAL_FAILURE(connectionSesion_->channelListener_->OnConnect(0));

    EXPECT_NO_FATAL_FAILURE(connectionSesion_->channelListener_->OnDisConnect(0,
        ShutdownReason::SHUTDOWN_REASON_UNKNOWN));

    EXPECT_NO_FATAL_FAILURE(connectionSesion_->channelListener_->OnMessage(0, nullptr));

    EXPECT_NO_FATAL_FAILURE(connectionSesion_->channelListener_->OnBytes(0, nullptr));

    FileInfo info;
    EXPECT_NO_FATAL_FAILURE(connectionSesion_->channelListener_->OnSendFile(0, info));

    EXPECT_NO_FATAL_FAILURE(connectionSesion_->channelListener_->OnRecvFile(0, info));

    EXPECT_NO_FATAL_FAILURE(connectionSesion_->channelListener_->GetRecvPath(0));

    connectionSesion_->pixelMapListener = std::make_shared<AbilityConnectionSession::PixelMapListener>();
    EXPECT_NO_FATAL_FAILURE(connectionSesion_->pixelMapListener->OnRecvPixelMap(nullptr));
    DTEST_LOG << "AbilityConnectionSessionTest CollabChannelListener_Test_001 end" << std::endl;
}

/**
 * @tc.name: HandleSessionConnect_Test_001
 * @tc.desc: call HandleSessionConnect
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(AbilityConnectionSessionTest, HandleSessionConnect_Test_001, TestSize.Level3)
{
    DTEST_LOG << "AbilityConnectionSessionTest HandleSessionConnect_Test_001 begin" << std::endl;
    ASSERT_NE(connectionSesion_, nullptr);
    connectionSesion_->sessionStatus_ = SessionStatus::CONNECTED;
    EXPECT_NO_FATAL_FAILURE(connectionSesion_->HandleSessionConnect());

    connectionSesion_->sessionListener_ = nullptr;
    EXPECT_NO_FATAL_FAILURE(connectionSesion_->HandleSessionConnect());

    EXPECT_NO_FATAL_FAILURE(connectionSesion_->OnChannelClosed(30, ShutdownReason::SHUTDOWN_REASON_LNN_OFFLINE));

    EXPECT_NO_FATAL_FAILURE(connectionSesion_->OnError(0, 0));
    DTEST_LOG << "AbilityConnectionSessionTest HandleSessionConnect_Test_001 end" << std::endl;
}

/**
 * @tc.name: ConvertToConnectErrorCode_Test_001
 * @tc.desc: call ConvertToConnectErrorCode
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(AbilityConnectionSessionTest, ConvertToConnectErrorCode_Test_001, TestSize.Level3)
{
    DTEST_LOG << "AbilityConnectionSessionTest ConvertToConnectErrorCode_Test_001 begin" << std::endl;
    ASSERT_NE(connectionSesion_, nullptr);
    int32_t collabResult = CONNECTED_SESSION_EXISTS;
    auto ret = connectionSesion_->ConvertToConnectErrorCode(collabResult);
    EXPECT_EQ(ret, ConnectErrorCode::CONNECTED_SESSION_EXISTS);

    collabResult = SAME_SESSION_IS_CONNECTING;
    ret = connectionSesion_->ConvertToConnectErrorCode(collabResult);
    EXPECT_EQ(ret, ConnectErrorCode::CONNECTED_SESSION_EXISTS);

    collabResult = PEER_WIFI_NOT_OPEN;
    ret = connectionSesion_->ConvertToConnectErrorCode(collabResult);
    EXPECT_EQ(ret, ConnectErrorCode::PEER_WIFI_NOT_OPEN);

    collabResult = SOFTBUS_CONN_PASSIVE_TYPE_AP_P2P_CHIP_CONFLICT;
    ret = connectionSesion_->ConvertToConnectErrorCode(collabResult);
    EXPECT_EQ(ret, ConnectErrorCode::PEER_WIFI_NOT_OPEN);

    collabResult = SOFTBUS_CONN_PASSIVE_TYPE_AP_STA_P2P_CHIP_CONFLICT;
    ret = connectionSesion_->ConvertToConnectErrorCode(collabResult);
    EXPECT_EQ(ret, ConnectErrorCode::PEER_WIFI_NOT_OPEN);

    collabResult = SOFTBUS_CONN_PASSIVE_TYPE_AP_STA_CHIP_CONFLICT;
    ret = connectionSesion_->ConvertToConnectErrorCode(collabResult);
    EXPECT_EQ(ret, ConnectErrorCode::PEER_WIFI_NOT_OPEN);

    collabResult = SOFTBUS_CONN_PASSIVE_TYPE_AP_HML_CHIP_CONFLICT;
    ret = connectionSesion_->ConvertToConnectErrorCode(collabResult);
    EXPECT_EQ(ret, ConnectErrorCode::PEER_WIFI_NOT_OPEN);

    collabResult = SOFTBUS_CONN_PASSIVE_TYPE_AP_STA_HML_CHIP_CONFLICT;
    ret = connectionSesion_->ConvertToConnectErrorCode(collabResult);
    EXPECT_EQ(ret, ConnectErrorCode::PEER_WIFI_NOT_OPEN);

    collabResult = DistributedSchedule::COLLAB_ABILITY_REJECT_ERR;
    ret = connectionSesion_->ConvertToConnectErrorCode(collabResult);
    EXPECT_EQ(ret, ConnectErrorCode::PEER_APP_REJECTED);

    collabResult = PEER_ABILITY_NO_ONCOLLABORATE;
    ret = connectionSesion_->ConvertToConnectErrorCode(collabResult);
    EXPECT_EQ(ret, ConnectErrorCode::PEER_ABILITY_NO_ONCOLLABORATE);
    DTEST_LOG << "AbilityConnectionSessionTest ConvertToConnectErrorCode_Test_001 end" << std::endl;
}

/**
 * @tc.name: ConvertToDisconnectReason_Test_001
 * @tc.desc: call ConvertToDisconnectReason
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(AbilityConnectionSessionTest, ConvertToDisconnectReason_Test_001, TestSize.Level3)
{
    DTEST_LOG << "AbilityConnectionSessionTest ConvertToDisconnectReason_Test_001 begin" << std::endl;
    ASSERT_NE(connectionSesion_, nullptr);
    ShutdownReason reason = ShutdownReason::SHUTDOWN_REASON_PEER;
    auto ret = connectionSesion_->ConvertToDisconnectReason(reason);
    EXPECT_EQ(ret, DisconnectReason::PEER_APP_EXIT);
    DTEST_LOG << "AbilityConnectionSessionTest ConvertToDisconnectReason_Test_001 end" << std::endl;
}

/**
 * @tc.name: CheckConnectedSession_Test_001
 * @tc.desc: call CheckConnectedSession
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(AbilityConnectionSessionTest, CheckConnectedSession_Test_001, TestSize.Level3)
{
    DTEST_LOG << "AbilityConnectionSessionTest CheckConnectedSession_Test_001 begin" << std::endl;
    ASSERT_NE(connectionSesion_, nullptr);
    AbilityConnectionSession::ConnectCallback callback = nullptr;
    connectionSesion_->sessionStatus_ = SessionStatus::CONNECTED;
    auto result = connectionSesion_->Connect(callback);
    EXPECT_EQ(result, CONNECTED_SESSION_EXISTS);

    auto ret = connectionSesion_->CheckConnectedSession();
    EXPECT_EQ(ret, true);

    connectionSesion_->sessionStatus_ = SessionStatus::CONNECTING;
    ret = connectionSesion_->CheckConnectedSession();
    EXPECT_EQ(ret, false);

    EXPECT_NO_FATAL_FAILURE(connectionSesion_->Connect(callback));
    DTEST_LOG << "AbilityConnectionSessionTest CheckConnectedSession_Test_001 end" << std::endl;
}

/**
 * @tc.name: ConnectTransChannel_Test_001
 * @tc.desc: call ConnectTransChannel
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(AbilityConnectionSessionTest, ConnectTransChannel_Test_001, TestSize.Level3)
{
    DTEST_LOG << "AbilityConnectionSessionTest ConnectTransChannel_Test_001 begin" << std::endl;
    ASSERT_NE(connectionSesion_, nullptr);
    connectionSesion_->transChannels_.clear();
    auto result = connectionSesion_->ConnectTransChannel(TransChannelType::MESSAGE);
    EXPECT_EQ(result, STREAM_CHANNEL_NOT_EXITS);
    DTEST_LOG << "AbilityConnectionSessionTest ConnectTransChannel_Test_001 end" << std::endl;
}

/**
 * @tc.name: SendMessage_Test_001
 * @tc.desc: call SendData
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(AbilityConnectionSessionTest, SendMessage_Test_001, TestSize.Level3)
{
    DTEST_LOG << "AbilityConnectionSessionTest SendMessage_Test_001 begin" << std::endl;
    ASSERT_NE(connectionSesion_, nullptr);
    int32_t ret = connectionSesion_->SendMessage("SESSION_CONNECT_SUCCESS", MessageType::SESSION_CONNECT_SUCCESS);
    EXPECT_EQ(ret, INVALID_PARAMETERS_ERR);
    DTEST_LOG << "AbilityConnectionSessionTest SendMessage_Test_001 end" << std::endl;
}

/**
 * @tc.name: SendMessage_Test_001
 * @tc.desc: call SendData
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(AbilityConnectionSessionTest, SendMessageSync_Test_001, TestSize.Level3)
{
    DTEST_LOG << "AbilityConnectionSessionTest SendMessageSync_Test_001 begin" << std::endl;
    ASSERT_NE(connectionSesion_, nullptr);
    int32_t ret = connectionSesion_->SendMessageSync("SESSION_CONNECT_SUCCESS", MessageType::SESSION_CONNECT_SUCCESS);
    EXPECT_EQ(ret, INVALID_PARAMETERS_ERR);
    DTEST_LOG << "AbilityConnectionSessionTest SendMessageSync_Test_001 end" << std::endl;
}

/**
 * @tc.name: OnMessageReceived_AsyncDetachHandler_NoUafWhenReleased_Test_001
 * @tc.desc: Verify the weak_ptr-guarded async message handler (fix for the
 *           std::thread(handler, ...).detach() + raw-this UAF) keeps the session alive while
 *           the handler runs and does not use-after-free when the caller releases its reference.
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(AbilityConnectionSessionTest, OnMessageReceived_AsyncDetachHandler_NoUafWhenReleased_Test_001,
    TestSize.Level3)
{
    DTEST_LOG << "OnMessageReceived_AsyncDetachHandler_NoUafWhenReleased_Test_001 begin" << std::endl;
    PeerInfo peerInfo = {"", "bn", "mn", "an", "sid"};
    PeerInfo localInfo = {"", "bn1", "mn1", "an1", "sid1"};
    AbilityConnectionSessionInfo info{"test", localInfo, peerInfo};
    ConnectOption options;
    auto session = std::make_shared<AbilityConnectionSession>(TEST_SESSION_ID, "serverSocketName", info, options);
    ASSERT_NE(session, nullptr);

    // Inject a handler that sets a latch then sleeps, keeping the detached thread busy so we can
    // race session release against the still-running handler.
    auto promisePtr = std::make_shared<std::promise<bool>>();
    std::future<bool> fut = promisePtr->get_future();
    session->messageHandlerMap_[static_cast<uint32_t>(MessageType::CONNECT_FILE_CHANNEL)] =
        [promisePtr](const std::string&) {
            promisePtr->set_value(true);
            std::this_thread::sleep_for(std::chrono::milliseconds(ASYNC_HANDLER_SLEEP_MS));
        };

    // Make IsVaildChannel pass; deliberately do NOT register a SEND_FILE channel so
    // GetTransChannelInfo(SEND_FILE) fails -> the std::thread(handler, ...).detach() path runs.
    int32_t channelId = TEST_CHANNEL_ID;
    TransChannelInfo chInfo;
    chInfo.channelId = channelId;
    session->transChannels_[TransChannelType::MESSAGE] = chInfo;

    MessageDataHeader header(MessageDataHeader::PROTOCOL_VERSION,
        static_cast<uint32_t>(MessageType::CONNECT_FILE_CHANNEL), 0);
    std::shared_ptr<AVTransDataBuffer> dataBuffer;
    dataBuffer = header.Serialize();
    ASSERT_NE(dataBuffer, nullptr);
    EXPECT_NO_FATAL_FAILURE(session->OnMessageReceived(channelId, dataBuffer));

    // Alive path: the detached thread locked the weak_ptr, ran the handler, set the latch.
    EXPECT_EQ(fut.wait_for(std::chrono::seconds(FUTURE_WAIT_TIMEOUT_S)), std::future_status::ready);
    EXPECT_TRUE(fut.get());

    // Release the test's reference while the handler is still sleeping. With the weak_ptr fix,
    // the detached thread holds a shared_ptr (from lock()) for the handler's duration, so the
    // object is not destroyed here; it is destroyed only after the thread exits. No UAF.
    session.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(RELEASE_SETTLE_MS));
    DTEST_LOG << "OnMessageReceived_AsyncDetachHandler_NoUafWhenReleased_Test_001 end" << std::endl;
}

/**
 * @tc.name: OnMessageReceived_AsyncDetachHandler_StressRelease_Test_001
 * @tc.desc: Stress/amplification for the "session released before the detached thread locks the
 *           weak_ptr" case. Run under AddressSanitizer to catch UAF regressions; the weak_ptr
 *           fix makes the handler bail safely (lock() returns nullptr) when the session is gone.
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(AbilityConnectionSessionTest, OnMessageReceived_AsyncDetachHandler_StressRelease_Test_001, TestSize.Level3)
{
    DTEST_LOG << "OnMessageReceived_AsyncDetachHandler_StressRelease_Test_001 begin" << std::endl;
    PeerInfo peerInfo = {"", "bn", "mn", "an", "sid"};
    PeerInfo localInfo = {"", "bn1", "mn1", "an1", "sid1"};
    AbilityConnectionSessionInfo info{"test", localInfo, peerInfo};
    ConnectOption options;
    MessageDataHeader header(MessageDataHeader::PROTOCOL_VERSION,
        static_cast<uint32_t>(MessageType::CONNECT_FILE_CHANNEL), 0);
    std::shared_ptr<AVTransDataBuffer> dataBuffer;
    dataBuffer = header.Serialize();
    ASSERT_NE(dataBuffer, nullptr);
    constexpr int32_t loop = STRESS_LOOP_COUNT;
    for (int32_t i = 0; i < loop; ++i) {
        auto session = std::make_shared<AbilityConnectionSession>(STRESS_SESSION_ID_BASE + i,
            "serverSocketName", info, options);
        // No-op handler: exercise only the weak_ptr lock / bail path, avoid ConnectFileChannel deps.
        session->messageHandlerMap_[static_cast<uint32_t>(MessageType::CONNECT_FILE_CHANNEL)] =
            [](const std::string&) {};
        int32_t channelId = STRESS_CHANNEL_ID_BASE + i;
        TransChannelInfo chInfo;
        chInfo.channelId = channelId;
        session->transChannels_[TransChannelType::MESSAGE] = chInfo;
        EXPECT_NO_FATAL_FAILURE(session->OnMessageReceived(channelId, dataBuffer));
        session.reset(); // race: detach thread may lock weak_ptr before or after this
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(STRESS_SETTLE_MS));
    DTEST_LOG << "OnMessageReceived_AsyncDetachHandler_StressRelease_Test_001 end" << std::endl;
}

/**
 * @tc.name: ConnectStreamChannel_DetachThread_Smoke_Test_001
 * @tc.desc: Smoke test for the ConnectStreamChannel detached-thread path (fix for the
 *           detach + raw-this UAF). NOTE: this does NOT reproduce the UAF -- in the unit-test
 *           environment ChannelManager::ConnectChannel fails fast, so DoConnectStreamChannel
 *           returns before touching this->recvEngine_ / UpdateTransChannelStatus (the members
 *           that would UAF). Deterministic UAF coverage requires mocking ChannelManager
 *           (currently `final` + singleton).
 * @tc.type: FUNC
 * @tc.require: I6SJQ6
 */
HWTEST_F(AbilityConnectionSessionTest, ConnectStreamChannel_DetachThread_Smoke_Test_001, TestSize.Level3)
{
    DTEST_LOG << "AbilityConnectionSessionTest ConnectStreamChannel_DetachThread_Smoke_Test_001 begin" << std::endl;
    PeerInfo peerInfo = {"", "bn", "mn", "an", "sid"};
    PeerInfo localInfo = {"", "bn1", "mn1", "an1", "sid1"};
    AbilityConnectionSessionInfo info{"test", localInfo, peerInfo};
    ConnectOption options;
    options.needSendStream = true;
    constexpr int32_t loop = STRESS_LOOP_COUNT;
    for (int32_t i = 0; i < loop; ++i) {
        auto session = std::make_shared<AbilityConnectionSession>(STREAM_SESSION_ID_BASE + i,
            "serverSocketName", info, options);
        session->direction_ = CollabrateDirection::COLLABRATE_SOURCE;
        TransChannelInfo chInfo;
        chInfo.isConnected = false;
        session->transChannels_[TransChannelType::STREAM] = chInfo;
        EXPECT_EQ(session->ConnectStreamChannel(), ERR_OK);
        session.reset();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(STRESS_SETTLE_MS));
    DTEST_LOG << "AbilityConnectionSessionTest ConnectStreamChannel_DetachThread_Smoke_Test_001 end" << std::endl;
}
}
}
