/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "dms_mission_manager_test.h"

#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>

#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "nlohmann/json.hpp"
#include "string_ex.h"
#include "system_ability_definition.h"

#define private public
#define protected public
#include "distributed_sched_adapter.h"
#include "distributed_sched_proxy.h"
#include "distributed_sched_service.h"
#include "distributed_sched_test_util.h"
#include "distributed_sched_utils.h"
#include "dtbschedmgr_device_info_storage.h"
#include "mission/distributed_sched_mission_manager.h"
#include "mission/extension/dms_main_service_channel.h"
#include "mission/mission_constant.h"
#include "mission/snapshot.h"
#include "mission/snapshot_converter.h"
#include "mock_distributed_sched.h"
#include "test_log.h"
#undef private
#undef protected

using namespace std;
using namespace testing;
using namespace testing::ext;
using namespace OHOS::DistributedHardware;

namespace OHOS {
namespace DistributedSchedule {
namespace {
const std::string DEVICE_NAME = "DEVICE_PHONE_001";
const std::string DEVICE_ID = "123456789ABCD";
const std::string DEVICE_ID_FALSE = "123456789ABCDF";
const std::string DEVICE_ID_FALSE_AGAIN = "123456789ABCDFA";
const std::u16string U16DEVICE_ID = u"123456789ABCD";
const std::string BUNDLE_NAME = "ohos.test.test";
constexpr int32_t TASK_ID = 11;
constexpr size_t BYTESTREAM_LENGTH = 100;
constexpr uint8_t ONE_BYTE = '6';
const int32_t NUM_MISSIONS = 100;
const int32_t NORMAL_NUM_MISSIONS = 10;
constexpr int32_t REQUEST_CODE_ERR = 305;
constexpr int32_t MAX_WAIT_TIME = 1000;

constexpr uint8_t MINI_MALJPEG[] = {
    0xFF, 0xD8, 0xFF, 0xDB, 0x00, 0x43, 0x00, 0x08, 0x06, 0x06, 0x07, 0x06, 0x05, 0x08,
    0x07, 0x07, 0x07, 0x09, 0x09, 0x08, 0x0A, 0x0C, 0x14, 0x0D, 0x0C, 0x0B, 0x0B, 0x0C,
    0x19, 0x12, 0x13, 0x0F, 0x14, 0x1D, 0x1A, 0x1F, 0x1E, 0x1D, 0x1A, 0x1C, 0x1C, 0x20,
    0x24, 0x2E, 0x27, 0x20, 0x22, 0x2C, 0x23, 0x1C, 0x1C, 0x28, 0x37, 0x29, 0x2C, 0x30,
    0x31, 0x34, 0x34, 0x34, 0x1F, 0x27, 0x39, 0x3D, 0x38, 0x32, 0x3C, 0x2E, 0x33, 0x34,
    0x32, 0xFF, 0xC0, 0x00, 0x0B, 0x08, 0x00, 0x01, 0x00, 0x01, 0x01, 0x01, 0x11, 0x00,
    0xFF, 0xC4, 0x00, 0x14, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0xFF, 0xC4, 0x00, 0x14, 0x10, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xFF, 0xDA, 0x00, 0x08, 0x01, 0x01, 0x00, 0x00, 0x3F, 0x00, 0x37, 0xFF, 0xD9,
};
constexpr uint32_t MINI_MALJPEG_SIZE = static_cast<uint32_t>(sizeof(MINI_MALJPEG));

std::shared_ptr<Media::PixelMap> CreateUtPixelMap()
{
    Snapshot snapshot;
    auto pixelMap = snapshot.CreatePixelMap(MINI_MALJPEG, MINI_MALJPEG_SIZE);
    return std::shared_ptr<Media::PixelMap>(pixelMap.release());
}

class DmsMainServiceChannelFixtureDefaultMock : public DmsMainServiceChannel {
public:
    void SetFallbackLocalId(std::string id)
    {
        fallbackLocalId_ = std::move(id);
    }

    std::shared_ptr<DmsDeviceInfo> GetDeviceInfoById(const std::string& deviceId) override
    {
        return DtbschedmgrDeviceInfoStorage::GetInstance().GetDeviceInfoById(deviceId);
    }

    std::string GetUuidByNetworkId(const std::string& networkId) override
    {
        return DtbschedmgrDeviceInfoStorage::GetInstance().GetUuidByNetworkId(networkId);
    }

    bool GetLocalDeviceId(std::string& networkId) override
    {
        if (DtbschedmgrDeviceInfoStorage::GetInstance().GetLocalDeviceId(networkId)) {
            return true;
        }
        networkId = fallbackLocalId_;
        return true;
    }

    std::string GetNetworkIdByUuid(const std::string& uuid) override
    {
        return DtbschedmgrDeviceInfoStorage::GetInstance().GetNetworkIdByUuid(uuid);
    }

    int32_t GetLocalMissionInfos(int32_t numMissions, std::vector<DstbMissionInfo>& missionInfos) override
    {
        (void)numMissions;
        (void)missionInfos;
        return ERR_OK;
    }

    int32_t RegisterMissionListener(const sptr<AAFwk::IMissionListener>& listener) override
    {
        (void)listener;
        return ERR_OK;
    }

    int32_t UnRegisterMissionListener(const sptr<AAFwk::IMissionListener>& listener) override
    {
        (void)listener;
        return ERR_OK;
    }

    int32_t GetLocalMissionSnapshotInfo(const std::string& networkId, int32_t missionId,
        AAFwk::MissionSnapshot& missionSnapshot) override
    {
        (void)networkId;
        (void)missionId;
        auto pixelMap = CreateUtPixelMap();
        if (pixelMap != nullptr) {
            missionSnapshot.snapshot = pixelMap;
            return ERR_OK;
        }
        return -1;
    }

    std::string GetAnonymStr(const std::string& value) override
    {
        return value;
    }

private:
    std::string fallbackLocalId_ { "ut-mission-local-device" };
};

std::shared_ptr<DmsMainServiceChannel> g_utMainServiceChannelHolder;

std::shared_ptr<DmsMainServiceChannel> CreateUtMainServiceChannelDefault()
{
    auto channel = std::make_shared<DmsMainServiceChannelFixtureDefaultMock>();
    std::string localDeviceId;
    if (!DtbschedmgrDeviceInfoStorage::GetInstance().GetLocalDeviceId(localDeviceId)) {
        std::static_pointer_cast<DmsMainServiceChannelFixtureDefaultMock>(channel)
            ->SetFallbackLocalId("ut-mission-local-device");
    }
    return channel;
}

void BindUtMainServiceChannel(const std::shared_ptr<DmsMainServiceChannel>& channel)
{
    g_utMainServiceChannelHolder = channel;
    auto& mgr = DistributedSchedMissionManager::GetInstance();
    mgr.mainServiceChannel_ = channel;
    mgr.SetMainServiceChannel(channel);
}

/* Set by the branch coverage cases added at the end of this file only, so TearDown can restore the
   singleton state for them without changing the behaviour of the legacy cases. */
bool g_missionUtCaseActive = false;
void ResetMissionManagerUtState();
}

bool DMSMissionManagerTest::isCaseDone_ = false;
std::mutex DMSMissionManagerTest::caseDoneLock_;
std::condition_variable DMSMissionManagerTest::caseDoneCondition_;

int32_t DistributedSchedAdapter::GetLocalMissionSnapshotInfo(const std::string& networkId, int32_t missionId,
    AAFwk::MissionSnapshot& missionSnapshot)
{
    return -1;
}

void DMSMissionManagerTest::SetUpTestCase()
{
    if (!DistributedSchedUtil::LoadDistributedSchedService()) {
        DTEST_LOG << "DMSMissionManagerTest::SetUpTestCase LoadDistributedSchedService failed" << std::endl;
    }
    const std::string pkgName = "DBinderBus_" + std::to_string(getprocpid());
    std::shared_ptr<DmInitCallback> initCallback_ = std::make_shared<DeviceInitCallBack>();
    DeviceManager::GetInstance().InitDeviceManager(pkgName, initCallback_);
}

void DMSMissionManagerTest::TearDownTestCase()
{
}

void DMSMissionManagerTest::SetUp()
{
    DistributedSchedUtil::MockPermission();
    savedMainServiceChannelForUt_ = DistributedSchedMissionManager::GetInstance().GetMainServiceChannel();

    std::string localDeviceId;
    if (DtbschedmgrDeviceInfoStorage::GetInstance().GetLocalDeviceId(localDeviceId)) {
        localDeviceId_ = localDeviceId;
        u16localDeviceId_ = Str8ToStr16(localDeviceId);
        DTEST_LOG << "getLocalDevicesId ok" << std::endl;
    } else {
        DTEST_LOG << "getLocalDevicesId failed!" << std::endl;
        localDeviceId_.clear();
        u16localDeviceId_.clear();
    }
    BindUtMainServiceChannel(CreateUtMainServiceChannelDefault());
}

void DMSMissionManagerTest::TearDown()
{
    g_utMainServiceChannelHolder = savedMainServiceChannelForUt_;
    auto& mgr = DistributedSchedMissionManager::GetInstance();
    mgr.mainServiceChannel_ = savedMainServiceChannelForUt_;
    mgr.SetMainServiceChannel(savedMainServiceChannelForUt_);
    savedMainServiceChannelForUt_.reset();
    if (g_missionUtCaseActive) {
        g_missionUtCaseActive = false;
        ResetMissionManagerUtState();
    }
}

void DMSMissionManagerTest::DeviceInitCallBack::OnRemoteDied()
{
}

sptr<IDistributedSched> DMSMissionManagerTest::GetDms()
{
    if (proxy_ != nullptr) {
        return proxy_;
    }

    /* LoadDistributedSchedService() stores binder in DistributedSchedUtil::remote_. Prefer it over
       GetSystemAbility, which may differ or malfunction in unittest process and crash in iface_cast. */
    sptr<IRemoteObject> remoteObject;
    {
        std::lock_guard<std::mutex> guard(DistributedSchedUtil::remoteMutex_);
        remoteObject = DistributedSchedUtil::remote_;
    }
    if (remoteObject != nullptr) {
        proxy_ = iface_cast<IDistributedSched>(remoteObject);
        if (proxy_ == nullptr) {
            DTEST_LOG << "DMSMissionManagerTest iface_cast from LoadDistributedSchedService remote failed" << std::endl;
        }
        return proxy_;
    }

    auto sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sm == nullptr) {
        DTEST_LOG << "DMSMissionManagerTest sm is nullptr" << std::endl;
        return nullptr;
    }
    DTEST_LOG << "DMSMissionManagerTest sm ok, fallback GetSystemAbility" << std::endl;
    auto distributedObject = sm->GetSystemAbility(DISTRIBUTED_SCHED_SA_ID);
    if (distributedObject == nullptr) {
        DTEST_LOG << "DMSMissionManagerTest GetSystemAbility DISTRIBUTED_SCHED_SA_ID is nullptr" << std::endl;
        return nullptr;
    }
    proxy_ = iface_cast<IDistributedSched>(distributedObject);
    if (proxy_ == nullptr) {
        DTEST_LOG << "DMSMissionManagerTest DistributedSched iface_cast is nullptr" << std::endl;
    }
    return proxy_;
}

void RemoteMissionListenerTest::NotifyMissionsChanged(const std::string& deviceId)
{
    DTEST_LOG << "NotifyMissionsChanged" << std::endl;
}

void RemoteMissionListenerTest::NotifySnapshot(const std::string& deviceId, int32_t missionId)
{
    DTEST_LOG << "NotifySnapshot" << std::endl;
}

void RemoteMissionListenerTest::NotifyNetDisconnect(const std::string& deviceId, int32_t state)
{
    DTEST_LOG << "NotifyNetDisconnect" << std::endl;
}

/**
 * @tc.name: testGetRemoteMissionInfo001
 * @tc.desc: get remote mission info
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testGetRemoteMissionInfo001, TestSize.Level1)
{
    std::vector<AAFwk::MissionInfo> infos;
    auto ret = DistributedSchedMissionManager::GetInstance().GetMissionInfos(DEVICE_ID, 0, infos);
    EXPECT_TRUE(ret != ERR_NONE);

    ret = DistributedSchedMissionManager::GetInstance().GetMissionInfos(localDeviceId_, 0, infos);
    EXPECT_TRUE(ret != ERR_NONE);

    ret = DistributedSchedMissionManager::GetInstance().GetMissionInfos("", 0, infos);
    EXPECT_TRUE(ret != ERR_NONE);

    ret = DistributedSchedMissionManager::GetInstance().GetMissionInfos(DEVICE_ID, -1, infos);
    EXPECT_TRUE(ret != ERR_NONE);

    ret = DistributedSchedMissionManager::GetInstance().GetMissionInfos(localDeviceId_, -1, infos);
    EXPECT_TRUE(ret != ERR_NONE);
}

/**
 * @tc.name: testGetRemoteMissionInfo002
 * @tc.desc: get remote mission info
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testGetRemoteMissionInfo002, TestSize.Level1)
{
    sptr<IDistributedSched> proxy = GetDms();
    ASSERT_NE(nullptr, proxy);
    std::vector<AAFwk::MissionInfo> infos;
    auto ret = proxy->GetMissionInfos(DEVICE_ID, 0, infos);
    EXPECT_TRUE(ret != ERR_NONE);

    ret = proxy->GetMissionInfos(localDeviceId_, 0, infos);
    EXPECT_TRUE(ret != ERR_NONE);

    ret = proxy->GetMissionInfos("", 0, infos);
    EXPECT_TRUE(ret != ERR_NONE);

    ret = proxy->GetMissionInfos(DEVICE_ID, -1, infos);
    EXPECT_TRUE(ret != ERR_NONE);

    ret = proxy->GetMissionInfos(localDeviceId_, -1, infos);
    EXPECT_TRUE(ret != ERR_NONE);
}

/**
 * @tc.name: testStartSyncRemoteMissions001
 * @tc.desc: prepare and sync missions from remote
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testStartSyncRemoteMissions001, TestSize.Level1)
{
    sptr<IDistributedSched> proxy = GetDms();
    ASSERT_NE(nullptr, proxy);
    int32_t callingUid = 0;
    auto ret = proxy->StartSyncRemoteMissions(DEVICE_ID, false, 0, callingUid);
    EXPECT_NE(ret, ERR_NONE);
}

/**
 * @tc.name: testStartSyncRemoteMissions002
 * @tc.desc: prepare and sync missions from remote
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testStartSyncRemoteMissions002, TestSize.Level1)
{
    sptr<IDistributedSched> proxy = GetDms();
    ASSERT_NE(nullptr, proxy);
    int32_t callingUid = 0;
    auto ret = proxy->StartSyncRemoteMissions("", false, 0, callingUid);
    EXPECT_NE(ret, ERR_NONE);
}

/**
 * @tc.name: testStartSyncRemoteMissions003
 * @tc.desc: prepare and sync missions from remote
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testStartSyncRemoteMissions003, TestSize.Level1)
{
    sptr<IDistributedSched> proxy = GetDms();
    ASSERT_NE(nullptr, proxy);
    int32_t callingUid = 0;
    auto ret = proxy->StartSyncRemoteMissions(localDeviceId_, false, 0, callingUid);
    EXPECT_NE(ret, ERR_NONE);
}

/**
 * @tc.name: testStartSyncRemoteMissions004
 * @tc.desc: prepare and sync missions from remote
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testStartSyncRemoteMissions004, TestSize.Level3)
{
    DTEST_LOG << "testStartSyncRemoteMissions004 begin" << std::endl;
    auto deviceInfo = std::make_shared<DmsDeviceInfo>("", 0, "");
    DtbschedmgrDeviceInfoStorage::GetInstance().remoteDevices_[DEVICE_ID] = deviceInfo;
    auto ret = DistributedSchedMissionManager::GetInstance().StartSyncRemoteMissions(DEVICE_ID, false, 0);
    EXPECT_EQ(ret, ERR_NONE);
    DTEST_LOG << "testStartSyncRemoteMissions004 end" << std::endl;
}
/**
 * @tc.name: testStartSyncRemoteMissions005
 * @tc.desc: prepare and sync missions from remote
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testStartSyncRemoteMissions005, TestSize.Level3)
{
    DTEST_LOG << "testStartSyncRemoteMissions005 begin" << std::endl;
    auto ret = DistributedSchedMissionManager::GetInstance().StartSyncRemoteMissions("", false, 0);
    EXPECT_EQ(ret, INVALID_PARAMETERS_ERR);
    DTEST_LOG << "testStartSyncRemoteMissions005 end" << std::endl;
}

/**
 * @tc.name: testStartSyncRemoteMissions006
 * @tc.desc: prepare and sync missions from remote
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testStartSyncRemoteMissions006, TestSize.Level3)
{
    DTEST_LOG << "testStartSyncRemoteMissions006 begin" << std::endl;
    auto ret = DistributedSchedMissionManager::GetInstance().StartSyncRemoteMissions(localDeviceId_, false, 0);
    EXPECT_NE(ret, ERR_NONE);
    DTEST_LOG << "testStartSyncRemoteMissions006 end" << std::endl;
}

/**
 * @tc.name: testStartSyncRemoteMissions007
 * @tc.desc: prepare and sync missions from remote
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testStartSyncRemoteMissions007, TestSize.Level3)
{
    DTEST_LOG << "testStartSyncRemoteMissions007 begin" << std::endl;
    auto deviceInfo = std::make_shared<DmsDeviceInfo>("", 0, "");
    DtbschedmgrDeviceInfoStorage::GetInstance().remoteDevices_[DEVICE_ID] = deviceInfo;
    auto ret = DistributedSchedMissionManager::GetInstance().StartSyncRemoteMissions(DEVICE_ID, true, 0);
    EXPECT_EQ(ret, ERR_NONE);
    DTEST_LOG << "testStartSyncRemoteMissions007 end" << std::endl;
}

/**
 * @tc.name: testStartSyncRemoteMissions008
 * @tc.desc: prepare and sync missions from remote
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testStartSyncRemoteMissions008, TestSize.Level3)
{
    DTEST_LOG << "testStartSyncRemoteMissions008 begin" << std::endl;
    auto ret = DistributedSchedMissionManager::GetInstance().StartSyncRemoteMissions("", true, 0);
    EXPECT_EQ(ret, INVALID_PARAMETERS_ERR);
    DTEST_LOG << "testStartSyncRemoteMissions008 end" << std::endl;
}

/**
 * @tc.name: testStartSyncRemoteMissions009
 * @tc.desc: prepare and sync missions from remote
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testStartSyncRemoteMissions009, TestSize.Level3)
{
    DTEST_LOG << "testStartSyncRemoteMissions009 begin" << std::endl;
    auto ret = DistributedSchedMissionManager::GetInstance().StartSyncRemoteMissions(localDeviceId_, true, 0);
    EXPECT_NE(ret, ERR_NONE);
    DTEST_LOG << "testStartSyncRemoteMissions009 end" << std::endl;
}

/**
 * @tc.name: testStartSyncRemoteMissions010
 * @tc.desc: prepare and sync missions from remote
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testStartSyncRemoteMissions010, TestSize.Level3)
{
    DTEST_LOG << "testStartSyncRemoteMissions010 begin" << std::endl;
    auto deviceInfo = std::make_shared<DmsDeviceInfo>("", 0, "");
    DtbschedmgrDeviceInfoStorage::GetInstance().remoteDevices_[DEVICE_ID] = deviceInfo;
    auto ret = DistributedSchedMissionManager::GetInstance().StartSyncRemoteMissions(DEVICE_ID, true, 1);
    EXPECT_EQ(ret, ERR_NONE);
    DTEST_LOG << "testStartSyncRemoteMissions010 end" << std::endl;
}

/**
 * @tc.name: testStartSyncRemoteMissions011
 * @tc.desc: prepare and sync missions from remote
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testStartSyncRemoteMissions011, TestSize.Level3)
{
    DTEST_LOG << "testStartSyncRemoteMissions011 begin" << std::endl;
    auto ret = DistributedSchedMissionManager::GetInstance().StartSyncRemoteMissions("", true, 1);
    EXPECT_EQ(ret, INVALID_PARAMETERS_ERR);
    DTEST_LOG << "testStartSyncRemoteMissions011 end" << std::endl;
}

/**
 * @tc.name: testStartSyncRemoteMissions012
 * @tc.desc: prepare and sync missions from remote
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testStartSyncRemoteMissions012, TestSize.Level3)
{
    DTEST_LOG << "testStartSyncRemoteMissions012 begin" << std::endl;
    auto ret = DistributedSchedMissionManager::GetInstance().StartSyncRemoteMissions(localDeviceId_, true, 1);
    EXPECT_NE(ret, ERR_NONE);
    DTEST_LOG << "testStartSyncRemoteMissions012 end" << std::endl;
}

/**
 * @tc.name: testStartSyncRemoteMissions013
 * @tc.desc: prepare and sync missions from remote
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testStartSyncRemoteMissions013, TestSize.Level3)
{
    DTEST_LOG << "testStartSyncRemoteMissions013 begin" << std::endl;
    auto ret = DistributedSchedMissionManager::GetInstance().StartSyncRemoteMissions(DEVICE_ID, localDeviceId_);
    EXPECT_EQ(ret, ERR_NONE);
    DTEST_LOG << "testStartSyncRemoteMissions013 end" << std::endl;
}

/**
 * @tc.name: testStartSyncRemoteMissions014
 * @tc.desc: prepare and sync missions from remote
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testStartSyncRemoteMissions014, TestSize.Level3)
{
    DTEST_LOG << "testStartSyncRemoteMissions014 begin" << std::endl;
    BindUtMainServiceChannel(CreateUtMainServiceChannelDefault());
    ASSERT_NE(DistributedSchedMissionManager::GetInstance().GetMainServiceChannel(), nullptr);

    sptr<IDistributedSched> proxy = new MockDistributedSched();
    ASSERT_NE(proxy, nullptr);
    auto ret = DistributedSchedMissionManager::GetInstance().StartSyncRemoteMissions(DEVICE_ID, proxy, 0);
    EXPECT_EQ(ret, ERR_NONE);
    DTEST_LOG << "testStartSyncRemoteMissions014 end" << std::endl;
}

/**
 * @tc.name: testStartSyncRemoteMissions017
 * @tc.desc: prepare and sync missions from remote
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testStartSyncRemoteMissions017, TestSize.Level3)
{
    DTEST_LOG << "testStartSyncRemoteMissions017 begin" << std::endl;
    auto ret = DistributedSchedMissionManager::GetInstance().StartSyncRemoteMissions(DEVICE_ID,
        sptr<IDistributedSched>(nullptr), 0);
    EXPECT_EQ(ret, INVALID_PARAMETERS_ERR);
    DTEST_LOG << "testStartSyncRemoteMissions017 end" << std::endl;
}

/**
 * @tc.name: testNeedSyncDevice001
 * @tc.desc: need sync device
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testNeedSyncDevice001, TestSize.Level3)
{
    DTEST_LOG << "testCleanMissionResources001 begin" << std::endl;
    bool ret = DistributedSchedMissionManager::GetInstance().NeedSyncDevice(DEVICE_ID);
    EXPECT_FALSE(ret);
    DTEST_LOG << "testNeedSyncDevice001 end" << std::endl;
}

/**
 * @tc.name: testNeedSyncDevice002
 * @tc.desc: need sync device
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testNeedSyncDevice002, TestSize.Level3)
{
    DTEST_LOG << "testNeedSyncDevice002 begin" << std::endl;
    remoteSyncDeviceSet_.emplace(DEVICE_ID);
    bool ret = DistributedSchedMissionManager::GetInstance().NeedSyncDevice(DEVICE_ID);
    EXPECT_FALSE(ret);
    DTEST_LOG << "testNeedSyncDevice002 end" << std::endl;
}

/**
 * @tc.name: testHasSyncListener001
 * @tc.desc: need sync device
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testHasSyncListener001, TestSize.Level3)
{
    DTEST_LOG << "testHasSyncListener001 begin" << std::endl;
    DistributedSchedMissionManager::GetInstance().CleanMissionResources(DEVICE_ID);
    bool ret = DistributedSchedMissionManager::GetInstance().HasSyncListener(DEVICE_ID);
    EXPECT_FALSE(ret);
    DTEST_LOG << "testHasSyncListener001 end" << std::endl;
}

/**
 * @tc.name: testStopSyncRemoteMissions001
 * @tc.desc: stop sync missions from remote with fake deviceId
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testStopSyncRemoteMissions001, TestSize.Level1)
{
    sptr<IDistributedSched> proxy = GetDms();
    ASSERT_NE(nullptr, proxy);
    int32_t callingUid = 0;
    auto ret = proxy->StopSyncRemoteMissions(DEVICE_ID, callingUid);
    EXPECT_NE(ret, ERR_NONE);
}

/**
 * @tc.name: testStopSyncRemoteMissions002
 * @tc.desc: stop sync missions from remote with local deviceId
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testStopSyncRemoteMissions002, TestSize.Level1)
{
    sptr<IDistributedSched> proxy = GetDms();
    ASSERT_NE(nullptr, proxy);
    int32_t callingUid = 0;
    auto ret = proxy->StopSyncRemoteMissions(localDeviceId_, callingUid);
    EXPECT_NE(ret, ERR_NONE);
}

/**
 * @tc.name: testStopSyncRemoteMissions003
 * @tc.desc: stop sync missions from remote with empty deviceId
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testStopSyncRemoteMissions003, TestSize.Level1)
{
    sptr<IDistributedSched> proxy = GetDms();
    ASSERT_NE(nullptr, proxy);
    int32_t callingUid = 0;
    auto ret = proxy->StopSyncRemoteMissions("", callingUid);
    EXPECT_NE(ret, ERR_NONE);
}

/**
 * @tc.name: testStopSyncRemoteMissions004
 * @tc.desc: stop sync missions from remote with empty deviceId
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testStopSyncRemoteMissions004, TestSize.Level3)
{
    DTEST_LOG << "testStopSyncRemoteMissions004 begin" << std::endl;
    auto ret = DistributedSchedMissionManager::GetInstance().StopSyncRemoteMissions("", false, true);
    EXPECT_NE(ret, ERR_NONE);
    DTEST_LOG << "testStopSyncRemoteMissions004 end" << std::endl;
}

/**
 * @tc.name: testStopSyncRemoteMissions005
 * @tc.desc: stop sync missions from remote with local deviceId
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testStopSyncRemoteMissions005, TestSize.Level3)
{
    DTEST_LOG << "testStopSyncRemoteMissions005 begin" << std::endl;
    auto ret = DistributedSchedMissionManager::GetInstance().StopSyncRemoteMissions(localDeviceId_, false, true);
    EXPECT_NE(ret, ERR_NONE);
    DTEST_LOG << "testStopSyncRemoteMissions005 end" << std::endl;
}

/**
 * @tc.name: testStopSyncRemoteMissions006
 * @tc.desc: stop sync missions from remote with deviceId
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testStopSyncRemoteMissions006, TestSize.Level3)
{
    DTEST_LOG << "testStopSyncRemoteMissions006 begin" << std::endl;
    auto ret = DistributedSchedMissionManager::GetInstance().StopSyncRemoteMissions(DEVICE_ID, false, true);
    EXPECT_NE(ret, ERR_NONE);
    DTEST_LOG << "testStopSyncRemoteMissions006 end" << std::endl;
}

/**
 * @tc.name: testStopSyncRemoteMissions007
 * @tc.desc: stop sync missions from remote with empty deviceId
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testStopSyncRemoteMissions007, TestSize.Level3)
{
    DTEST_LOG << "testStopSyncRemoteMissions007 begin" << std::endl;
    auto ret = DistributedSchedMissionManager::GetInstance().StopSyncRemoteMissions("", true, true);
    EXPECT_EQ(ret, ERR_NONE);
    DTEST_LOG << "testStopSyncRemoteMissions007 end" << std::endl;
}

/**
 * @tc.name: testStopSyncRemoteMissions008
 * @tc.desc: stop sync missions from remote with local deviceId
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testStopSyncRemoteMissions008, TestSize.Level3)
{
    DTEST_LOG << "testStopSyncRemoteMissions008 begin" << std::endl;
    auto ret = DistributedSchedMissionManager::GetInstance().StopSyncRemoteMissions(localDeviceId_, true, true);
    EXPECT_EQ(ret, ERR_NONE);
    DTEST_LOG << "testStopSyncRemoteMissions008 end" << std::endl;
}

/**
 * @tc.name: testStopSyncRemoteMissions009
 * @tc.desc: stop sync missions from remote with deviceId
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testStopSyncRemoteMissions009, TestSize.Level3)
{
    DTEST_LOG << "testStopSyncRemoteMissions009 begin" << std::endl;
    auto ret = DistributedSchedMissionManager::GetInstance().StopSyncRemoteMissions(DEVICE_ID, true, true);
    EXPECT_EQ(ret, ERR_NONE);
    DTEST_LOG << "testStopSyncRemoteMissions009 end" << std::endl;
}

/**
 * @tc.name: testStopSyncRemoteMissions010
 * @tc.desc: stop sync missions from remote with empty deviceId
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testStopSyncRemoteMissions010, TestSize.Level3)
{
    DTEST_LOG << "testStopSyncRemoteMissions010 begin" << std::endl;
    auto ret = DistributedSchedMissionManager::GetInstance().StopSyncRemoteMissions("", true, false);
    EXPECT_EQ(ret, ERR_NONE);
    DTEST_LOG << "testStopSyncRemoteMissions010 end" << std::endl;
}

/**
 * @tc.name: testStopSyncRemoteMissions011
 * @tc.desc: stop sync missions from remote with local deviceId
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testStopSyncRemoteMissions011, TestSize.Level3)
{
    DTEST_LOG << "testStopSyncRemoteMissions011 begin" << std::endl;
    auto ret = DistributedSchedMissionManager::GetInstance().StopSyncRemoteMissions(localDeviceId_, true, false);
    EXPECT_EQ(ret, ERR_NONE);
    DTEST_LOG << "testStopSyncRemoteMissions011 end" << std::endl;
}

/**
 * @tc.name: testStopSyncRemoteMissions012
 * @tc.desc: stop sync missions from remote with deviceId
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testStopSyncRemoteMissions012, TestSize.Level3)
{
    DTEST_LOG << "testStopSyncRemoteMissions012 begin" << std::endl;
    auto ret = DistributedSchedMissionManager::GetInstance().StopSyncRemoteMissions(DEVICE_ID, true, false);
    EXPECT_EQ(ret, ERR_NONE);
    DTEST_LOG << "testStopSyncRemoteMissions012 end" << std::endl;
}

/**
 * @tc.name: testRegisterMissionListener001
 * @tc.desc: register mission listener
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testRegisterMissionListener001, TestSize.Level1)
{
    sptr<IDistributedSched> proxy = GetDms();
    ASSERT_NE(nullptr, proxy);
    int32_t callingUid = 0;
    auto ret = proxy->RegisterMissionListener(U16DEVICE_ID, nullptr, callingUid);
    EXPECT_TRUE(ret != ERR_NONE);

    ret = proxy->RegisterMissionListener(u16localDeviceId_, nullptr, callingUid);
    EXPECT_TRUE(ret != ERR_NONE);

    ret = proxy->RegisterMissionListener(u"", nullptr, callingUid);
    EXPECT_TRUE(ret != ERR_NONE);
}

/**
 * @tc.name: testRegisterMissionListener002
 * @tc.desc: register mission listener
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testRegisterMissionListener002, TestSize.Level1)
{
    sptr<IDistributedSched> proxy = GetDms();
    ASSERT_NE(nullptr, proxy);
    sptr<IRemoteObject> listener(new RemoteMissionListenerTest());
    int32_t callingUid = 0;
    auto ret = proxy->RegisterMissionListener(U16DEVICE_ID, listener, callingUid);
    EXPECT_TRUE(ret != ERR_NONE);

    ret = proxy->RegisterMissionListener(u16localDeviceId_, listener, callingUid);
    EXPECT_TRUE(ret != ERR_NONE);

    ret = proxy->RegisterMissionListener(u"", listener, callingUid);
    EXPECT_TRUE(ret != ERR_NONE);

    ret = proxy->UnRegisterMissionListener(U16DEVICE_ID, listener);
    EXPECT_TRUE(ret != ERR_NONE);

    ret = proxy->UnRegisterMissionListener(u16localDeviceId_, listener);
    EXPECT_TRUE(ret != ERR_NONE);

    ret = proxy->UnRegisterMissionListener(u"", listener);
    EXPECT_TRUE(ret != ERR_NONE);
}

/**
 * @tc.name: testRegisterMissionListener003
 * @tc.desc: test register mission listener
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testRegisterMissionListener003, TestSize.Level3)
{
    DTEST_LOG << "testRegisterMissionListener003 begin" << std::endl;
    sptr<IRemoteObject> listener(new RemoteMissionListenerTest());
    auto ret = DistributedSchedMissionManager::GetInstance().RegisterMissionListener(u"", listener);
    EXPECT_EQ(ret, INVALID_PARAMETERS_ERR);
    DTEST_LOG << "testRegisterMissionListener003 end" << std::endl;
}

/**
 * @tc.name: testRegisterMissionListener004
 * @tc.desc: test register mission listener
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testRegisterMissionListener004, TestSize.Level3)
{
    DTEST_LOG << "testRegisterMissionListener004 begin" << std::endl;
    sptr<IRemoteObject> listener(new RemoteMissionListenerTest());
    DtbschedmgrDeviceInfoStorage::GetInstance().uuidNetworkIdMap_["1234567"] = DEVICE_ID;
    auto ret = DistributedSchedMissionManager::GetInstance().RegisterMissionListener(U16DEVICE_ID, listener);
    EXPECT_EQ(ret, ERR_NONE);
    DTEST_LOG << "testRegisterMissionListener004 end" << std::endl;
}

/**
 * @tc.name: testRegisterMissionListener005
 * @tc.desc: test register mission listener
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testRegisterMissionListener005, TestSize.Level3)
{
    DTEST_LOG << "testRegisterMissionListener005 begin" << std::endl;
    sptr<IRemoteObject> listener = nullptr;
    auto ret = DistributedSchedMissionManager::GetInstance().RegisterMissionListener(u"", listener);
    EXPECT_EQ(ret, INVALID_PARAMETERS_ERR);
    DTEST_LOG << "testRegisterMissionListener005 end" << std::endl;
}

/**
 * @tc.name: testRegisterMissionListener006
 * @tc.desc: test register mission listener
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testRegisterMissionListener006, TestSize.Level3)
{
    DTEST_LOG << "testRegisterMissionListener006 begin" << std::endl;
    sptr<IRemoteObject> listener = nullptr;
    auto ret = DistributedSchedMissionManager::GetInstance().RegisterMissionListener(U16DEVICE_ID, listener);
    EXPECT_EQ(ret, INVALID_PARAMETERS_ERR);
    DTEST_LOG << "testRegisterMissionListener006 end" << std::endl;
}

/**
 * @tc.name: testRegisterMissionListener007
 * @tc.desc: test register mission listener
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testRegisterMissionListener007, TestSize.Level3)
{
    DTEST_LOG << "testRegisterMissionListener007 begin" << std::endl;
    u16string deviceId = Str8ToStr16(localDeviceId_);
    sptr<IRemoteObject> listener = nullptr;
    auto ret = DistributedSchedMissionManager::GetInstance().RegisterMissionListener(deviceId, listener);
    EXPECT_EQ(ret, INVALID_PARAMETERS_ERR);
    DTEST_LOG << "testRegisterMissionListener007 end" << std::endl;
}

/**
 * @tc.name: testRegisterMissionListener008
 * @tc.desc: test register mission listener
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testRegisterMissionListener008, TestSize.Level3)
{
    DTEST_LOG << "testRegisterMissionListener008 begin" << std::endl;
    u16string deviceId = Str8ToStr16(localDeviceId_);
    sptr<IRemoteObject> listener(new RemoteMissionListenerTest());
    auto ret = DistributedSchedMissionManager::GetInstance().RegisterMissionListener(deviceId, listener);
    EXPECT_TRUE(ret == INVALID_PARAMETERS_ERR);
    DTEST_LOG << "testRegisterMissionListener008 end" << std::endl;
}

/**
 * @tc.name: testUnRegisterMissionListener001
 * @tc.desc: register mission listener
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testUnRegisterMissionListener001, TestSize.Level1)
{
    sptr<IDistributedSched> proxy = GetDms();
    ASSERT_NE(nullptr, proxy);
    auto ret = proxy->UnRegisterMissionListener(U16DEVICE_ID, nullptr);
    EXPECT_TRUE(ret != ERR_NONE);

    ret = proxy->UnRegisterMissionListener(u16localDeviceId_, nullptr);
    EXPECT_TRUE(ret != ERR_NONE);

    ret = proxy->UnRegisterMissionListener(u"", nullptr);
    EXPECT_TRUE(ret != ERR_NONE);
}

/**
 * @tc.name: testUnRegisterMissionListener002
 * @tc.desc: register mission listener
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testUnRegisterMissionListener002, TestSize.Level3)
{
    DTEST_LOG << "testUnRegisterMissionListener002 begin" << std::endl;
    sptr<IRemoteObject> listener(new RemoteMissionListenerTest());
    auto ret = DistributedSchedMissionManager::GetInstance().UnRegisterMissionListener(U16DEVICE_ID, listener);
    EXPECT_EQ(ret, ERR_NONE);
    DTEST_LOG << "testUnRegisterMissionListener002 end" << std::endl;
}

/**
 * @tc.name: testUnRegisterMissionListener003
 * @tc.desc: register mission listener
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testUnRegisterMissionListener003, TestSize.Level3)
{
    DTEST_LOG << "testUnRegisterMissionListener003 begin" << std::endl;
    auto ret = DistributedSchedMissionManager::GetInstance().UnRegisterMissionListener(U16DEVICE_ID, nullptr);
    EXPECT_EQ(ret, INVALID_PARAMETERS_ERR);
    DTEST_LOG << "testUnRegisterMissionListener003 end" << std::endl;
}

/**
 * @tc.name: testUnRegisterMissionListener004
 * @tc.desc: register mission listener
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testUnRegisterMissionListener004, TestSize.Level3)
{
    DTEST_LOG << "testUnRegisterMissionListener004 begin" << std::endl;
    u16string deviceId = Str8ToStr16(localDeviceId_);
    auto ret = DistributedSchedMissionManager::GetInstance().UnRegisterMissionListener(deviceId, nullptr);
    EXPECT_EQ(ret, INVALID_PARAMETERS_ERR);
    DTEST_LOG << "testUnRegisterMissionListener004 end" << std::endl;
}

/**
 * @tc.name: testUnRegisterMissionListener005
 * @tc.desc: register mission listener
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testUnRegisterMissionListener005, TestSize.Level3)
{
    DTEST_LOG << "testUnRegisterMissionListener005 begin" << std::endl;
    u16string deviceId = Str8ToStr16(localDeviceId_);
    sptr<IRemoteObject> listener(new RemoteMissionListenerTest());
    auto ret = DistributedSchedMissionManager::GetInstance().UnRegisterMissionListener(deviceId, listener);
    EXPECT_EQ(ret, INVALID_PARAMETERS_ERR);
    DTEST_LOG << "testUnRegisterMissionListener005 end" << std::endl;
}

/**
 * @tc.name: testIsDeviceIdValidated001
 * @tc.desc: judging whether the deviceId is valid
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testIsDeviceIdValidated001, TestSize.Level1)
{
    std::string emptyDeviceId = "";
    auto ret = DistributedSchedMissionManager::GetInstance().IsDeviceIdValidated(emptyDeviceId);
    EXPECT_FALSE(ret);

    ret = DistributedSchedMissionManager::GetInstance().IsDeviceIdValidated(DEVICE_ID_FALSE);
    EXPECT_FALSE(ret);

    ret = DistributedSchedMissionManager::GetInstance().IsDeviceIdValidated(DEVICE_ID_FALSE_AGAIN);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: testGetMissionInfos001
 * @tc.desc: get missions
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testGetMissionInfos001, TestSize.Level1)
{
    std::string deviceid = DEVICE_ID;
    int32_t numMissions = NUM_MISSIONS;
    std::vector<AAFwk::MissionInfo> missionInfos;

    auto ret = DistributedSchedMissionManager::GetInstance().GetMissionInfos(deviceid, numMissions, missionInfos);
    EXPECT_TRUE(ret != ERR_NONE);
}

/**
 * @tc.name: testGetMissionInfos002
 * @tc.desc: get missions
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testGetMissionInfos002, TestSize.Level3)
{
    DTEST_LOG << "testGetMissionInfos002 begin" << std::endl;
    std::string deviceid = localDeviceId_;
    int32_t numMissions = NUM_MISSIONS;
    std::vector<AAFwk::MissionInfo> missionInfos;

    auto ret = DistributedSchedMissionManager::GetInstance().GetMissionInfos(deviceid, numMissions, missionInfos);
    EXPECT_TRUE(ret != ERR_NONE);
    DTEST_LOG << "testGetMissionInfos002 end" << std::endl;
}

/**
 * @tc.name: testGetMissionInfos003
 * @tc.desc: get missions
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testGetMissionInfos003, TestSize.Level3)
{
    DTEST_LOG << "testGetMissionInfos003 begin" << std::endl;
    std::string deviceid = "";
    int32_t numMissions = NUM_MISSIONS;
    std::vector<AAFwk::MissionInfo> missionInfos;

    auto ret = DistributedSchedMissionManager::GetInstance().GetMissionInfos(deviceid, numMissions, missionInfos);
    EXPECT_TRUE(ret != ERR_NONE);
    DTEST_LOG << "testGetMissionInfos003 end" << std::endl;
}

/**
 * @tc.name: testGetMissionInfos004
 * @tc.desc: get missions
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testGetMissionInfos004, TestSize.Level3)
{
    DTEST_LOG << "testGetMissionInfos004 begin" << std::endl;
    std::string deviceid = DEVICE_ID;
    int32_t numMissions = -1;
    std::vector<AAFwk::MissionInfo> missionInfos;
    auto ret = DistributedSchedMissionManager::GetInstance().GetMissionInfos(deviceid, numMissions, missionInfos);
    EXPECT_TRUE(ret == INVALID_PARAMETERS_ERR);
    DTEST_LOG << "testGetMissionInfos004 end" << std::endl;
}

/**
 * @tc.name: testGetMissionInfos005
 * @tc.desc: get missions
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testGetMissionInfos005, TestSize.Level3)
{
    DTEST_LOG << "testGetMissionInfos005 begin" << std::endl;
    std::string deviceid = "";
    int32_t numMissions = NUM_MISSIONS;
    std::vector<AAFwk::MissionInfo> missionInfos;
    auto ret = DistributedSchedMissionManager::GetInstance().GetMissionInfos(deviceid, numMissions, missionInfos);
    EXPECT_TRUE(ret == INVALID_PARAMETERS_ERR);
    DTEST_LOG << "testGetMissionInfos005 end" << std::endl;
}

/**
 * @tc.name: testGetMissionInfos006
 * @tc.desc: get missions
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testGetMissionInfos006, TestSize.Level3)
{
    DTEST_LOG << "testGetMissionInfos006 begin" << std::endl;
    std::string deviceid = "";
    int32_t numMissions = 0;
    std::vector<AAFwk::MissionInfo> missionInfos;
    auto ret = DistributedSchedMissionManager::GetInstance().GetMissionInfos(deviceid, numMissions, missionInfos);
    EXPECT_TRUE(ret == INVALID_PARAMETERS_ERR);
    DTEST_LOG << "testGetMissionInfos006 end" << std::endl;
}

/**
 * @tc.name: testFetchCachedRemoteMissions001
 * @tc.desc: test stop fake device, return error code
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testFetchCachedRemoteMissions001, TestSize.Level1)
{
    std::vector<DstbMissionInfo> infos;
    auto ret = DistributedSchedMissionManager::GetInstance().FetchCachedRemoteMissions(DEVICE_ID, 0, infos);
    EXPECT_TRUE(ret != ERR_NONE);
}

/**
 * @tc.name: testFetchCachedRemoteMissions002
 * @tc.desc: test empty deviceId, return error code
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testFetchCachedRemoteMissions002, TestSize.Level3)
{
    DTEST_LOG << "testFetchCachedRemoteMissions002 begin" << std::endl;
    std::vector<DstbMissionInfo> infos;
    auto ret = DistributedSchedMissionManager::GetInstance().FetchCachedRemoteMissions("", 0, infos);
    EXPECT_TRUE(ret == INVALID_PARAMETERS_ERR);
    DTEST_LOG << "testFetchCachedRemoteMissions002 end" << std::endl;
}

/**
 * @tc.name: testFetchCachedRemoteMissions003
 * @tc.desc: test local deviceId, return error code
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testFetchCachedRemoteMissions003, TestSize.Level3)
{
    DTEST_LOG << "testFetchCachedRemoteMissions003 begin" << std::endl;
    std::vector<DstbMissionInfo> infos;
    auto ret = DistributedSchedMissionManager::GetInstance().FetchCachedRemoteMissions(localDeviceId_, 0, infos);
    EXPECT_TRUE(ret == ERR_NULL_OBJECT);
    DTEST_LOG << "testFetchCachedRemoteMissions003 end" << std::endl;
}

/**
 * @tc.name: testFetchCachedRemoteMissions004
 * @tc.desc: test stop fake device, return error code
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testFetchCachedRemoteMissions004, TestSize.Level3)
{
    DTEST_LOG << "testFetchCachedRemoteMissions004 begin" << std::endl;
    std::vector<DstbMissionInfo> infos;
    auto ret = DistributedSchedMissionManager::GetInstance().FetchCachedRemoteMissions(DEVICE_ID, NUM_MISSIONS, infos);
    EXPECT_TRUE(ret != ERR_NONE);
    DTEST_LOG << "testFetchCachedRemoteMissions004 end" << std::endl;
}

/**
 * @tc.name: testFetchCachedRemoteMissions005
 * @tc.desc: test empty deviceId, return error code
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testFetchCachedRemoteMissions005, TestSize.Level3)
{
    DTEST_LOG << "testFetchCachedRemoteMissions005 begin" << std::endl;
    std::vector<DstbMissionInfo> infos;
    auto ret = DistributedSchedMissionManager::GetInstance().FetchCachedRemoteMissions("", NUM_MISSIONS, infos);
    EXPECT_TRUE(ret == INVALID_PARAMETERS_ERR);
    DTEST_LOG << "testFetchCachedRemoteMissions005 end" << std::endl;
}

/**
 * @tc.name: testFetchCachedRemoteMissions006
 * @tc.desc: test local deviceId, return error code
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testFetchCachedRemoteMissions006, TestSize.Level3)
{
    DTEST_LOG << "testFetchCachedRemoteMissions006 begin" << std::endl;
    std::vector<DstbMissionInfo> infos;
    auto ret = DistributedSchedMissionManager::GetInstance().FetchCachedRemoteMissions(localDeviceId_,
        NUM_MISSIONS, infos);
    EXPECT_TRUE(ret == ERR_NULL_OBJECT);
    DTEST_LOG << "testFetchCachedRemoteMissions006 end" << std::endl;
}

/**
 * @tc.name: testFetchCachedRemoteMissions007
 * @tc.desc: test stop fake device, return error code
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testFetchCachedRemoteMissions007, TestSize.Level3)
{
    DTEST_LOG << "testFetchCachedRemoteMissions007 begin" << std::endl;
    std::vector<DstbMissionInfo> infos;
    auto ret = DistributedSchedMissionManager::GetInstance().FetchCachedRemoteMissions(DEVICE_ID,
        NORMAL_NUM_MISSIONS, infos);
    EXPECT_TRUE(ret != ERR_NONE);
    DTEST_LOG << "testFetchCachedRemoteMissions007 end" << std::endl;
}

/**
 * @tc.name: testFetchCachedRemoteMissions008
 * @tc.desc: test empty deviceId, return error code
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testFetchCachedRemoteMissions008, TestSize.Level3)
{
    DTEST_LOG << "testFetchCachedRemoteMissions008 begin" << std::endl;
    std::vector<DstbMissionInfo> infos;
    auto ret = DistributedSchedMissionManager::GetInstance().FetchCachedRemoteMissions("", NORMAL_NUM_MISSIONS, infos);
    EXPECT_TRUE(ret == INVALID_PARAMETERS_ERR);
    DTEST_LOG << "testFetchCachedRemoteMissions008 end" << std::endl;
}

/**
 * @tc.name: testFetchCachedRemoteMissions009
 * @tc.desc: test local deviceId, return error code
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testFetchCachedRemoteMissions009, TestSize.Level3)
{
    DTEST_LOG << "testFetchCachedRemoteMissions009 begin" << std::endl;
    std::vector<DstbMissionInfo> infos;
    auto ret = DistributedSchedMissionManager::GetInstance().FetchCachedRemoteMissions(localDeviceId_,
        NORMAL_NUM_MISSIONS, infos);
    EXPECT_TRUE(ret == ERR_NULL_OBJECT);
    DTEST_LOG << "testFetchCachedRemoteMissions009 end" << std::endl;
}

/**
 * @tc.name: testStopDataStorage001
 * @tc.desc: test stop data storage
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testStopDataStorage001, TestSize.Level3)
{
    DTEST_LOG << "testStopDataStorage001 begin" << std::endl;
    auto ret = DistributedSchedMissionManager::GetInstance().StopDataStorage();
    EXPECT_EQ(ret, ERR_NULL_OBJECT);
    DTEST_LOG << "testStopDataStorage001 end" << std::endl;
}

/**
 * @tc.name: testStoreSnapshotInfo001
 * @tc.desc: test store snapshot info
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testStoreSnapshotInfo001, TestSize.Level3)
{
    DTEST_LOG << "testStoreSnapshotInfo001 begin" << std::endl;
    auto ret = DistributedSchedMissionManager::GetInstance().StoreSnapshotInfo(DEVICE_ID, 0, 0, 0);
    EXPECT_EQ(ret, ERR_NULL_OBJECT);
    DTEST_LOG << "testStoreSnapshotInfo001 end" << std::endl;
}

/**
 * @tc.name: testRemoveSnapshotInfo001
 * @tc.desc: test remove snapshot info
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testRemoveSnapshotInfo001, TestSize.Level3)
{
    DTEST_LOG << "testRemoveSnapshotInfo001 begin" << std::endl;
    auto ret = DistributedSchedMissionManager::GetInstance().RemoveSnapshotInfo(DEVICE_ID, 0);
    EXPECT_EQ(ret, ERR_NULL_OBJECT);
    DTEST_LOG << "testRemoveSnapshotInfo001 end" << std::endl;
}

/**
 * @tc.name: testInitDataStorage001
 * @tc.desc: test init data storage
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testInitDataStorage001, TestSize.Level3)
{
    DTEST_LOG << "testInitDataStorage001 begin" << std::endl;
    auto ret = DistributedSchedMissionManager::GetInstance().InitDataStorage();
    EXPECT_EQ(ret, ERR_NONE);
    DTEST_LOG << "testInitDataStorage001 end" << std::endl;
}

/**
 * @tc.name: testInitDataStorage002
 * @tc.desc: test init data storage
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testInitDataStorage002, TestSize.Level3)
{
    DTEST_LOG << "testInitDataStorage002 begin" << std::endl;
    auto ret = DistributedSchedMissionManager::GetInstance().InitDataStorage();
    EXPECT_EQ(ret, ERR_NONE);
    DTEST_LOG << "testInitDataStorage002 end" << std::endl;
}

/**
 * @tc.name: testStoreSnapshotInfo002
 * @tc.desc: test store snapshot info
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testStoreSnapshotInfo002, TestSize.Level3)
{
    DTEST_LOG << "testStoreSnapshotInfo002 begin" << std::endl;
    auto ret = DistributedSchedMissionManager::GetInstance().StoreSnapshotInfo(DEVICE_ID_FALSE, 0, 0, 0);
    EXPECT_EQ(ret, INVALID_PARAMETERS_ERR);
    DTEST_LOG << "testStoreSnapshotInfo002 end" << std::endl;
}

/**
 * @tc.name: testRemoveSnapshotInfo002
 * @tc.desc: test remove snapshot info
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testRemoveSnapshotInfo002, TestSize.Level3)
{
    DTEST_LOG << "testRemoveSnapshotInfo002 begin" << std::endl;
    auto ret = DistributedSchedMissionManager::GetInstance().RemoveSnapshotInfo(DEVICE_ID_FALSE, 0);
    EXPECT_EQ(ret, INVALID_PARAMETERS_ERR);
    DTEST_LOG << "testRemoveSnapshotInfo002 end" << std::endl;
}

/**
 * @tc.name: testGetRemoteMissionSnapshotInfo001
 * @tc.desc: test get remote mission snapshot info
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testGetRemoteMissionSnapshotInfo001, TestSize.Level3)
{
    DTEST_LOG << "testGetRemoteMissionSnapshotInfo001 begin" << std::endl;
    DistributedSchedMissionManager::GetInstance().NotifySnapshotChanged(DEVICE_ID, 0);
    unique_ptr<AAFwk::MissionSnapshot> missionSnapshot = nullptr;
    auto ret = DistributedSchedMissionManager::GetInstance().GetRemoteMissionSnapshotInfo("", 0, missionSnapshot);
    EXPECT_EQ(ret, INVALID_PARAMETERS_ERR);
    DTEST_LOG << "testGetRemoteMissionSnapshotInfo001 end" << std::endl;
}

/**
 * @tc.name: testDeviceOnlineNotify003
 * @tc.desc: test device online notify
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testDeviceOnlineNotify003, TestSize.Level3)
{
    DTEST_LOG << "testDeviceOnlineNotify003 begin" << std::endl;
    DistributedSchedMissionManager::GetInstance().Init();
    DistributedSchedMissionManager::GetInstance().DeviceOnlineNotify("");
    DistributedSchedMissionManager::GetInstance().DeviceOnlineNotify(DEVICE_ID);
    EXPECT_NE(DistributedSchedMissionManager::GetInstance().missionHandler_, nullptr);
    DTEST_LOG << "testDeviceOnlineNotify003 end" << std::endl;
}

/**
 * @tc.name: testOnRemoteDied001
 * @tc.desc: test on remote died
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testOnRemoteDied001, TestSize.Level3)
{
    DTEST_LOG << "testOnRemoteDied001 begin" << std::endl;
    //DeviceOfflineNotify
    DistributedSchedMissionManager::GetInstance().DeviceOfflineNotify("");
    std::set<std::string> remoteSyncDeviceSet_ = set<std::string>();
    DistributedSchedMissionManager::GetInstance().DeviceOfflineNotify(DEVICE_ID);

    //OnRemoteDied
    wptr<IRemoteObject> remote = nullptr;
    DistributedSchedMissionManager::GetInstance().OnRemoteDied(remote);
    remote = new RemoteMissionListenerTest();
    DistributedSchedMissionManager::GetInstance().OnRemoteDied(remote);
    EXPECT_NE(remote.promote(), nullptr);
    DTEST_LOG << "testOnRemoteDied001 end" << std::endl;
}

/**
 * @tc.name: testStartSyncMissionsFromRemote001
 * @tc.desc: prepare and sync missions from remote
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testStartSyncMissionsFromRemote001, TestSize.Level3)
{
    DTEST_LOG << "testStartSyncMissionsFromRemote001 begin" << std::endl;
    DistributedSchedUtil::MockManageMissions();
    std::vector<DstbMissionInfo> missionInfos;
    CallerInfo callerInfo;
    DistributedSchedMissionManager::GetInstance().Init();
    DistributedSchedMissionManager::GetInstance().GenerateCallerInfo(callerInfo);
    auto ret = DistributedSchedMissionManager::GetInstance().StartSyncMissionsFromRemote(callerInfo, missionInfos);
    EXPECT_EQ(ret, ERR_NONE);
    DTEST_LOG << "testStartSyncMissionsFromRemote001 end" << std::endl;
}

/**
 * @tc.name: testStartSyncMissionsFromRemote001
 * @tc.desc: check access control
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testCheckAccessControlForMissions001, TestSize.Level3)
{
    DTEST_LOG << "testCheckAccessControlForMissions001 begin" << std::endl;
    DistributedSchedUtil::MockManageMissions();
    std::vector<DstbMissionInfo> missionInfos;
    CallerInfo callerInfo;
    DistributedSchedMissionManager::GetInstance().Init();
    DistributedSchedMissionManager::GetInstance().GenerateCallerInfo(callerInfo);
    auto ret = DistributedSchedMissionManager::GetInstance().CheckAccessControlForMissions(callerInfo);
    EXPECT_EQ(ret, false);
    DTEST_LOG << "testCheckAccessControlForMissions001 end" << std::endl;
}

/**
 * @tc.name: testStopSyncMissionsFromRemote001
 * @tc.desc: stop sync missions from remote
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testStopSyncMissionsFromRemote001, TestSize.Level3)
{
    DTEST_LOG << "testStopSyncMissionsFromRemote001 begin" << std::endl;
    DistributedSchedMissionManager::GetInstance().Init();
    DistributedSchedMissionManager::GetInstance().StopSyncMissionsFromRemote("");
    DistributedSchedMissionManager::GetInstance().remoteSyncDeviceSet_.clear();
    DistributedSchedMissionManager::GetInstance().remoteSyncDeviceSet_.insert(DEVICE_ID);
    DistributedSchedMissionManager::GetInstance().StopSyncMissionsFromRemote(DEVICE_ID);
    EXPECT_EQ(DistributedSchedMissionManager::GetInstance().remoteSyncDeviceSet_.empty(), true);
    DTEST_LOG << "testStopSyncMissionsFromRemote001 end" << std::endl;
}

/**
 * @tc.name: testNotifyMissionsChangedFromRemote001
 * @tc.desc: notify missions changed from remote
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testNotifyMissionsChangedFromRemote001, TestSize.Level3)
{
    DTEST_LOG << "testNotifyMissionsChangedFromRemote001 begin" << std::endl;
    CallerInfo callerInfo;
    callerInfo.sourceDeviceId = DEVICE_ID;
    callerInfo.uid = 0;
    callerInfo.pid = 0;
    callerInfo.dmsVersion = 0;
    std::vector<DstbMissionInfo> missionInfos;
    DistributedSchedMissionManager::GetInstance().Init();
    auto ret = DistributedSchedMissionManager::GetInstance().NotifyMissionsChangedFromRemote(callerInfo, missionInfos);
    EXPECT_EQ(ret, INVALID_PARAMETERS_ERR);
    DTEST_LOG << "testNotifyMissionsChangedFromRemote001 end" << std::endl;
}

/**
 * @tc.name: testNotifyMissionsChangedFromRemote002
 * @tc.desc: notify missions changed from remote
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testNotifyMissionsChangedFromRemote002, TestSize.Level3)
{
    DTEST_LOG << "testNotifyMissionsChangedFromRemote002 begin" << std::endl;
    CallerInfo callerInfo;
    callerInfo.sourceDeviceId = DEVICE_ID;
    callerInfo.uid = 0;
    callerInfo.pid = 0;
    callerInfo.dmsVersion = 0;
    std::vector<DstbMissionInfo> missionInfos;
    DistributedSchedMissionManager::GetInstance().Init();
    sptr<IRemoteObject> listener(new RemoteMissionListenerTest());
    auto& listenerInfo = DistributedSchedMissionManager::GetInstance().listenDeviceMap_[U16DEVICE_ID];
    EXPECT_EQ(listenerInfo.Emplace(listener), true);
    auto ret = DistributedSchedMissionManager::GetInstance().NotifyMissionsChangedFromRemote(callerInfo, missionInfos);
    EXPECT_EQ(ret, ERR_NONE);
    DTEST_LOG << "testNotifyMissionsChangedFromRemote002 end" << std::endl;
}

/**
 * @tc.name: testNotifyLocalMissionsChanged001
 * @tc.desc: notify local missions changed
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testNotifyLocalMissionsChanged001, TestSize.Level3)
{
    DTEST_LOG << "testNotifyLocalMissionsChanged001 begin" << std::endl;
    DistributedSchedMissionManager::GetInstance().Init();
    DistributedSchedMissionManager::GetInstance().NotifyLocalMissionsChanged();
    EXPECT_NE(DistributedSchedMissionManager::GetInstance().missionChangeHandler_, nullptr);
    DTEST_LOG << "testNotifyLocalMissionsChanged001 end" << std::endl;
}

/**
 * @tc.name: testNotifyMissionSnapshotCreated001
 * @tc.desc: notify mission snapshot created
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testNotifyMissionSnapshotCreated001, TestSize.Level3)
{
    DTEST_LOG << "testNotifyMissionSnapshotCreated001 begin" << std::endl;
    DistributedSchedMissionManager::GetInstance().missionChangeHandler_ = nullptr;
    DistributedSchedMissionManager::GetInstance().NotifyMissionSnapshotCreated(1);
    DistributedSchedMissionManager::GetInstance().Init();
    DistributedSchedMissionManager::GetInstance().NotifyMissionSnapshotCreated(1);
    EXPECT_NE(DistributedSchedMissionManager::GetInstance().missionChangeHandler_, nullptr);
    DTEST_LOG << "testNotifyMissionSnapshotCreated001 end" << std::endl;
}

/**
 * @tc.name: testNotifyMissionSnapshotChanged001
 * @tc.desc: notify mission snapshot changed
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testNotifyMissionSnapshotChanged001, TestSize.Level3)
{
    DTEST_LOG << "testNotifyMissionSnapshotChanged001 begin" << std::endl;
    DistributedSchedMissionManager::GetInstance().Init();
    DistributedSchedMissionManager::GetInstance().NotifyMissionSnapshotChanged(1);
    EXPECT_NE(DistributedSchedMissionManager::GetInstance().missionChangeHandler_, nullptr);
    DTEST_LOG << "testNotifyMissionSnapshotChanged001 end" << std::endl;
}

/**
 * @tc.name: testNotifyMissionSnapshotDestroyed001
 * @tc.desc: notify mission snapshot destroyed
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testNotifyMissionSnapshotDestroyed001, TestSize.Level3)
{
    DTEST_LOG << "testNotifyMissionSnapshotDestroyed001 begin" << std::endl;
    DistributedSchedMissionManager::GetInstance().Init();
    DistributedSchedMissionManager::GetInstance().NotifyMissionSnapshotDestroyed(1);
    EXPECT_NE(DistributedSchedMissionManager::GetInstance().missionChangeHandler_, nullptr);
    DTEST_LOG << "testNotifyMissionSnapshotDestroyed001 end" << std::endl;
}

/**
 * @tc.name: testNotifyMissionsChangedToRemote001
 * @tc.desc: notify missions changed to remote
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testNotifyMissionsChangedToRemote001, TestSize.Level3)
{
    DTEST_LOG << "testNotifyMissionsChangedToRemote001 begin" << std::endl;
    std::vector<DstbMissionInfo> missionInfos;
    auto ret = DistributedSchedMissionManager::GetInstance().NotifyMissionsChangedToRemote(missionInfos);
    EXPECT_EQ(ret, ERR_NONE);
    DTEST_LOG << "testNotifyMissionsChangedToRemote001 end" << std::endl;
}

/**
 * @tc.name: testOnRemoteDmsDied001
 * @tc.desc: on remote dms died
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testOnRemoteDmsDied001, TestSize.Level3)
{
    DTEST_LOG << "testOnRemoteDmsDied001 begin" << std::endl;
    //NotifyMissionsChangedToRemoteInner
    CallerInfo callerInfo;
    std::vector<DstbMissionInfo> missionInfos;
    DistributedSchedMissionManager::GetInstance().NotifyMissionsChangedToRemoteInner("", missionInfos, callerInfo);
    DistributedSchedMissionManager::GetInstance().NotifyMissionsChangedToRemoteInner(
        DEVICE_ID, missionInfos, callerInfo);
    
    wptr<IRemoteObject> remote = nullptr;
    DistributedSchedMissionManager::GetInstance().OnRemoteDmsDied(remote);
    remote = wptr<IRemoteObject>();
    DistributedSchedMissionManager::GetInstance().OnRemoteDmsDied(remote);

    DistributedSchedMissionManager::GetInstance().missionHandler_ = nullptr;
    DistributedSchedMissionManager::GetInstance().OnRemoteDmsDied(remote);
    EXPECT_EQ(DistributedSchedMissionManager::GetInstance().missionHandler_, nullptr);
    DTEST_LOG << "testOnRemoteDmsDied001 end" << std::endl;
}

/**
 * @tc.name: testRetryRegisterMissionChange001
 * @tc.desc: retry register mission change
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testRetryRegisterMissionChange001, TestSize.Level3)
{
    DTEST_LOG << "testRetryRegisterMissionChange001 begin" << std::endl;
    //RetryStartSyncRemoteMissions
    DistributedSchedMissionManager::GetInstance().Init();
    DistributedSchedMissionManager::GetInstance().RetryStartSyncRemoteMissions("", localDeviceId_, 1);
    DistributedSchedMissionManager::GetInstance().RetryStartSyncRemoteMissions(DEVICE_ID, localDeviceId_, 1);

    //OnMissionListenerDied
    sptr<IRemoteObject> remote = nullptr;
    DistributedSchedMissionManager::GetInstance().OnMissionListenerDied(remote);

    remote = sptr<IRemoteObject>();
    DistributedSchedMissionManager::GetInstance().OnMissionListenerDied(remote);

    //RetryRegisterMissionChange
    DistributedSchedMissionManager::GetInstance().RetryRegisterMissionChange(1);
    EXPECT_NE(DistributedSchedMissionManager::GetInstance().missionHandler_, nullptr);
    DTEST_LOG << "testRetryRegisterMissionChange001 end" << std::endl;
}

/**
 * @tc.name: testMissionSnapshotChanged001
 * @tc.desc: mission snapshot changed
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testMissionSnapshotChanged001, TestSize.Level3)
{
    DTEST_LOG << "testMissionSnapshotChanged001 begin" << std::endl;
    DistributedSchedMissionManager::GetInstance().NotifyDmsProxyProcessDied();
    std::vector<DstbMissionInfo> missionInfos;
    DistributedSchedMissionManager::GetInstance().InitAllSnapshots(missionInfos);
    auto ret = DistributedSchedMissionManager::GetInstance().MissionSnapshotChanged(NUM_MISSIONS);
    EXPECT_EQ(ret, ERR_NONE);
    DTEST_LOG << "testMissionSnapshotChanged001 end" << std::endl;
}

/**
 * @tc.name: ProxyCallStopSyncMissionsFromRemote001
 * @tc.desc: call dms proxy StopSyncMissionsFromRemote
 * @tc.type: FUNC
 * @tc.require: I5XDDS
 */
HWTEST_F(DMSMissionManagerTest, ProxyCallStopSyncMissionsFromRemote001, TestSize.Level3)
{
    DTEST_LOG << "DistributedSchedServiceTest ProxyCallStopSyncMissionsFromRemote001 start" << std::endl;
    //OnDnetDied
    DistributedSchedMissionManager::GetInstance().Init();
    DistributedSchedMissionManager::GetInstance().OnDnetDied();
    DistributedSchedMissionManager::GetInstance().isRegMissionChange_ = false;
    DistributedSchedMissionManager::GetInstance().OnDnetDied();

    //proxy StopSyncMissionsFromRemote
    sptr<IDistributedSched> proxy = GetDms();
    ASSERT_NE(nullptr, proxy);
    CallerInfo callerInfo;
    int32_t ret = proxy->StopSyncMissionsFromRemote(callerInfo);
    EXPECT_EQ(ret, REQUEST_CODE_ERR);
    DTEST_LOG << "DistributedSchedServiceTest ProxyCallStopSyncMissionsFromRemote001 end" << std::endl;
}

/**
 * @tc.name: ProxyCallNotifyMissionsChangedFromRemote001
 * @tc.desc: call dms proxy NotifyMissionsChangedFromRemote
 * @tc.type: FUNC
 * @tc.require: I5XDDS
 */
HWTEST_F(DMSMissionManagerTest, ProxyCallNotifyMissionsChangedFromRemote001, TestSize.Level3)
{
    DTEST_LOG << "DistributedSchedServiceTest ProxyCallNotifyMissionsChangedFromRemote001 start" << std::endl;
    sptr<IDistributedSched> proxy = GetDms();
    ASSERT_NE(nullptr, proxy);
    CallerInfo callerInfo;
    std::vector<DstbMissionInfo> missionInfos;
    int32_t ret = proxy->NotifyMissionsChangedFromRemote(missionInfos, callerInfo);
    EXPECT_EQ(ret, REQUEST_CODE_ERR);
    DTEST_LOG << "DistributedSchedServiceTest ProxyCallNotifyMissionsChangedFromRemote001 end" << std::endl;
}

/**
 * @tc.name: ProxyCallGetRemoteMissionSnapshotInfo001
 * @tc.desc: call dms proxy GetRemoteMissionSnapshotInfo
 * @tc.type: FUNC
 * @tc.require: I5XDDS
 */
HWTEST_F(DMSMissionManagerTest, ProxyCallGetRemoteMissionSnapshotInfo001, TestSize.Level3)
{
    DTEST_LOG << "DistributedSchedServiceTest ProxyCallGetRemoteMissionSnapshotInfo001 start" << std::endl;
    sptr<IDistributedSched> proxy = GetDms();
    ASSERT_NE(nullptr, proxy);
    unique_ptr<AAFwk::MissionSnapshot> missionSnapshot = nullptr;
    int32_t ret = proxy->GetRemoteMissionSnapshotInfo("MockDevId", 1, missionSnapshot);
    EXPECT_EQ(ret, DMS_PERMISSION_DENIED);
    DTEST_LOG << "DistributedSchedServiceTest ProxyCallGetRemoteMissionSnapshotInfo001 end" << std::endl;
}

/**
 * @tc.name: ProxyCallGetRemoteMissionSnapshotInfo002
 * @tc.desc: call dms proxy GetRemoteMissionSnapshotInfo
 * @tc.type: FUNC
 * @tc.require: I5XDDS
 */
HWTEST_F(DMSMissionManagerTest, ProxyCallGetRemoteMissionSnapshotInfo002, TestSize.Level3)
{
    DTEST_LOG << "DistributedSchedServiceTest ProxyCallGetRemoteMissionSnapshotInfo002 start" << std::endl;
    sptr<IDistributedSched> proxy = GetDms();
    ASSERT_NE(nullptr, proxy);
    unique_ptr<AAFwk::MissionSnapshot> missionSnapshot = nullptr;
    int32_t ret = proxy->GetRemoteMissionSnapshotInfo("", 1, missionSnapshot);
    EXPECT_EQ(ret, ERR_NULL_OBJECT);
    DTEST_LOG << "DistributedSchedServiceTest ProxyCallGetRemoteMissionSnapshotInfo002 end" << std::endl;
}

/**
 * @tc.name: ProxyCallGetRemoteMissionSnapshotInfo003
 * @tc.desc: call dms proxy GetRemoteMissionSnapshotInfo
 * @tc.type: FUNC
 * @tc.require: I5XDDS
 */
HWTEST_F(DMSMissionManagerTest, ProxyCallGetRemoteMissionSnapshotInfo003, TestSize.Level3)
{
    DTEST_LOG << "DistributedSchedServiceTest ProxyCallGetRemoteMissionSnapshotInfo003 start" << std::endl;
    sptr<IDistributedSched> proxy = GetDms();
    ASSERT_NE(nullptr, proxy);
    unique_ptr<AAFwk::MissionSnapshot> missionSnapshot = nullptr;
    int32_t ret = proxy->GetRemoteMissionSnapshotInfo("MockDevId", -1, missionSnapshot);
    EXPECT_EQ(ret, INVALID_PARAMETERS_ERR);
    DTEST_LOG << "DistributedSchedServiceTest ProxyCallGetRemoteMissionSnapshotInfo003 end" << std::endl;
}

/**
 * @tc.name: testDeleteDataStorage005
 * @tc.desc: test DeleteDataStorage
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testDeleteDataStorage005, TestSize.Level3)
{
    DTEST_LOG << "testDeleteDataStorage005 begin" << std::endl;
    std::string uuid = "uuid for GetUuidByNetworkId";
    {
        lock_guard<mutex> autoLock(DtbschedmgrDeviceInfoStorage::GetInstance().uuidNetworkIdLock_);
        DtbschedmgrDeviceInfoStorage::GetInstance().uuidNetworkIdMap_[uuid] = localDeviceId_;
    }
    DistributedSchedMissionManager::GetInstance().distributedDataStorage_ =
        std::make_shared<DistributedDataStorage>();
    auto result = DistributedSchedMissionManager::GetInstance().distributedDataStorage_->Init();
    auto initDoneTask = []() {
        std::lock_guard<std::mutex> autoLock(caseDoneLock_);
        isCaseDone_ = true;
        caseDoneCondition_.notify_all();
    };
    if (DistributedSchedMissionManager::GetInstance().distributedDataStorage_ != nullptr) {
        std::shared_ptr<AppExecFwk::EventHandler> dmsDataStorageHandler =
            DistributedSchedMissionManager::GetInstance().distributedDataStorage_->dmsDataStorageHandler_;
        if (dmsDataStorageHandler != nullptr) {
            dmsDataStorageHandler->PostTask(initDoneTask);
        }
    }
    std::unique_lock<std::mutex> lock(caseDoneLock_);
    caseDoneCondition_.wait_for(lock, std::chrono::milliseconds(MAX_WAIT_TIME),
        [&] () { return isCaseDone_; });
    EXPECT_TRUE(result);
    isCaseDone_ = false;
    uint8_t* byteStream = new uint8_t[BYTESTREAM_LENGTH];
    for (size_t i = 0; i < BYTESTREAM_LENGTH; ++i) {
        byteStream[i] = ONE_BYTE;
    }
    result = DistributedSchedMissionManager::GetInstance().distributedDataStorage_->Insert(localDeviceId_,
        TASK_ID, byteStream, BYTESTREAM_LENGTH);
    EXPECT_TRUE(result);
    DistributedSchedMissionManager::GetInstance().missionHandler_ = nullptr;
    DistributedSchedMissionManager::GetInstance().DeleteDataStorage(localDeviceId_, false);
    DistributedSchedMissionManager::GetInstance().distributedDataStorage_->Stop();
    delete[] byteStream;
    byteStream = nullptr;
    DTEST_LOG << "testDeleteDataStorage005 end" << std::endl;
}

/**
 * @tc.name: testDeleteDataStorage006
 * @tc.desc: test DeleteDataStorage
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testDeleteDataStorage006, TestSize.Level3)
{
    DTEST_LOG << "testDeleteDataStorage006 begin" << std::endl;
    std::string uuid = "uuid for GetUuidByNetworkId";
    {
        lock_guard<mutex> autoLock(DtbschedmgrDeviceInfoStorage::GetInstance().uuidNetworkIdLock_);
        DtbschedmgrDeviceInfoStorage::GetInstance().uuidNetworkIdMap_[uuid] = localDeviceId_;
    }
    DistributedSchedMissionManager::GetInstance().distributedDataStorage_ =
        std::make_shared<DistributedDataStorage>();
    auto result = DistributedSchedMissionManager::GetInstance().distributedDataStorage_->Init();
    auto initDoneTask = []() {
        std::lock_guard<std::mutex> autoLock(caseDoneLock_);
        isCaseDone_ = true;
        caseDoneCondition_.notify_all();
    };
    if (DistributedSchedMissionManager::GetInstance().distributedDataStorage_ != nullptr) {
        std::shared_ptr<AppExecFwk::EventHandler> dmsDataStorageHandler =
            DistributedSchedMissionManager::GetInstance().distributedDataStorage_->dmsDataStorageHandler_;
        if (dmsDataStorageHandler != nullptr) {
            dmsDataStorageHandler->PostTask(initDoneTask);
        }
    }
    std::unique_lock<std::mutex> lock(caseDoneLock_);
    caseDoneCondition_.wait_for(lock, std::chrono::milliseconds(MAX_WAIT_TIME),
        [&] () { return isCaseDone_; });
    EXPECT_TRUE(result);
    isCaseDone_ = false;
    uint8_t* byteStream = new uint8_t[BYTESTREAM_LENGTH];
    for (size_t i = 0; i < BYTESTREAM_LENGTH; ++i) {
        byteStream[i] = ONE_BYTE;
    }
    result = DistributedSchedMissionManager::GetInstance().distributedDataStorage_->Insert(localDeviceId_,
        TASK_ID, byteStream, BYTESTREAM_LENGTH);
    EXPECT_TRUE(result);
    DistributedSchedMissionManager::GetInstance().missionHandler_ = nullptr;
    DistributedSchedMissionManager::GetInstance().DeleteDataStorage(localDeviceId_, true);
    DistributedSchedMissionManager::GetInstance().distributedDataStorage_->Stop();
    delete[] byteStream;
    byteStream = nullptr;
    DTEST_LOG << "testDeleteDataStorage006 end" << std::endl;
}

/**
 * @tc.name: testRegisterMissionListener009
 * @tc.desc: test register mission listener
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testRegisterMissionListener009, TestSize.Level3)
{
    DTEST_LOG << "testRegisterMissionListener009 begin" << std::endl;
    auto runner = AppExecFwk::EventRunner::Create("MissionManagerHandler");
    DistributedSchedMissionManager::GetInstance().missionHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
    u16string deviceId = Str8ToStr16(localDeviceId_);
    std::string uuid = "uuid for GetUuidByNetworkId";
    {
        lock_guard<mutex> autoLock(DtbschedmgrDeviceInfoStorage::GetInstance().uuidNetworkIdLock_);
        DtbschedmgrDeviceInfoStorage::GetInstance().uuidNetworkIdMap_[uuid] = localDeviceId_;
    }
    sptr<IRemoteObject> listener(new RemoteMissionListenerTest());
    auto ret = DistributedSchedMissionManager::GetInstance().RegisterMissionListener(deviceId, listener);
    EXPECT_EQ(ret, INVALID_PARAMETERS_ERR);
    DTEST_LOG << "testRegisterMissionListener009 end" << std::endl;
}

/**
 * @tc.name: testRegisterMissionListener010
 * @tc.desc: test register mission listener
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testRegisterMissionListener010, TestSize.Level3)
{
    DTEST_LOG << "testRegisterMissionListener010 begin" << std::endl;
    auto runner = AppExecFwk::EventRunner::Create("MissionManagerHandler");
    DistributedSchedMissionManager::GetInstance().missionHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
    u16string deviceId = Str8ToStr16(DEVICE_ID);
    std::string uuid = "uuid for GetUuidByNetworkId";
    {
        lock_guard<mutex> autoLock(DtbschedmgrDeviceInfoStorage::GetInstance().uuidNetworkIdLock_);
        DtbschedmgrDeviceInfoStorage::GetInstance().uuidNetworkIdMap_[uuid] = DEVICE_ID;
    }
    sptr<IRemoteObject> listener(new RemoteMissionListenerTest());
    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().listenDeviceLock_);
        ListenerInfo listenerInfo;
        listenerInfo.Emplace(listener);
        DistributedSchedMissionManager::GetInstance().listenDeviceMap_[deviceId] = listenerInfo;
    }
    auto ret = DistributedSchedMissionManager::GetInstance().RegisterMissionListener(deviceId, listener);
    EXPECT_EQ(ret, ERR_NONE);
    DTEST_LOG << "testRegisterMissionListener010 end" << std::endl;
}

/**
 * @tc.name: testIsDeviceIdValidated002
 * @tc.desc: test IsDeviceIdValidated
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testIsDeviceIdValidated002, TestSize.Level1)
{
    DTEST_LOG << "testIsDeviceIdValidated002 begin" << std::endl;
    {
        lock_guard<mutex> autoLock(DtbschedmgrDeviceInfoStorage::GetInstance().deviceLock_);
        DtbschedmgrDeviceInfoStorage::GetInstance().remoteDevices_.clear();
    }

    int32_t deviceType = 0;
    std::shared_ptr<DmsDeviceInfo> dmsDeviceInfo =
        make_shared<DmsDeviceInfo>(DEVICE_NAME, deviceType, localDeviceId_);
    {
        lock_guard<mutex> autoLock(DtbschedmgrDeviceInfoStorage::GetInstance().deviceLock_);
        DtbschedmgrDeviceInfoStorage::GetInstance().remoteDevices_[localDeviceId_] = dmsDeviceInfo;
    }
    auto ret = DistributedSchedMissionManager::GetInstance().IsDeviceIdValidated(localDeviceId_);
    EXPECT_TRUE(ret);
    DTEST_LOG << "testIsDeviceIdValidated002 end" << std::endl;
}


/**
 * @tc.name: testGetMissionInfos007
 * @tc.desc: get missions
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testGetMissionInfos007, TestSize.Level3)
{
    DTEST_LOG << "testGetMissionInfos007 begin" << std::endl;
    {
        lock_guard<mutex> autoLock(DtbschedmgrDeviceInfoStorage::GetInstance().deviceLock_);
        DtbschedmgrDeviceInfoStorage::GetInstance().remoteDevices_.clear();
    }

    int32_t deviceType = 0;
    std::shared_ptr<DmsDeviceInfo> dmsDeviceInfo =
        make_shared<DmsDeviceInfo>(DEVICE_NAME, deviceType, DEVICE_ID);
    {
        lock_guard<mutex> autoLock(DtbschedmgrDeviceInfoStorage::GetInstance().deviceLock_);
        DtbschedmgrDeviceInfoStorage::GetInstance().remoteDevices_[DEVICE_ID] = dmsDeviceInfo;
    }
    int32_t numMissions = 0;
    std::vector<AAFwk::MissionInfo> missionInfos;
    auto ret = DistributedSchedMissionManager::GetInstance().GetMissionInfos(DEVICE_ID, numMissions, missionInfos);
    EXPECT_EQ(ret, INVALID_PARAMETERS_ERR);
    DTEST_LOG << "testGetMissionInfos007 end" << std::endl;
}

/**
 * @tc.name: testGetMissionInfos008
 * @tc.desc: get missions
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testGetMissionInfos008, TestSize.Level3)
{
    DTEST_LOG << "testGetMissionInfos008 begin" << std::endl;
    {
        lock_guard<mutex> autoLock(DtbschedmgrDeviceInfoStorage::GetInstance().deviceLock_);
        DtbschedmgrDeviceInfoStorage::GetInstance().remoteDevices_.clear();
    }

    int32_t deviceType = 0;
    std::shared_ptr<DmsDeviceInfo> dmsDeviceInfo =
        make_shared<DmsDeviceInfo>(DEVICE_NAME, deviceType, DEVICE_ID);
    {
        lock_guard<mutex> autoLock(DtbschedmgrDeviceInfoStorage::GetInstance().deviceLock_);
        DtbschedmgrDeviceInfoStorage::GetInstance().remoteDevices_[DEVICE_ID] = dmsDeviceInfo;
    }
    int32_t numMissions = NORMAL_NUM_MISSIONS;
    std::vector<AAFwk::MissionInfo> missionInfos;
    auto ret = DistributedSchedMissionManager::GetInstance().GetMissionInfos(DEVICE_ID, numMissions, missionInfos);
    EXPECT_EQ(ret, ERR_OK);
    DTEST_LOG << "testGetMissionInfos008 end" << std::endl;
}

/**
 * @tc.name: testGetMissionInfos009
 * @tc.desc: get missions
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testGetMissionInfos009, TestSize.Level3)
{
    DTEST_LOG << "testGetMissionInfos009 begin" << std::endl;
    {
        lock_guard<mutex> autoLock(DtbschedmgrDeviceInfoStorage::GetInstance().deviceLock_);
        DtbschedmgrDeviceInfoStorage::GetInstance().remoteDevices_.clear();
    }

    int32_t deviceType = 0;
    std::shared_ptr<DmsDeviceInfo> dmsDeviceInfo =
        make_shared<DmsDeviceInfo>(DEVICE_NAME, deviceType, DEVICE_ID);
    {
        lock_guard<mutex> autoLock(DtbschedmgrDeviceInfoStorage::GetInstance().deviceLock_);
        DtbschedmgrDeviceInfoStorage::GetInstance().remoteDevices_[localDeviceId_] = dmsDeviceInfo;
    }
    int32_t numMissions = NORMAL_NUM_MISSIONS;
    std::vector<AAFwk::MissionInfo> missionInfos;
    std::string uuid = DtbschedmgrDeviceInfoStorage::GetInstance().GetUuidByNetworkId(localDeviceId_);
    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().remoteMissionInfosLock_);
        DistributedSchedMissionManager::GetInstance().deviceMissionInfos_.erase(uuid);
    }
    std::vector<DstbMissionInfo> dstbMissionInfos;
    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().remoteMissionInfosLock_);
        DistributedSchedMissionManager::GetInstance().deviceMissionInfos_[uuid] = dstbMissionInfos;
    }
    auto ret = DistributedSchedMissionManager::GetInstance().GetMissionInfos(localDeviceId_, numMissions, missionInfos);
    EXPECT_EQ(ret, ERR_OK);
    DTEST_LOG << "testGetMissionInfos009 end" << std::endl;
}

/**
 * @tc.name: testInitDataStorage003
 * @tc.desc: test init data storage
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testInitDataStorage003, TestSize.Level3)
{
    DTEST_LOG << "testInitDataStorage003 begin" << std::endl;
    if (DistributedSchedMissionManager::GetInstance().distributedDataStorage_ == nullptr) {
        DistributedSchedMissionManager::GetInstance().distributedDataStorage_ =
            std::make_shared<DistributedDataStorage>();
    }
    auto ret = DistributedSchedMissionManager::GetInstance().InitDataStorage();
    EXPECT_EQ(ret, ERR_NONE);
    ret = DistributedSchedMissionManager::GetInstance().StopDataStorage();
    DTEST_LOG << "testInitDataStorage003 end" << std::endl;
}

/**
 * @tc.name: testStopDataStorage003
 * @tc.desc: test stop data storage
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testStopDataStorage003, TestSize.Level3)
{
    DTEST_LOG << "testStopDataStorage003 begin" << std::endl;
    DistributedSchedMissionManager::GetInstance().distributedDataStorage_ =
        std::make_shared<DistributedDataStorage>();
    auto result = DistributedSchedMissionManager::GetInstance().distributedDataStorage_->Init();
    auto initDoneTask = []() {
        std::lock_guard<std::mutex> autoLock(caseDoneLock_);
        isCaseDone_ = true;
        caseDoneCondition_.notify_all();
    };
    if (DistributedSchedMissionManager::GetInstance().distributedDataStorage_ != nullptr) {
        std::shared_ptr<AppExecFwk::EventHandler> dmsDataStorageHandler =
            DistributedSchedMissionManager::GetInstance().distributedDataStorage_->dmsDataStorageHandler_;
        if (dmsDataStorageHandler != nullptr) {
            dmsDataStorageHandler->PostTask(initDoneTask);
        }
    }
    std::unique_lock<std::mutex> lock(caseDoneLock_);
    caseDoneCondition_.wait_for(lock, std::chrono::milliseconds(MAX_WAIT_TIME),
        [&] () { return isCaseDone_; });
    EXPECT_TRUE(result);
    isCaseDone_ = false;
    auto ret = DistributedSchedMissionManager::GetInstance().StopDataStorage();
    EXPECT_EQ(ret, ERR_NONE);
    DTEST_LOG << "testStopDataStorage003 end" << std::endl;
}

/**
 * @tc.name: testStoreSnapshotInfo004
 * @tc.desc: test store snapshot info
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testStoreSnapshotInfo004, TestSize.Level3)
{
    DTEST_LOG << "testStoreSnapshotInfo004 begin" << std::endl;
    DistributedSchedMissionManager::GetInstance().distributedDataStorage_ =
            std::make_shared<DistributedDataStorage>();
    auto ret = DistributedSchedMissionManager::GetInstance().StoreSnapshotInfo(DEVICE_ID, 0, 0, 0);
    EXPECT_EQ(ret, INVALID_PARAMETERS_ERR);
    DTEST_LOG << "testStoreSnapshotInfo004 end" << std::endl;
}

/**
 * @tc.name: testStoreSnapshotInfo003
 * @tc.desc: test store snapshot info
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testStoreSnapshotInfo003, TestSize.Level3)
{
    DTEST_LOG << "testStoreSnapshotInfo003 begin" << std::endl;
    DistributedSchedMissionManager::GetInstance().distributedDataStorage_ =
        std::make_shared<DistributedDataStorage>();
    auto result = DistributedSchedMissionManager::GetInstance().distributedDataStorage_->Init();
    auto initDoneTask = []() {
        std::lock_guard<std::mutex> autoLock(caseDoneLock_);
        isCaseDone_ = true;
        caseDoneCondition_.notify_all();
    };
    if (DistributedSchedMissionManager::GetInstance().distributedDataStorage_ != nullptr) {
        std::shared_ptr<AppExecFwk::EventHandler> dmsDataStorageHandler =
            DistributedSchedMissionManager::GetInstance().distributedDataStorage_->dmsDataStorageHandler_;
        if (dmsDataStorageHandler != nullptr) {
            dmsDataStorageHandler->PostTask(initDoneTask);
        }
    }
    std::unique_lock<std::mutex> lock(caseDoneLock_);
    caseDoneCondition_.wait_for(lock, std::chrono::milliseconds(MAX_WAIT_TIME),
        [&] () { return isCaseDone_; });
    EXPECT_TRUE(result);
    isCaseDone_ = false;
    std::string uuid = "uuid for GetUuidByNetworkId";
    {
        lock_guard<mutex> autoLock(DtbschedmgrDeviceInfoStorage::GetInstance().uuidNetworkIdLock_);
        DtbschedmgrDeviceInfoStorage::GetInstance().uuidNetworkIdMap_[uuid] = localDeviceId_;
    }
    uint8_t* byteStream = new uint8_t[BYTESTREAM_LENGTH];
    for (size_t i = 0; i < BYTESTREAM_LENGTH; ++i) {
        byteStream[i] = ONE_BYTE;
    }
    auto ret = DistributedSchedMissionManager::GetInstance().
        StoreSnapshotInfo(localDeviceId_, TASK_ID, byteStream, BYTESTREAM_LENGTH);
    EXPECT_EQ(ret, ERR_NONE);
    ret = DistributedSchedMissionManager::GetInstance().RemoveSnapshotInfo(localDeviceId_, TASK_ID);
    EXPECT_EQ(ret, ERR_NONE);
    DistributedSchedMissionManager::GetInstance().distributedDataStorage_->Stop();
    delete[] byteStream;
    byteStream = nullptr;
    DTEST_LOG << "testStoreSnapshotInfo003 end" << std::endl;
}

/**
 * @tc.name: testRemoveSnapshotInfo003
 * @tc.desc: test remove snapshot info
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testRemoveSnapshotInfo003, TestSize.Level3)
{
    DTEST_LOG << "testRemoveSnapshotInfo003 begin" << std::endl;
    DistributedSchedMissionManager::GetInstance().distributedDataStorage_ =
            std::make_shared<DistributedDataStorage>();
    auto ret = DistributedSchedMissionManager::GetInstance().RemoveSnapshotInfo(DEVICE_ID, 0);
    EXPECT_EQ(ret, INVALID_PARAMETERS_ERR);
    DTEST_LOG << "testRemoveSnapshotInfo003 end" << std::endl;
}

/**
 * @tc.name: testGetRemoteMissionSnapshotInfo002
 * @tc.desc: test get remote mission snapshot info
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testGetRemoteMissionSnapshotInfo002, TestSize.Level3)
{
    DTEST_LOG << "testGetRemoteMissionSnapshotInfo002 begin" << std::endl;
    unique_ptr<AAFwk::MissionSnapshot> missionSnapshot = nullptr;
    DistributedSchedMissionManager::GetInstance().distributedDataStorage_ = nullptr;
    std::string uuid = "uuid for GetUuidByNetworkId";
    {
        lock_guard<mutex> autoLock(DtbschedmgrDeviceInfoStorage::GetInstance().uuidNetworkIdLock_);
        DtbschedmgrDeviceInfoStorage::GetInstance().uuidNetworkIdMap_[uuid] = localDeviceId_;
    }
    auto ret = DistributedSchedMissionManager::GetInstance().GetRemoteMissionSnapshotInfo(localDeviceId_,
        0, missionSnapshot);
    EXPECT_EQ(ret, ERR_NULL_OBJECT);
    DTEST_LOG << "testGetRemoteMissionSnapshotInfo002 end" << std::endl;
}

/**
 * @tc.name: testGetRemoteMissionSnapshotInfo004
 * @tc.desc: test get remote mission snapshot info
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testGetRemoteMissionSnapshotInfo004, TestSize.Level3)
{
    DTEST_LOG << "testGetRemoteMissionSnapshotInfo004 begin" << std::endl;
    unique_ptr<AAFwk::MissionSnapshot> missionSnapshot = nullptr;
    std::string uuid = "uuid for GetUuidByNetworkId";
    {
        lock_guard<mutex> autoLock(DtbschedmgrDeviceInfoStorage::GetInstance().uuidNetworkIdLock_);
        DtbschedmgrDeviceInfoStorage::GetInstance().uuidNetworkIdMap_[uuid] = localDeviceId_;
    }
    DistributedSchedMissionManager::GetInstance().distributedDataStorage_ =
        std::make_shared<DistributedDataStorage>();
    auto ret = DistributedSchedMissionManager::GetInstance().GetRemoteMissionSnapshotInfo(localDeviceId_,
        TASK_ID, missionSnapshot);
    EXPECT_EQ(ret, INVALID_PARAMETERS_ERR);
    DTEST_LOG << "testGetRemoteMissionSnapshotInfo004 end" << std::endl;
}

/**
 * @tc.name: testGetRemoteMissionSnapshotInfo005
 * @tc.desc: test get remote mission snapshot info
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testGetRemoteMissionSnapshotInfo005, TestSize.Level3)
{
    DTEST_LOG << "testGetRemoteMissionSnapshotInfo005 begin" << std::endl;
    unique_ptr<AAFwk::MissionSnapshot> missionSnapshot = nullptr;
    std::string uuid = "uuid for GetUuidByNetworkId";
    {
        lock_guard<mutex> autoLock(DtbschedmgrDeviceInfoStorage::GetInstance().uuidNetworkIdLock_);
        DtbschedmgrDeviceInfoStorage::GetInstance().uuidNetworkIdMap_[uuid] = localDeviceId_;
    }
    DistributedSchedMissionManager::GetInstance().distributedDataStorage_ =
        std::make_shared<DistributedDataStorage>();
    auto result = DistributedSchedMissionManager::GetInstance().distributedDataStorage_->Init();
    auto initDoneTask = []() {
        std::lock_guard<std::mutex> autoLock(caseDoneLock_);
        isCaseDone_ = true;
        caseDoneCondition_.notify_all();
    };
    if (DistributedSchedMissionManager::GetInstance().distributedDataStorage_ != nullptr) {
        std::shared_ptr<AppExecFwk::EventHandler> dmsDataStorageHandler =
            DistributedSchedMissionManager::GetInstance().distributedDataStorage_->dmsDataStorageHandler_;
        if (dmsDataStorageHandler != nullptr) {
            dmsDataStorageHandler->PostTask(initDoneTask);
        }
    }
    std::unique_lock<std::mutex> lock(caseDoneLock_);
    caseDoneCondition_.wait_for(lock, std::chrono::milliseconds(MAX_WAIT_TIME),
        [&] () { return isCaseDone_; });
    EXPECT_TRUE(result);
    isCaseDone_ = false;
    uint8_t* byteStream = new uint8_t[BYTESTREAM_LENGTH];
    for (size_t i = 0; i < BYTESTREAM_LENGTH; ++i) {
        byteStream[i] = ONE_BYTE;
    }
    result = DistributedSchedMissionManager::GetInstance().distributedDataStorage_->Insert(localDeviceId_,
        TASK_ID, byteStream, BYTESTREAM_LENGTH);
    EXPECT_TRUE(result);
    auto ret = DistributedSchedMissionManager::GetInstance().GetRemoteMissionSnapshotInfo(localDeviceId_,
        TASK_ID, missionSnapshot);
    EXPECT_EQ(ret, ERR_NULL_OBJECT);
    DistributedSchedMissionManager::GetInstance().distributedDataStorage_->Stop();
    delete[] byteStream;
    byteStream = nullptr;
    DTEST_LOG << "testGetRemoteMissionSnapshotInfo005 end" << std::endl;
}

/**
 * @tc.name: testDeviceOfflineNotify003
 * @tc.desc: test device offline notify
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testDeviceOfflineNotify003, TestSize.Level3)
{
    DTEST_LOG << "testDeviceOfflineNotify003 begin" << std::endl;
    DistributedSchedMissionManager::GetInstance().Init();
    DistributedSchedMissionManager::GetInstance().DeviceOnlineNotify(DEVICE_ID);
    sptr<IRemoteObject> listener(new RemoteMissionListenerTest());
    sptr<IDistributedSched> remoteDmsObj(new DistributedSchedProxy(listener));
    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().remoteDmsLock_);
        DistributedSchedMissionManager::GetInstance().remoteDmsMap_.clear();
    }
    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().remoteDmsLock_);
        DistributedSchedMissionManager::GetInstance().remoteDmsMap_[DEVICE_ID] = remoteDmsObj;
    }
    DistributedSchedMissionManager::GetInstance().DeviceOfflineNotify(DEVICE_ID);
    EXPECT_EQ(DistributedSchedMissionManager::GetInstance().remoteDmsMap_.empty(), true);
    DTEST_LOG << "testDeviceOfflineNotify003 end" << std::endl;
}

/**
 * @tc.name: testStartSyncRemoteMissions015
 * @tc.desc: prepare and sync missions from remote
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testStartSyncRemoteMissions015, TestSize.Level3)
{
    DTEST_LOG << "testStartSyncRemoteMissions015 begin" << std::endl;
    u16string deviceId = Str8ToStr16(DEVICE_ID);
    sptr<IRemoteObject> listener(new RemoteMissionListenerTest());
    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().listenDeviceLock_);
        DistributedSchedMissionManager::GetInstance().listenDeviceMap_.clear();
    }
    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().listenDeviceLock_);
        ListenerInfo listenerInfo;
        listenerInfo.Emplace(listener);
        listenerInfo.called = true;
        DistributedSchedMissionManager::GetInstance().listenDeviceMap_[deviceId] = listenerInfo;
    }
    sptr<IDistributedSched> proxy = GetDms();
    ASSERT_NE(nullptr, proxy);
    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().remoteDmsLock_);
        DistributedSchedMissionManager::GetInstance().remoteDmsMap_.clear();
    }
    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().remoteDmsLock_);
        DistributedSchedMissionManager::GetInstance().remoteDmsMap_[DEVICE_ID] = proxy;
    }
    auto ret = DistributedSchedMissionManager::GetInstance().StartSyncRemoteMissions(DEVICE_ID, localDeviceId_);
    EXPECT_EQ(ret, ERR_NONE);
    DTEST_LOG << "testStartSyncRemoteMissions015 end" << std::endl;
}

/**
 * @tc.name: testStartSyncRemoteMissions016
 * @tc.desc: prepare and sync missions from remote
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testStartSyncRemoteMissions016, TestSize.Level3)
{
    DTEST_LOG << "testStartSyncRemoteMissions016 begin" << std::endl;
    u16string deviceId = Str8ToStr16(DEVICE_ID);
    sptr<IRemoteObject> listener(new RemoteMissionListenerTest());
    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().listenDeviceLock_);
        DistributedSchedMissionManager::GetInstance().listenDeviceMap_.clear();
    }
    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().listenDeviceLock_);
        ListenerInfo listenerInfo;
        listenerInfo.Emplace(listener);
        DistributedSchedMissionManager::GetInstance().listenDeviceMap_[deviceId] = listenerInfo;
    }
    sptr<IDistributedSched> proxy = GetDms();
    ASSERT_NE(nullptr, proxy);
    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().remoteDmsLock_);
        DistributedSchedMissionManager::GetInstance().remoteDmsMap_.clear();
    }
    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().remoteDmsLock_);
        DistributedSchedMissionManager::GetInstance().remoteDmsMap_[DEVICE_ID] = proxy;
    }
    auto ret = DistributedSchedMissionManager::GetInstance().StartSyncRemoteMissions(DEVICE_ID, localDeviceId_);
    EXPECT_NE(ret, ERR_NONE);
    DTEST_LOG << "testStartSyncRemoteMissions016 end" << std::endl;
}

/**
 * @tc.name: testUnRegisterMissionListener006
 * @tc.desc: register mission listener
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testUnRegisterMissionListener006, TestSize.Level3)
{
    DTEST_LOG << "testUnRegisterMissionListener006 begin" << std::endl;
    /**
     * @tc.steps: step1. test device online notify
     */
    DTEST_LOG << "testDeviceOnlineNotify001 begin" << std::endl;
    DistributedSchedMissionManager::GetInstance().DeviceOnlineNotify("");
    DistributedSchedMissionManager::GetInstance().DeviceOnlineNotify(DEVICE_ID);
    DTEST_LOG << "testDeviceOnlineNotify001 end" << std::endl;
    
    sptr<IRemoteObject> listener(new RemoteMissionListenerTest());
    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().listenDeviceLock_);
        DistributedSchedMissionManager::GetInstance().listenDeviceMap_.clear();
    }
    auto deviceInfo = std::make_shared<DmsDeviceInfo>("", 0, "");
    DtbschedmgrDeviceInfoStorage::GetInstance().remoteDevices_[DEVICE_ID] = deviceInfo;
    auto ret = DistributedSchedMissionManager::GetInstance().UnRegisterMissionListener(U16DEVICE_ID, listener);
    EXPECT_EQ(ret, ERR_NONE);
    DTEST_LOG << "testUnRegisterMissionListener006 end" << std::endl;
}

/**
 * @tc.name: testUnRegisterMissionListener007
 * @tc.desc: register mission listener
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testUnRegisterMissionListener007, TestSize.Level3)
{
    DTEST_LOG << "testUnRegisterMissionListener007 begin" << std::endl;
    /**
     * @tc.steps: step1. test delete data storage
     */
    DTEST_LOG << "testDeleteDataStorage001 begin" << std::endl;
    DistributedSchedMissionManager::GetInstance().DeleteDataStorage("", false);
    DTEST_LOG << "testDeleteDataStorage001 end" << std::endl;
    DTEST_LOG << "testDeleteDataStorage002 begin" << std::endl;
    DistributedSchedMissionManager::GetInstance().DeleteDataStorage(DEVICE_ID, false);
    DTEST_LOG << "testDeleteDataStorage002 end" << std::endl;
    DTEST_LOG << "testDeleteDataStorage003 begin" << std::endl;
    DistributedSchedMissionManager::GetInstance().DeleteDataStorage("", true);
    DTEST_LOG << "testDeleteDataStorage003 end" << std::endl;
    DTEST_LOG << "testDeleteDataStorage004 begin" << std::endl;
    DistributedSchedMissionManager::GetInstance().DeleteDataStorage(DEVICE_ID, true);
    DTEST_LOG << "testDeleteDataStorage004 end" << std::endl;

    u16string deviceId = Str8ToStr16(DEVICE_ID);
    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().listenDeviceLock_);
        DistributedSchedMissionManager::GetInstance().listenDeviceMap_.clear();
    }
    sptr<IRemoteObject> listener(new RemoteMissionListenerTest());
    auto deviceInfo = std::make_shared<DmsDeviceInfo>("", 0, "");
    DtbschedmgrDeviceInfoStorage::GetInstance().remoteDevices_[DEVICE_ID] = deviceInfo;
    auto ret = DistributedSchedMissionManager::GetInstance().UnRegisterMissionListener(deviceId, listener);
    EXPECT_EQ(ret, ERR_NONE);
    DTEST_LOG << "testUnRegisterMissionListener007 end" << std::endl;
}

/**
 * @tc.name: testUnRegisterMissionListener008
 * @tc.desc: register mission listener
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testUnRegisterMissionListener008, TestSize.Level3)
{
    DTEST_LOG << "testUnRegisterMissionListener008 begin" << std::endl;
    /**
     * @tc.steps: step1. enqueue cached snapshot info
     */
    DTEST_LOG << "testEnqueueCachedSnapshotInfo001 begin" << std::endl;
    DistributedSchedMissionManager::GetInstance().EnqueueCachedSnapshotInfo("", 1, nullptr);
    DTEST_LOG << "testEnqueueCachedSnapshotInfo001 end" << std::endl;
    DTEST_LOG << "testEnqueueCachedSnapshotInfo002 begin" << std::endl;
    DistributedSchedMissionManager::GetInstance().EnqueueCachedSnapshotInfo(DEVICE_ID, 1, nullptr);
    DTEST_LOG << "testEnqueueCachedSnapshotInfo002 end" << std::endl;
    DTEST_LOG << "testEnqueueCachedSnapshotInfo003 begin" << std::endl;
    unique_ptr<Snapshot> snapshot = make_unique<Snapshot>();
    DistributedSchedMissionManager::GetInstance().EnqueueCachedSnapshotInfo("", 1, std::move(snapshot));
    DTEST_LOG << "testEnqueueCachedSnapshotInfo003 end" << std::endl;
    DTEST_LOG << "testDequeueCachedSnapshotInfo001 begin" << std::endl;
    DistributedSchedMissionManager::GetInstance().DequeueCachedSnapshotInfo("", 1);
    DTEST_LOG << "testDequeueCachedSnapshotInfo001 end" << std::endl;
    DTEST_LOG << "testDequeueCachedSnapshotInfo002 begin" << std::endl;
    DistributedSchedMissionManager::GetInstance().DequeueCachedSnapshotInfo(DEVICE_ID, 1);
    DTEST_LOG << "testDequeueCachedSnapshotInfo002 end" << std::endl;
    /**
     * @tc.steps: step2. reborn mission cache
     */
    DTEST_LOG << "testRebornMissionCache001 begin" << std::endl;
    std::vector<DstbMissionInfo> missionInfos;
    DistributedSchedMissionManager::GetInstance().RebornMissionCache("", missionInfos);
    DTEST_LOG << "testRebornMissionCache001 end" << std::endl;
    DTEST_LOG << "testRebornMissionCache002 begin" << std::endl;
    DistributedSchedMissionManager::GetInstance().RebornMissionCache(DEVICE_ID, missionInfos);
    DTEST_LOG << "testRebornMissionCache002 end" << std::endl;

    u16string deviceId = Str8ToStr16(DEVICE_ID);
    sptr<IRemoteObject> listener(new RemoteMissionListenerTest());
    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().listenDeviceLock_);
        DistributedSchedMissionManager::GetInstance().listenDeviceMap_.clear();
    }
    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().listenDeviceLock_);
        ListenerInfo listenerInfo;
        listenerInfo.Emplace(listener);
        DistributedSchedMissionManager::GetInstance().listenDeviceMap_[deviceId] = listenerInfo;
    }
    auto deviceInfo = std::make_shared<DmsDeviceInfo>("", 0, "");
    DtbschedmgrDeviceInfoStorage::GetInstance().remoteDevices_[DEVICE_ID] = deviceInfo;
    auto ret = DistributedSchedMissionManager::GetInstance().UnRegisterMissionListener(deviceId, listener);
    EXPECT_EQ(ret, ERR_NONE);
    DTEST_LOG << "testUnRegisterMissionListener008 end" << std::endl;
}

/**
 * @tc.name: testUnRegisterMissionListener009
 * @tc.desc: register mission listener
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testUnRegisterMissionListener009, TestSize.Level3)
{
    DTEST_LOG << "testUnRegisterMissionListener009 begin" << std::endl;
    /**
     * @tc.steps: step1. fetch device handler
     */
    DTEST_LOG << "testFetchDeviceHandler001 begin" << std::endl;
    DistributedSchedMissionManager::GetInstance().FetchDeviceHandler("");
    DistributedSchedMissionManager::GetInstance().FetchDeviceHandler(DEVICE_ID);
    DTEST_LOG << "testFetchDeviceHandler001 end" << std::endl;

    u16string deviceId = Str8ToStr16(DEVICE_ID);
    sptr<IRemoteObject> listener(new RemoteMissionListenerTest());
    sptr<IRemoteObject> nullListener = nullptr;
    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().listenDeviceLock_);
        DistributedSchedMissionManager::GetInstance().listenDeviceMap_.clear();
    }
    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().listenDeviceLock_);
        ListenerInfo listenerInfo;
        listenerInfo.Emplace(listener);
        listenerInfo.Emplace(nullListener);
        DistributedSchedMissionManager::GetInstance().listenDeviceMap_[deviceId] = listenerInfo;
    }
    auto deviceInfo = std::make_shared<DmsDeviceInfo>("", 0, "");
    DtbschedmgrDeviceInfoStorage::GetInstance().remoteDevices_[DEVICE_ID] = deviceInfo;
    auto ret = DistributedSchedMissionManager::GetInstance().UnRegisterMissionListener(deviceId, listener);
    EXPECT_EQ(ret, ERR_NONE);
    DTEST_LOG << "testUnRegisterMissionListener009 end" << std::endl;
}

/**
 * @tc.name: testCleanMissionResources002
 * @tc.desc: prepare and sync missions from remote
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testCleanMissionResources002, TestSize.Level3)
{
    DTEST_LOG << "testCleanMissionResources002 begin" << std::endl;
    u16string deviceId = Str8ToStr16(DEVICE_ID);
    sptr<IRemoteObject> nullListener = nullptr;
    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().listenDeviceLock_);
        DistributedSchedMissionManager::GetInstance().listenDeviceMap_.clear();
    }
    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().listenDeviceLock_);
        ListenerInfo listenerInfo;
        listenerInfo.Emplace(nullListener);
        DistributedSchedMissionManager::GetInstance().listenDeviceMap_[deviceId] = listenerInfo;
    }
    DistributedSchedMissionManager::GetInstance().CleanMissionResources(DEVICE_ID);
    EXPECT_EQ(DistributedSchedMissionManager::GetInstance().listenDeviceMap_.empty(), true);
    DTEST_LOG << "testCleanMissionResources002 end" << std::endl;
}

/**
 * @tc.name: testStopSyncRemoteMissions013
 * @tc.desc: stop sync missions from remote with deviceId
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testStopSyncRemoteMissions013, TestSize.Level3)
{
    DTEST_LOG << "testStopSyncRemoteMissions013 begin" << std::endl;
    /**
     * @tc.steps: step1. test GetRemoteDms
     */
    DTEST_LOG << "testGetRemoteDms001 begin" << std::endl;
    sptr<IDistributedSched> proxy = GetDms();
    ASSERT_NE(nullptr, proxy);
    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().remoteDmsLock_);
        DistributedSchedMissionManager::GetInstance().remoteDmsMap_.clear();
    }
    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().remoteDmsLock_);
        DistributedSchedMissionManager::GetInstance().remoteDmsMap_[localDeviceId_] = proxy;
    }

    DistributedSchedMissionManager::GetInstance().GetRemoteDms(localDeviceId_);
    DTEST_LOG << "testGetRemoteDms001 end" << std::endl;

    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().remoteDmsLock_);
        DistributedSchedMissionManager::GetInstance().remoteDmsMap_.clear();
    }
    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().remoteDmsLock_);
        DistributedSchedMissionManager::GetInstance().remoteDmsMap_[DEVICE_ID] = proxy;
    }
    auto ret = DistributedSchedMissionManager::GetInstance().StopSyncRemoteMissions(DEVICE_ID, false, false);
    EXPECT_NE(ret, ERR_NONE);
    DTEST_LOG << "testStopSyncRemoteMissions013 end" << std::endl;
}

/**
 * @tc.name: testStartSyncMissionsFromRemote002
 * @tc.desc: prepare and sync missions from remote
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testStartSyncMissionsFromRemote002, TestSize.Level3)
{
    DTEST_LOG << "testStartSyncMissionsFromRemote002 begin" << std::endl;
    DistributedSchedUtil::MockManageMissions();
    /**
     * @tc.steps: step1. test GetRemoteDms
     */
    DTEST_LOG << "testGetRemoteDms002 begin" << std::endl;
    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().remoteDmsLock_);
        DistributedSchedMissionManager::GetInstance().remoteDmsMap_.clear();
    }
    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().remoteDmsLock_);
        DistributedSchedMissionManager::GetInstance().remoteDmsMap_[localDeviceId_] = nullptr;
    }

    DistributedSchedMissionManager::GetInstance().GetRemoteDms(localDeviceId_);
    DTEST_LOG << "testGetRemoteDms002 end" << std::endl;

    std::vector<DstbMissionInfo> missionInfos;
    CallerInfo callerInfo;
    auto runner = AppExecFwk::EventRunner::Create("MissionManagerHandler");
    DistributedSchedMissionManager::GetInstance().missionHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
    DistributedSchedMissionManager::GetInstance().missionChangeListener_ = new DistributedMissionChangeListener();
    DistributedSchedMissionManager::GetInstance().GenerateCallerInfo(callerInfo);
    DistributedSchedMissionManager::GetInstance().isRegMissionChange_ = true;
    auto ret = DistributedSchedMissionManager::GetInstance().StartSyncMissionsFromRemote(callerInfo, missionInfos);
    EXPECT_EQ(ret, ERR_NONE);
    DTEST_LOG << "testStartSyncMissionsFromRemote002 end" << std::endl;
}

/**
 * @tc.name: testStartSyncMissionsFromRemote003
 * @tc.desc: prepare and sync missions from remote
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testStartSyncMissionsFromRemote003, TestSize.Level3)
{
    DTEST_LOG << "testStartSyncMissionsFromRemote003 begin" << std::endl;
    DistributedSchedUtil::MockManageMissions();
    /**
     * @tc.steps: step1. test notify remote died
     */
    DTEST_LOG << "testNotifyRemoteDied002 begin" << std::endl;
    wptr<IRemoteObject> remote = new RemoteMissionListenerTest();
    if (DistributedSchedMissionManager::GetInstance().distributedDataStorage_ == nullptr) {
        DistributedSchedMissionManager::GetInstance().distributedDataStorage_ =
            std::make_shared<DistributedDataStorage>();
    }
    DistributedSchedMissionManager::GetInstance().NotifyRemoteDied(remote);
    DTEST_LOG << "testNotifyRemoteDied002 end" << std::endl;

    std::vector<DstbMissionInfo> missionInfos;
    CallerInfo callerInfo;
    auto runner = AppExecFwk::EventRunner::Create("MissionManagerHandler");
    DistributedSchedMissionManager::GetInstance().missionHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
    DistributedSchedMissionManager::GetInstance().missionChangeListener_ = nullptr;
    DistributedSchedMissionManager::GetInstance().GenerateCallerInfo(callerInfo);
    DistributedSchedMissionManager::GetInstance().isRegMissionChange_ = false;
    auto ret = DistributedSchedMissionManager::GetInstance().StartSyncMissionsFromRemote(callerInfo, missionInfos);
    DistributedSchedMissionManager::GetInstance().StopSyncMissionsFromRemote(DEVICE_ID);
    EXPECT_EQ(ret, ERR_NONE);
    DTEST_LOG << "testStartSyncMissionsFromRemote003 end" << std::endl;
}

/**
 * @tc.name: testNeedSyncDevice003
 * @tc.desc: need sync device
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testNeedSyncDevice003, TestSize.Level3)
{
    DTEST_LOG << "testNeedSyncDevice003 begin" << std::endl;
    /**
     * @tc.steps: step1. delete cached snapshot info
     */
    DTEST_LOG << "testDeleteCachedSnapshotInfo001 begin" << std::endl;
    u16string deviceId = Str8ToStr16(DEVICE_ID);
    sptr<IRemoteObject> listener(new RemoteMissionListenerTest());
    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().listenDeviceLock_);
        DistributedSchedMissionManager::GetInstance().listenDeviceMap_.clear();
    }
    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().listenDeviceLock_);
        ListenerInfo listenerInfo;
        listenerInfo.Emplace(listener);
        listenerInfo.called = true;
        DistributedSchedMissionManager::GetInstance().listenDeviceMap_[deviceId] = listenerInfo;
    }
    DistributedSchedMissionManager::GetInstance().NotifySnapshotChanged(DEVICE_ID, 0);
    std::string uuid = "uuid for GetUuidByNetworkId";
    {
        lock_guard<mutex> autoLock(DtbschedmgrDeviceInfoStorage::GetInstance().uuidNetworkIdLock_);
        DtbschedmgrDeviceInfoStorage::GetInstance().uuidNetworkIdMap_.clear();
    }
    {
        lock_guard<mutex> autoLock(DtbschedmgrDeviceInfoStorage::GetInstance().uuidNetworkIdLock_);
        DtbschedmgrDeviceInfoStorage::GetInstance().uuidNetworkIdMap_[uuid] = DEVICE_ID;
    }
    std::unique_ptr<Snapshot> snapshot = make_unique<Snapshot>();
    std::string key = DistributedSchedMissionManager::GetInstance().GenerateKeyInfo(DEVICE_ID, 1);
    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().listenDeviceLock_);
        DistributedSchedMissionManager::GetInstance().cachedSnapshotInfos_.clear();
    }
    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().listenDeviceLock_);
        DistributedSchedMissionManager::GetInstance().cachedSnapshotInfos_[key] = std::move(snapshot);
    }
    DistributedSchedMissionManager::GetInstance().DeleteCachedSnapshotInfo(DEVICE_ID);
    DTEST_LOG << "testDeleteCachedSnapshotInfo001 end" << std::endl;

    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().remoteSyncDeviceLock_);
        DistributedSchedMissionManager::GetInstance().remoteSyncDeviceSet_.clear();
    }
    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().remoteSyncDeviceLock_);
        DistributedSchedMissionManager::GetInstance().remoteSyncDeviceSet_.emplace(DEVICE_ID);
    }
    bool ret = DistributedSchedMissionManager::GetInstance().NeedSyncDevice(DEVICE_ID);
    EXPECT_TRUE(ret);
    DTEST_LOG << "testNeedSyncDevice003 end" << std::endl;
}

/**
 * @tc.name: testNeedSyncDevice004
 * @tc.desc: need sync device
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testNeedSyncDevice004, TestSize.Level3)
{
    DTEST_LOG << "testNeedSyncDevice004 begin" << std::endl;
    std::string deviceId;
    bool ret = DistributedSchedMissionManager::GetInstance().NeedSyncDevice(deviceId);
    EXPECT_FALSE(ret);
    DTEST_LOG << "testNeedSyncDevice004 end" << std::endl;
}

/**
 * @tc.name: testHasSyncListener002
 * @tc.desc: need sync device
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testHasSyncListener002, TestSize.Level3)
{
    DTEST_LOG << "testHasSyncListener002 begin" << std::endl;
    /**
     * @tc.steps: step1. reborn mission cache
     */
    DTEST_LOG << "testRebornMissionCache003 begin" << std::endl;
    std::string uuid = "uuid for GetUuidByNetworkId";
    {
        lock_guard<mutex> autoLock(DtbschedmgrDeviceInfoStorage::GetInstance().uuidNetworkIdLock_);
        DtbschedmgrDeviceInfoStorage::GetInstance().uuidNetworkIdMap_.clear();
    }
    {
        lock_guard<mutex> autoLock(DtbschedmgrDeviceInfoStorage::GetInstance().uuidNetworkIdLock_);
        DtbschedmgrDeviceInfoStorage::GetInstance().uuidNetworkIdMap_[uuid] = DEVICE_ID;
    }
    std::vector<DstbMissionInfo> missionInfos;
    DistributedSchedMissionManager::GetInstance().RebornMissionCache(DEVICE_ID, missionInfos);
    DTEST_LOG << "testRebornMissionCache003 end" << std::endl;

    u16string deviceId = Str8ToStr16(DEVICE_ID);
    sptr<IRemoteObject> listener(new RemoteMissionListenerTest());
    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().listenDeviceLock_);
        DistributedSchedMissionManager::GetInstance().listenDeviceMap_.clear();
    }
    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().listenDeviceLock_);
        ListenerInfo listenerInfo;
        listenerInfo.Emplace(listener);
        listenerInfo.called = true;
        DistributedSchedMissionManager::GetInstance().listenDeviceMap_[deviceId] = listenerInfo;
    }
    bool ret = DistributedSchedMissionManager::GetInstance().HasSyncListener(DEVICE_ID);
    EXPECT_TRUE(ret);
    DTEST_LOG << "testHasSyncListener002 end" << std::endl;
}

/**
 * @tc.name: testDequeueCachedSnapshotInfo003
 * @tc.desc: enqueue cached snapshot info
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testDequeueCachedSnapshotInfo003, TestSize.Level3)
{
    DTEST_LOG << "testDequeueCachedSnapshotInfo003 begin" << std::endl;
    u16string deviceId = Str8ToStr16(DEVICE_ID);
    sptr<IRemoteObject> listener(new RemoteMissionListenerTest());
    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().listenDeviceLock_);
        DistributedSchedMissionManager::GetInstance().listenDeviceMap_.clear();
    }
    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().listenDeviceLock_);
        ListenerInfo listenerInfo;
        listenerInfo.called = false;
        DistributedSchedMissionManager::GetInstance().listenDeviceMap_[deviceId] = listenerInfo;
    }
    DistributedSchedMissionManager::GetInstance().NotifySnapshotChanged(DEVICE_ID, 0);
    std::unique_ptr<Snapshot> snapshot = make_unique<Snapshot>();
    std::string key = DistributedSchedMissionManager::GetInstance().GenerateKeyInfo(DEVICE_ID, 1);
    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().listenDeviceLock_);
        DistributedSchedMissionManager::GetInstance().cachedSnapshotInfos_.clear();
    }
    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().listenDeviceLock_);
        DistributedSchedMissionManager::GetInstance().cachedSnapshotInfos_[key] = std::move(snapshot);
    }
    auto ret = DistributedSchedMissionManager::GetInstance().DequeueCachedSnapshotInfo(DEVICE_ID, 1);
    EXPECT_NE(ret, nullptr);
    DTEST_LOG << "testDequeueCachedSnapshotInfo003 end" << std::endl;
}

/**
 * @tc.name: testFetchCachedRemoteMissions010
 * @tc.desc: test FetchCachedRemoteMissions
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testFetchCachedRemoteMissions010, TestSize.Level3)
{
    DTEST_LOG << "testFetchCachedRemoteMissions010 begin" << std::endl;
    std::string uuid = "uuid for GetUuidByNetworkId";
    {
        lock_guard<mutex> autoLock(DtbschedmgrDeviceInfoStorage::GetInstance().uuidNetworkIdLock_);
        DtbschedmgrDeviceInfoStorage::GetInstance().uuidNetworkIdMap_.clear();
    }
    {
        lock_guard<mutex> autoLock(DtbschedmgrDeviceInfoStorage::GetInstance().uuidNetworkIdLock_);
        DtbschedmgrDeviceInfoStorage::GetInstance().uuidNetworkIdMap_[uuid] = DEVICE_ID;
    }
    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().remoteMissionInfosLock_);
        DistributedSchedMissionManager::GetInstance().deviceMissionInfos_.clear();
    }
    std::vector<DstbMissionInfo> dstbMissionInfos;
    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().remoteMissionInfosLock_);
        DistributedSchedMissionManager::GetInstance().deviceMissionInfos_[uuid] = dstbMissionInfos;
    }
    auto ret = DistributedSchedMissionManager::GetInstance().FetchCachedRemoteMissions(DEVICE_ID,
        0, dstbMissionInfos);
    EXPECT_EQ(ret, ERR_NONE);
    DTEST_LOG << "testFetchCachedRemoteMissions010 end" << std::endl;
}

/**
 * @tc.name: testNotifyMissionsChangedFromRemote003
 * @tc.desc: notify missions changed from remote
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testNotifyMissionsChangedFromRemote003, TestSize.Level3)
{
    DTEST_LOG << "testNotifyMissionsChangedFromRemote003 begin" << std::endl;
    CallerInfo callerInfo;
    callerInfo.sourceDeviceId = DEVICE_ID;
    callerInfo.uid = 0;
    callerInfo.pid = 0;
    callerInfo.dmsVersion = 0;
    std::vector<DstbMissionInfo> missionInfos;
    u16string deviceId = Str8ToStr16(DEVICE_ID);
    sptr<IRemoteObject> listener(new RemoteMissionListenerTest());
    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().listenDeviceLock_);
        DistributedSchedMissionManager::GetInstance().listenDeviceMap_.clear();
    }
    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().listenDeviceLock_);
        ListenerInfo listenerInfo;
        listenerInfo.Emplace(listener);
        listenerInfo.called = true;
        DistributedSchedMissionManager::GetInstance().listenDeviceMap_[deviceId] = listenerInfo;
    }
    DistributedSchedMissionManager::GetInstance().missionHandler_ = nullptr;
    auto ret = DistributedSchedMissionManager::GetInstance().NotifyMissionsChangedFromRemote(callerInfo, missionInfos);
    EXPECT_EQ(ret, INVALID_PARAMETERS_ERR);
    DTEST_LOG << "testNotifyMissionsChangedFromRemote003 end" << std::endl;
}

/**
 * @tc.name: testNotifyMissionsChangedToRemote002
 * @tc.desc: notify missions changed to remote
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testNotifyMissionsChangedToRemote002, TestSize.Level3)
{
    DTEST_LOG << "testNotifyMissionsChangedToRemote002 begin" << std::endl;
    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().remoteSyncDeviceLock_);
        DistributedSchedMissionManager::GetInstance().remoteSyncDeviceSet_.clear();
    }
    std::vector<DstbMissionInfo> missionInfos;
    auto ret = DistributedSchedMissionManager::GetInstance().NotifyMissionsChangedToRemote(missionInfos);
    EXPECT_EQ(ret, ERR_NONE);
    DTEST_LOG << "testNotifyMissionsChangedToRemote002 end" << std::endl;
}

/**
 * @tc.name: testNotifyMissionsChangedToRemote003
 * @tc.desc: notify missions changed to remote
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testNotifyMissionsChangedToRemote003, TestSize.Level3)
{
    DTEST_LOG << "testNotifyMissionsChangedToRemote003 begin" << std::endl;
    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().remoteSyncDeviceLock_);
        DistributedSchedMissionManager::GetInstance().remoteSyncDeviceSet_.clear();
    }
    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().remoteSyncDeviceLock_);
        DistributedSchedMissionManager::GetInstance().remoteSyncDeviceSet_.emplace(localDeviceId_);
    }
    {
        lock_guard<mutex> autoLock(DtbschedmgrDeviceInfoStorage::GetInstance().deviceLock_);
        DtbschedmgrDeviceInfoStorage::GetInstance().remoteDevices_.clear();
    }
    int32_t deviceType = 0;
    std::shared_ptr<DmsDeviceInfo> dmsDeviceInfo =
        make_shared<DmsDeviceInfo>(DEVICE_NAME, deviceType, DEVICE_ID);
    {
        lock_guard<mutex> autoLock(DtbschedmgrDeviceInfoStorage::GetInstance().deviceLock_);
        DtbschedmgrDeviceInfoStorage::GetInstance().remoteDevices_[localDeviceId_] = dmsDeviceInfo;
    }
    std::string uuid = "uuid for GetUuidByNetworkId";
    {
        lock_guard<mutex> autoLock(DtbschedmgrDeviceInfoStorage::GetInstance().uuidNetworkIdLock_);
        DtbschedmgrDeviceInfoStorage::GetInstance().uuidNetworkIdMap_.clear();
    }
    {
        lock_guard<mutex> autoLock(DtbschedmgrDeviceInfoStorage::GetInstance().uuidNetworkIdLock_);
        DtbschedmgrDeviceInfoStorage::GetInstance().uuidNetworkIdMap_[uuid] = localDeviceId_;
    }
    std::vector<DstbMissionInfo> missionInfos;
    auto ret = DistributedSchedMissionManager::GetInstance().NotifyMissionsChangedToRemote(missionInfos);
    EXPECT_EQ(ret, ERR_NONE);
    DTEST_LOG << "testNotifyMissionsChangedToRemote003 end" << std::endl;
}

/**
 * @tc.name: testFetchDeviceHandler003
 * @tc.desc: fetch device handler
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testFetchDeviceHandler003, TestSize.Level3)
{
    DTEST_LOG << "testFetchDeviceHandler003 begin" << std::endl;
    {
        lock_guard<mutex> autoLock(DtbschedmgrDeviceInfoStorage::GetInstance().deviceLock_);
        DtbschedmgrDeviceInfoStorage::GetInstance().remoteDevices_.clear();
    }

    int32_t deviceType = 0;
    std::shared_ptr<DmsDeviceInfo> dmsDeviceInfo =
        make_shared<DmsDeviceInfo>(DEVICE_NAME, deviceType, DEVICE_ID);
    {
        lock_guard<mutex> autoLock(DtbschedmgrDeviceInfoStorage::GetInstance().deviceLock_);
        DtbschedmgrDeviceInfoStorage::GetInstance().remoteDevices_[DEVICE_ID] = dmsDeviceInfo;
    }
    auto ret = DistributedSchedMissionManager::GetInstance().FetchDeviceHandler(DEVICE_ID);
    EXPECT_EQ(ret, nullptr);
    DTEST_LOG << "testFetchDeviceHandler003 end" << std::endl;
}

/**
 * @tc.name: testFetchDeviceHandler004
 * @tc.desc: fetch device handler
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testFetchDeviceHandler004, TestSize.Level3)
{
    DTEST_LOG << "testFetchDeviceHandler004 begin" << std::endl;
    {
        lock_guard<mutex> autoLock(DtbschedmgrDeviceInfoStorage::GetInstance().deviceLock_);
        DtbschedmgrDeviceInfoStorage::GetInstance().remoteDevices_.clear();
    }
    int32_t deviceType = 0;
    std::shared_ptr<DmsDeviceInfo> dmsDeviceInfo =
        make_shared<DmsDeviceInfo>(DEVICE_NAME, deviceType, DEVICE_ID);
    {
        lock_guard<mutex> autoLock(DtbschedmgrDeviceInfoStorage::GetInstance().deviceLock_);
        DtbschedmgrDeviceInfoStorage::GetInstance().remoteDevices_[DEVICE_ID] = dmsDeviceInfo;
    }

    std::string uuid = "uuid for GetUuidByNetworkId";
    {
        lock_guard<mutex> autoLock(DtbschedmgrDeviceInfoStorage::GetInstance().uuidNetworkIdLock_);
        DtbschedmgrDeviceInfoStorage::GetInstance().uuidNetworkIdMap_.clear();
    }
    {
        lock_guard<mutex> autoLock(DtbschedmgrDeviceInfoStorage::GetInstance().uuidNetworkIdLock_);
        DtbschedmgrDeviceInfoStorage::GetInstance().uuidNetworkIdMap_[uuid] = DEVICE_ID;
    }
    auto ret = DistributedSchedMissionManager::GetInstance().FetchDeviceHandler(DEVICE_ID);
    EXPECT_NE(ret, nullptr);
    DTEST_LOG << "testFetchDeviceHandler004 end" << std::endl;
}

/**
 * @tc.name: testFetchDeviceHandler005
 * @tc.desc: fetch device handler
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testFetchDeviceHandler005, TestSize.Level3)
{
    DTEST_LOG << "testFetchDeviceHandler005 begin" << std::endl;
    DistributedSchedMissionManager::GetInstance().isRegMissionChange_ = false;
    DistributedSchedMissionManager::GetInstance().NotifyDmsProxyProcessDied();
    {
        lock_guard<mutex> autoLock(DtbschedmgrDeviceInfoStorage::GetInstance().deviceLock_);
        DtbschedmgrDeviceInfoStorage::GetInstance().remoteDevices_.clear();
    }
    int32_t deviceType = 0;
    std::shared_ptr<DmsDeviceInfo> dmsDeviceInfo =
        make_shared<DmsDeviceInfo>(DEVICE_NAME, deviceType, DEVICE_ID);
    {
        lock_guard<mutex> autoLock(DtbschedmgrDeviceInfoStorage::GetInstance().deviceLock_);
        DtbschedmgrDeviceInfoStorage::GetInstance().remoteDevices_[DEVICE_ID] = dmsDeviceInfo;
    }

    std::string uuid = "uuid for GetUuidByNetworkId";
    {
        lock_guard<mutex> autoLock(DtbschedmgrDeviceInfoStorage::GetInstance().uuidNetworkIdLock_);
        DtbschedmgrDeviceInfoStorage::GetInstance().uuidNetworkIdMap_.clear();
    }
    {
        lock_guard<mutex> autoLock(DtbschedmgrDeviceInfoStorage::GetInstance().uuidNetworkIdLock_);
        DtbschedmgrDeviceInfoStorage::GetInstance().uuidNetworkIdMap_[uuid] = DEVICE_ID;
    }
    auto anonyUuid = GetAnonymStr(uuid);
    auto runner = AppExecFwk::EventRunner::Create(anonyUuid + "_MissionN");
    auto handler = std::make_shared<AppExecFwk::EventHandler>(runner);
    DistributedSchedMissionManager::GetInstance().deviceHandle_.emplace(uuid, handler);
    auto ret = DistributedSchedMissionManager::GetInstance().FetchDeviceHandler(DEVICE_ID);
    EXPECT_NE(ret, nullptr);
    DTEST_LOG << "testFetchDeviceHandler005 end" << std::endl;
}

/**
 * @tc.name: testRetryStartSyncRemoteMissions004
 * @tc.desc: retry start sync remote missions
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testRetryStartSyncRemoteMissions004, TestSize.Level3)
{
    DTEST_LOG << "testRetryStartSyncRemoteMissions004 begin" << std::endl;
    auto runner = AppExecFwk::EventRunner::Create("MissionManagerHandler");
    DistributedSchedMissionManager::GetInstance().missionHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
    u16string deviceId = Str8ToStr16(DEVICE_ID);
    sptr<IRemoteObject> listener(new RemoteMissionListenerTest());
    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().listenDeviceLock_);
        DistributedSchedMissionManager::GetInstance().listenDeviceMap_.clear();
    }
    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().listenDeviceLock_);
        ListenerInfo listenerInfo;
        listenerInfo.Emplace(listener);
        listenerInfo.called = true;
        DistributedSchedMissionManager::GetInstance().listenDeviceMap_[deviceId] = listenerInfo;
    }
    DistributedSchedMissionManager::GetInstance().RetryStartSyncRemoteMissions(DEVICE_ID, localDeviceId_, 1);
    EXPECT_NE(DistributedSchedMissionManager::GetInstance().missionHandler_, nullptr);
    DTEST_LOG << "testRetryStartSyncRemoteMissions004 end" << std::endl;
}

/**
 * @tc.name: testRetryStartSyncRemoteMissions005
 * @tc.desc: retry start sync remote missions
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testRetryStartSyncRemoteMissions005, TestSize.Level3)
{
    DTEST_LOG << "testRetryStartSyncRemoteMissions005 begin" << std::endl;
    auto runner = AppExecFwk::EventRunner::Create("MissionManagerHandler");
    DistributedSchedMissionManager::GetInstance().missionHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
    u16string deviceId = Str8ToStr16(DEVICE_ID);
    sptr<IRemoteObject> listener(new RemoteMissionListenerTest());
    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().listenDeviceLock_);
        DistributedSchedMissionManager::GetInstance().listenDeviceMap_.clear();
    }
    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().listenDeviceLock_);
        ListenerInfo listenerInfo;
        listenerInfo.Emplace(listener);
        listenerInfo.called = true;
        DistributedSchedMissionManager::GetInstance().listenDeviceMap_[deviceId] = listenerInfo;
    }
    sptr<IDistributedSched> proxy = GetDms();
    ASSERT_NE(nullptr, proxy);
    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().remoteDmsLock_);
        DistributedSchedMissionManager::GetInstance().remoteDmsMap_.clear();
    }
    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().remoteDmsLock_);
        DistributedSchedMissionManager::GetInstance().remoteDmsMap_[DEVICE_ID] = proxy;
    }
    DistributedSchedMissionManager::GetInstance().RetryStartSyncRemoteMissions(DEVICE_ID, localDeviceId_, 1);
    EXPECT_NE(DistributedSchedMissionManager::GetInstance().missionHandler_, nullptr);
    DTEST_LOG << "testRetryStartSyncRemoteMissions005 end" << std::endl;
}

/**
 * @tc.name: testOnMissionListenerDied003
 * @tc.desc: on mission listener died
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testOnMissionListenerDied003, TestSize.Level3)
{
    DTEST_LOG << "testOnMissionListenerDied003 begin" << std::endl;
    u16string deviceId = Str8ToStr16(DEVICE_ID);
    sptr<IRemoteObject> listener(new RemoteMissionListenerTest());
    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().listenDeviceLock_);
        DistributedSchedMissionManager::GetInstance().listenDeviceMap_.clear();
    }
    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().listenDeviceLock_);
        ListenerInfo listenerInfo;
        listenerInfo.Emplace(listener);
        listenerInfo.called = true;
        DistributedSchedMissionManager::GetInstance().listenDeviceMap_[deviceId] = listenerInfo;
    }
    DistributedSchedMissionManager::GetInstance().OnMissionListenerDied(listener);
    EXPECT_EQ(DistributedSchedMissionManager::GetInstance().listenDeviceMap_.empty(), true);
    DTEST_LOG << "testOnMissionListenerDied003 end" << std::endl;
}

/**
 * @tc.name: testOnMissionListenerDied004
 * @tc.desc: on mission listener died
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testOnMissionListenerDied004, TestSize.Level3)
{
    DTEST_LOG << "testOnMissionListenerDied004 begin" << std::endl;
    u16string deviceId = Str8ToStr16(DEVICE_ID);
    sptr<IRemoteObject> listener(new RemoteMissionListenerTest());
    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().listenDeviceLock_);
        DistributedSchedMissionManager::GetInstance().listenDeviceMap_.clear();
    }
    sptr<IRemoteObject> remote = sptr<IRemoteObject>();
    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().listenDeviceLock_);
        ListenerInfo listenerInfo;
        listenerInfo.Emplace(listener);
        listenerInfo.called = true;
        listenerInfo.Emplace(remote);
        DistributedSchedMissionManager::GetInstance().listenDeviceMap_[deviceId] = listenerInfo;
    }
    DistributedSchedMissionManager::GetInstance().OnMissionListenerDied(listener);
    EXPECT_EQ(DistributedSchedMissionManager::GetInstance().listenDeviceMap_.empty(), false);
    DTEST_LOG << "testOnMissionListenerDied004 end" << std::endl;
}

/**
 * @tc.name: testOnMissionListenerDied005
 * @tc.desc: on mission listener died
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testOnMissionListenerDied005, TestSize.Level3)
{
    DTEST_LOG << "testOnMissionListenerDied005 begin" << std::endl;
    u16string deviceId = Str8ToStr16(DEVICE_ID);
    sptr<IRemoteObject> listener(new RemoteMissionListenerTest());
    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().listenDeviceLock_);
        DistributedSchedMissionManager::GetInstance().listenDeviceMap_.clear();
    }
    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().listenDeviceLock_);
        ListenerInfo listenerInfo;
        listenerInfo.Emplace(listener);
        listenerInfo.called = false;
        DistributedSchedMissionManager::GetInstance().listenDeviceMap_[deviceId] = listenerInfo;
    }
    DistributedSchedMissionManager::GetInstance().OnMissionListenerDied(listener);
    EXPECT_EQ(DistributedSchedMissionManager::GetInstance().listenDeviceMap_.empty(), true);
    DTEST_LOG << "testOnMissionListenerDied005 end" << std::endl;
}

/**
 * @tc.name: testOnRemoteDmsDied006
 * @tc.desc: on remote dms died
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testOnRemoteDmsDied006, TestSize.Level3)
{
    DTEST_LOG << "testOnRemoteDmsDied006 begin" << std::endl;
    auto runner = AppExecFwk::EventRunner::Create("MissionManagerHandler");
    DistributedSchedMissionManager::GetInstance().missionHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
    sptr<IRemoteObject> remote(new RemoteMissionListenerTest());
    sptr<IDistributedSched> remoteDmsObj(new DistributedSchedProxy(remote));
    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().remoteDmsLock_);
        DistributedSchedMissionManager::GetInstance().remoteDmsMap_.clear();
    }
    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().remoteDmsLock_);
        DistributedSchedMissionManager::GetInstance().remoteDmsMap_[DEVICE_ID] = remoteDmsObj;
    }
    DistributedSchedMissionManager::GetInstance().OnRemoteDmsDied(remote);
    EXPECT_EQ(DistributedSchedMissionManager::GetInstance().listenDeviceMap_.empty(), true);
    DTEST_LOG << "testOnRemoteDmsDied006 end" << std::endl;
}

/**
 * @tc.name: testOnRemoteDmsDied007
 * @tc.desc: on remote dms died
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testOnRemoteDmsDied007, TestSize.Level3)
{
    DTEST_LOG << "testOnRemoteDmsDied007 begin" << std::endl;
    auto runner = AppExecFwk::EventRunner::Create("MissionManagerHandler");
    DistributedSchedMissionManager::GetInstance().missionHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
    sptr<IRemoteObject> remote(new RemoteMissionListenerTest());
    sptr<IDistributedSched> proxy = GetDms();
    ASSERT_NE(nullptr, proxy);
    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().remoteDmsLock_);
        DistributedSchedMissionManager::GetInstance().remoteDmsMap_.clear();
    }
    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().remoteDmsLock_);
        DistributedSchedMissionManager::GetInstance().remoteDmsMap_[DEVICE_ID] = proxy;
    }
    DistributedSchedMissionManager::GetInstance().OnRemoteDmsDied(remote);
    EXPECT_EQ(DistributedSchedMissionManager::GetInstance().listenDeviceMap_.empty(), true);
    DTEST_LOG << "testOnRemoteDmsDied007 end" << std::endl;
}

/**
 * @tc.name: testOnRemoteDmsDied008
 * @tc.desc: on remote dms died
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, testOnRemoteDmsDied008, TestSize.Level3)
{
    DTEST_LOG << "testOnRemoteDmsDied008 begin" << std::endl;
    auto runner = AppExecFwk::EventRunner::Create("MissionManagerHandler");
    DistributedSchedMissionManager::GetInstance().missionHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
    sptr<IRemoteObject> remote(new RemoteMissionListenerTest());
    sptr<IDistributedSched> remoteDmsObj(new DistributedSchedProxy(remote));
    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().remoteDmsLock_);
        DistributedSchedMissionManager::GetInstance().remoteDmsMap_.clear();
    }
    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().remoteDmsLock_);
        DistributedSchedMissionManager::GetInstance().remoteDmsMap_[DEVICE_ID] = remoteDmsObj;
    }
    u16string deviceId = Str8ToStr16(DEVICE_ID);
    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().listenDeviceLock_);
        DistributedSchedMissionManager::GetInstance().listenDeviceMap_.clear();
    }
    {
        std::lock_guard<std::mutex> autoLock(DistributedSchedMissionManager::GetInstance().listenDeviceLock_);
        ListenerInfo listenerInfo;
        listenerInfo.Emplace(remote);
        listenerInfo.called = true;
        DistributedSchedMissionManager::GetInstance().listenDeviceMap_[deviceId] = listenerInfo;
    }
    DistributedSchedMissionManager::GetInstance().OnRemoteDmsDied(remote);
    EXPECT_EQ(DistributedSchedMissionManager::GetInstance().listenDeviceMap_.empty(), false);
    DTEST_LOG << "testOnRemoteDmsDied008 end" << std::endl;
}

namespace {
class DmsMainServiceChannelTestMock : public DmsMainServiceChannel {
public:
    std::shared_ptr<DmsDeviceInfo> GetDeviceInfoById(const std::string& deviceId) override
    {
        (void)deviceId;
        return nullptr;
    }
    std::string GetUuidByNetworkId(const std::string& networkId) override
    {
        (void)networkId;
        return "ut-test-uuid";
    }
    bool GetLocalDeviceId(std::string& networkId) override
    {
        (void)networkId;
        return false;
    }
    std::string GetNetworkIdByUuid(const std::string& uuid) override
    {
        (void)uuid;
        return "";
    }
    int32_t GetLocalMissionInfos(int32_t numMissions, std::vector<DstbMissionInfo>& missionInfos) override
    {
        (void)numMissions;
        (void)missionInfos;
        return ERR_OK;
    }
    int32_t RegisterMissionListener(const sptr<AAFwk::IMissionListener>& listener) override
    {
        (void)listener;
        return ERR_OK;
    }
    int32_t UnRegisterMissionListener(const sptr<AAFwk::IMissionListener>& listener) override
    {
        (void)listener;
        return ERR_OK;
    }
    int32_t GetLocalMissionSnapshotInfo(const std::string& networkId, int32_t missionId,
        AAFwk::MissionSnapshot& missionSnapshot) override
    {
        (void)networkId;
        (void)missionId;
        auto pixelMap = CreateUtPixelMap();
        if (pixelMap != nullptr) {
            missionSnapshot.snapshot = pixelMap;
            return ERR_OK;
        }
        return -1;
    }
    std::string GetAnonymStr(const std::string& value) override
    {
        return value;
    }
};
} // namespace

/**
 * @tc.name: MainServiceChannel_SetGet_001
 * @tc.desc: SetMainServiceChannel / GetMainServiceChannel round trip
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, MainServiceChannel_SetGet_001, TestSize.Level3)
{
    DTEST_LOG << "MainServiceChannel_SetGet_001 begin" << std::endl;
    auto mockCh = std::make_shared<DmsMainServiceChannelTestMock>();
    DistributedSchedMissionManager::GetInstance().SetMainServiceChannel(mockCh);
    EXPECT_EQ(DistributedSchedMissionManager::GetInstance().GetMainServiceChannel(), mockCh);
    DTEST_LOG << "MainServiceChannel_SetGet_001 end" << std::endl;
}

/**
 * @tc.name: IsDeviceIdValidated_NoDeviceInfo_001
 * @tc.desc: IsDeviceIdValidated returns false when channel returns no device info
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, IsDeviceIdValidated_NoDeviceInfo_001, TestSize.Level3)
{
    DTEST_LOG << "IsDeviceIdValidated_NoDeviceInfo_001 begin" << std::endl;
    DistributedSchedMissionManager::GetInstance().Init();
    auto mockCh = std::make_shared<DmsMainServiceChannelTestMock>();
    DistributedSchedMissionManager::GetInstance().SetMainServiceChannel(mockCh);
    bool ret = DistributedSchedMissionManager::GetInstance().IsDeviceIdValidated(DEVICE_ID);
    EXPECT_FALSE(ret);
    DTEST_LOG << "IsDeviceIdValidated_NoDeviceInfo_001 end" << std::endl;
}

/**
 * @tc.name: NotifyNetDisconnectOffline_EmptyListeners_001
 * @tc.desc: NotifyNetDisconnectOffline when listenDeviceMap_ is empty
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, NotifyNetDisconnectOffline_EmptyListeners_001, TestSize.Level3)
{
    DTEST_LOG << "NotifyNetDisconnectOffline_EmptyListeners_001 begin" << std::endl;
    DistributedSchedMissionManager::GetInstance().Init();
    {
        std::lock_guard<std::mutex> lock(DistributedSchedMissionManager::GetInstance().listenDeviceLock_);
        DistributedSchedMissionManager::GetInstance().listenDeviceMap_.clear();
    }
    EXPECT_NO_FATAL_FAILURE(DistributedSchedMissionManager::GetInstance().NotifyNetDisconnectOffline());
    DTEST_LOG << "NotifyNetDisconnectOffline_EmptyListeners_001 end" << std::endl;
}

/**
 * @tc.name: NotifyRemoteDied_DataStorageNull_001
 * @tc.desc: NotifyRemoteDied returns early when distributedDataStorage_ is null
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, NotifyRemoteDied_DataStorageNull_001, TestSize.Level3)
{
    DTEST_LOG << "NotifyRemoteDied_DataStorageNull_001 begin" << std::endl;
    DistributedSchedMissionManager::GetInstance().Init();
    auto prev = DistributedSchedMissionManager::GetInstance().distributedDataStorage_;
    DistributedSchedMissionManager::GetInstance().distributedDataStorage_ = nullptr;
    wptr<IRemoteObject> remote;
    EXPECT_NO_FATAL_FAILURE(DistributedSchedMissionManager::GetInstance().NotifyRemoteDied(remote));
    DistributedSchedMissionManager::GetInstance().distributedDataStorage_ = prev;
    DTEST_LOG << "NotifyRemoteDied_DataStorageNull_001 end" << std::endl;
}

/**
 * @tc.name: ListenerInfo_MapOperations_001
 * @tc.desc: ListenerInfo Emplace / Find / Erase / Size / Empty
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, ListenerInfo_MapOperations_001, TestSize.Level3)
{
    DTEST_LOG << "ListenerInfo_MapOperations_001 begin" << std::endl;
    ListenerInfo info;
    EXPECT_TRUE(info.Empty());
    sptr<IRemoteObject> listener(new RemoteMissionListenerTest());
    EXPECT_TRUE(info.Emplace(listener));
    EXPECT_FALSE(info.Emplace(listener));
    EXPECT_TRUE(info.Find(listener));
    EXPECT_EQ(info.Size(), 1);
    info.Erase(listener);
    EXPECT_TRUE(info.Empty());
    DTEST_LOG << "ListenerInfo_MapOperations_001 end" << std::endl;
}

/**
 * @tc.name: GetOsAccountData_Invoke_001
 * @tc.desc: Invoke GetOsAccountData (account path may be ifdef-gated)
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, GetOsAccountData_Invoke_001, TestSize.Level3)
{
    DTEST_LOG << "GetOsAccountData_Invoke_001 begin" << std::endl;
    IDistributedSched::AccountInfo accountInfo;
    bool ret = DistributedSchedMissionManager::GetInstance().GetOsAccountData(accountInfo);
    EXPECT_TRUE(ret);
    DTEST_LOG << "GetOsAccountData_Invoke_001 end" << std::endl;
}

namespace {
const std::string UT_MISSION_LOCAL_NETWORK_ID = "ut-mission-local-network-id";
const std::string UT_MISSION_REMOTE_NETWORK_ID = "ut-mission-remote-network-id";
const std::string UT_MISSION_REMOTE_UUID = "ut-mission-remote-uuid";
const std::string UT_MISSION_OTHER_UUID = "ut-mission-other-uuid";
constexpr int32_t UT_MISSION_ID_FIRST = 1001;
constexpr int32_t UT_MISSION_ID_SECOND = 1002;
constexpr int32_t UT_MISSION_ID_THIRD = 1003;
constexpr int32_t UT_SNAPSHOT_MISSION_ID_FIRST = 2001;
constexpr int32_t UT_SNAPSHOT_MISSION_ID_SECOND = 2002;
constexpr int32_t UT_CHANNEL_FAIL_RET = -1;
constexpr size_t UT_CACHED_MISSION_COUNT = 3;
constexpr size_t UT_SNAPSHOT_CACHE_ENTRY_COUNT = 3;
constexpr int32_t UT_SNAPSHOT_NULL_INDEX = 5;
constexpr int32_t UT_ACCOUNT_USER_ID = 100;
constexpr size_t UT_SNAPSHOT_CACHE_LIMIT = 10;
constexpr size_t UT_SNAPSHOT_CACHE_FILL_COUNT = 10;

/**
 * Configurable DmsMainServiceChannel for driving DistributedSchedMissionManager branches.
 * Every return value can be preset, so the tests do not depend on real device storage.
 */
class DmsMissionUtBranchChannel : public DmsMainServiceChannel {
public:
    std::shared_ptr<DmsDeviceInfo> GetDeviceInfoById(const std::string& deviceId) override
    {
        if (!deviceOnline_) {
            return nullptr;
        }
        return std::make_shared<DmsDeviceInfo>("ut-mission-device", 0, deviceId);
    }

    std::string GetUuidByNetworkId(const std::string& networkId) override
    {
        (void)networkId;
        uuidQueryCount_.fetch_add(1);
        return uuid_;
    }

    bool GetLocalDeviceId(std::string& networkId) override
    {
        localDeviceIdCount_.fetch_add(1);
        if (!getLocalDeviceIdRet_) {
            return false;
        }
        networkId = localDeviceId_;
        return true;
    }

    std::string GetNetworkIdByUuid(const std::string& uuid) override
    {
        (void)uuid;
        return UT_MISSION_REMOTE_NETWORK_ID;
    }

    int32_t GetLocalMissionInfos(int32_t numMissions, std::vector<DstbMissionInfo>& missionInfos) override
    {
        (void)numMissions;
        localMissionInfosCount_.fetch_add(1);
        if (getLocalMissionInfosRet_ != ERR_OK) {
            return getLocalMissionInfosRet_;
        }
        missionInfos = localMissionInfos_;
        return ERR_OK;
    }

    int32_t RegisterMissionListener(const sptr<AAFwk::IMissionListener>& listener) override
    {
        (void)listener;
        registerCount_.fetch_add(1);
        return registerRet_;
    }

    int32_t UnRegisterMissionListener(const sptr<AAFwk::IMissionListener>& listener) override
    {
        (void)listener;
        unregisterCount_.fetch_add(1);
        return unregisterRet_;
    }

    int32_t GetLocalMissionSnapshotInfo(const std::string& networkId, int32_t missionId,
        AAFwk::MissionSnapshot& missionSnapshot) override
    {
        (void)networkId;
        (void)missionId;
        snapshotQueryCount_.fetch_add(1);
        if (getLocalMissionSnapshotRet_ != ERR_OK) {
            return getLocalMissionSnapshotRet_;
        }
        missionSnapshot.snapshot = CreateUtPixelMap();
        return ERR_OK;
    }

    std::string GetAnonymStr(const std::string& value) override
    {
        return value;
    }

    bool deviceOnline_ = false;
    std::string uuid_ = UT_MISSION_REMOTE_UUID;
    std::string localDeviceId_ = UT_MISSION_LOCAL_NETWORK_ID;
    bool getLocalDeviceIdRet_ = true;
    int32_t getLocalMissionInfosRet_ = ERR_OK;
    std::vector<DstbMissionInfo> localMissionInfos_;
    int32_t registerRet_ = ERR_OK;
    int32_t unregisterRet_ = ERR_OK;
    int32_t getLocalMissionSnapshotRet_ = ERR_OK;
    std::atomic<int32_t> uuidQueryCount_ { 0 };
    std::atomic<int32_t> localDeviceIdCount_ { 0 };
    std::atomic<int32_t> localMissionInfosCount_ { 0 };
    std::atomic<int32_t> registerCount_ { 0 };
    std::atomic<int32_t> unregisterCount_ { 0 };
    std::atomic<int32_t> snapshotQueryCount_ { 0 };
};

/**
 * Clear the singleton state touched by the mission manager tests, so every case starts clean.
 */
void ResetMissionManagerUtState()
{
    auto& mgr = DistributedSchedMissionManager::GetInstance();
    {
        std::lock_guard<std::mutex> autoLock(mgr.listenDeviceLock_);
        mgr.listenDeviceMap_.clear();
        mgr.cachedSnapshotInfos_.clear();
    }
    {
        std::lock_guard<std::mutex> autoLock(mgr.remoteSyncDeviceLock_);
        mgr.remoteSyncDeviceSet_.clear();
    }
    {
        std::lock_guard<std::mutex> autoLock(mgr.remoteMissionInfosLock_);
        mgr.deviceMissionInfos_.clear();
    }
    {
        std::lock_guard<std::mutex> autoLock(mgr.remoteDmsLock_);
        mgr.remoteDmsMap_.clear();
    }
    mgr.deviceHandle_.clear();
    mgr.missionHandler_ = nullptr;
    mgr.missionChangeHandler_ = nullptr;
    mgr.distributedDataStorage_ = nullptr;
    mgr.isRegMissionChange_ = false;
}

std::shared_ptr<DmsMissionUtBranchChannel> PrepareMissionUtChannel(bool deviceOnline = true)
{
    ResetMissionManagerUtState();
    auto channel = std::make_shared<DmsMissionUtBranchChannel>();
    channel->deviceOnline_ = deviceOnline;
    BindUtMainServiceChannel(channel);
    g_missionUtCaseActive = true;
    return channel;
}

DstbMissionInfo CreateUtDstbMissionInfo(int32_t missionId)
{
    DstbMissionInfo missionInfo;
    missionInfo.id = missionId;
    missionInfo.label = "ut-mission-label-" + std::to_string(missionId);
    missionInfo.baseWant = std::make_shared<AAFwk::Want>();
    return missionInfo;
}

std::vector<DstbMissionInfo> CreateUtDstbMissionInfos()
{
    return {
        CreateUtDstbMissionInfo(UT_MISSION_ID_FIRST),
        CreateUtDstbMissionInfo(UT_MISSION_ID_SECOND),
        CreateUtDstbMissionInfo(UT_MISSION_ID_THIRD),
    };
}

std::unique_ptr<Snapshot> CreateUtSnapshotWithoutPixelMap()
{
    auto snapshot = std::make_unique<Snapshot>();
    snapshot->rect_ = std::make_unique<Rect>(0, 0, 0, 0);
    snapshot->windowBounds_ = std::make_unique<Rect>(0, 0, 0, 0);
    return snapshot;
}

std::unique_ptr<Snapshot> CreateUtSnapshotWithPixelMap()
{
    auto snapshot = CreateUtSnapshotWithoutPixelMap();
    snapshot->pixelMap_ = CreateUtPixelMap();
    return snapshot;
}

sptr<IDistributedSched> CreateUtMockRemoteDms()
{
    return new MockDistributedSched();
}

std::string CreateUtSnapshotKey(const std::string& deviceId, int32_t missionId)
{
    return deviceId + "_" + std::to_string(missionId);
}

bool HasUtSnapshotCacheEntry(const std::string& key)
{
    auto& mgr = DistributedSchedMissionManager::GetInstance();
    return mgr.cachedSnapshotInfos_.find(key) != mgr.cachedSnapshotInfos_.end();
}

bool IsUtSnapshotCacheEntryNull(const std::string& key)
{
    auto& mgr = DistributedSchedMissionManager::GetInstance();
    auto iter = mgr.cachedSnapshotInfos_.find(key);
    return iter != mgr.cachedSnapshotInfos_.end() && iter->second == nullptr;
}
} // namespace

/**
 * @tc.name: GetMissionInfos_002
 * @tc.desc: GetMissionInfos returns invalid parameters when deviceId is empty
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, GetMissionInfos_002, TestSize.Level3)
{
    DTEST_LOG << "GetMissionInfos_002 begin" << std::endl;
    PrepareMissionUtChannel();
    std::vector<AAFwk::MissionInfo> missionInfos;
    auto ret = DistributedSchedMissionManager::GetInstance().GetMissionInfos("", NORMAL_NUM_MISSIONS, missionInfos);
    EXPECT_EQ(ret, INVALID_PARAMETERS_ERR);
    EXPECT_TRUE(missionInfos.empty());
    DTEST_LOG << "GetMissionInfos_002 end" << std::endl;
}

/**
 * @tc.name: GetMissionInfos_003
 * @tc.desc: GetMissionInfos returns invalid parameters when numMissions is not positive
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, GetMissionInfos_003, TestSize.Level3)
{
    DTEST_LOG << "GetMissionInfos_003 begin" << std::endl;
    PrepareMissionUtChannel();
    auto& mgr = DistributedSchedMissionManager::GetInstance();
    std::vector<AAFwk::MissionInfo> missionInfos;
    EXPECT_EQ(mgr.GetMissionInfos(DEVICE_ID, 0, missionInfos), INVALID_PARAMETERS_ERR);
    EXPECT_EQ(mgr.GetMissionInfos(DEVICE_ID, -1, missionInfos), INVALID_PARAMETERS_ERR);
    EXPECT_TRUE(missionInfos.empty());
    DTEST_LOG << "GetMissionInfos_003 end" << std::endl;
}

/**
 * @tc.name: GetMissionInfos_004
 * @tc.desc: GetMissionInfos returns invalid parameters when uuid of the device is empty
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, GetMissionInfos_004, TestSize.Level3)
{
    DTEST_LOG << "GetMissionInfos_004 begin" << std::endl;
    auto channel = PrepareMissionUtChannel();
    channel->uuid_.clear();
    std::vector<AAFwk::MissionInfo> missionInfos;
    auto ret = DistributedSchedMissionManager::GetInstance().GetMissionInfos(DEVICE_ID, NORMAL_NUM_MISSIONS,
        missionInfos);
    EXPECT_EQ(ret, INVALID_PARAMETERS_ERR);
    EXPECT_TRUE(missionInfos.empty());
    DTEST_LOG << "GetMissionInfos_004 end" << std::endl;
}

/**
 * @tc.name: GetMissionInfos_005
 * @tc.desc: GetMissionInfos returns null object when the device has no cached missions
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, GetMissionInfos_005, TestSize.Level3)
{
    DTEST_LOG << "GetMissionInfos_005 begin" << std::endl;
    PrepareMissionUtChannel();
    std::vector<AAFwk::MissionInfo> missionInfos;
    auto ret = DistributedSchedMissionManager::GetInstance().GetMissionInfos(DEVICE_ID, NORMAL_NUM_MISSIONS,
        missionInfos);
    EXPECT_EQ(ret, ERR_NULL_OBJECT);
    EXPECT_TRUE(missionInfos.empty());
    DTEST_LOG << "GetMissionInfos_005 end" << std::endl;
}

/**
 * @tc.name: GetMissionInfos_001
 * @tc.desc: GetMissionInfos converts all cached missions when numMissions is larger than the cache
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, GetMissionInfos_001, TestSize.Level3)
{
    DTEST_LOG << "GetMissionInfos_001 begin" << std::endl;
    PrepareMissionUtChannel();
    auto& mgr = DistributedSchedMissionManager::GetInstance();
    auto dstbMissionInfos = CreateUtDstbMissionInfos();
    mgr.RebornMissionCache(DEVICE_ID, dstbMissionInfos);
    std::vector<AAFwk::MissionInfo> missionInfos;
    auto ret = mgr.GetMissionInfos(DEVICE_ID, NORMAL_NUM_MISSIONS, missionInfos);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(missionInfos.size(), dstbMissionInfos.size());
    if (missionInfos.size() == dstbMissionInfos.size()) {
        EXPECT_EQ(missionInfos[0].id, UT_MISSION_ID_FIRST);
        EXPECT_EQ(missionInfos[1].id, UT_MISSION_ID_SECOND);
        EXPECT_EQ(missionInfos[2].id, UT_MISSION_ID_THIRD);
    }
    DTEST_LOG << "GetMissionInfos_001 end" << std::endl;
}

/**
 * @tc.name: GetMissionInfos_006
 * @tc.desc: GetMissionInfos returns at most numMissions missions
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, GetMissionInfos_006, TestSize.Level3)
{
    DTEST_LOG << "GetMissionInfos_006 begin" << std::endl;
    PrepareMissionUtChannel();
    auto& mgr = DistributedSchedMissionManager::GetInstance();
    mgr.RebornMissionCache(DEVICE_ID, CreateUtDstbMissionInfos());
    std::vector<AAFwk::MissionInfo> missionInfos;
    auto ret = mgr.GetMissionInfos(DEVICE_ID, 2, missionInfos);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(missionInfos.size(), 2u);
    if (missionInfos.size() == 2u) {
        EXPECT_EQ(missionInfos[0].id, UT_MISSION_ID_FIRST);
        EXPECT_EQ(missionInfos[1].id, UT_MISSION_ID_SECOND);
    }
    DTEST_LOG << "GetMissionInfos_006 end" << std::endl;
}

/**
 * @tc.name: FetchCachedRemoteMissions_002
 * @tc.desc: FetchCachedRemoteMissions returns invalid parameters when uuid is empty
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, FetchCachedRemoteMissions_002, TestSize.Level3)
{
    DTEST_LOG << "FetchCachedRemoteMissions_002 begin" << std::endl;
    auto channel = PrepareMissionUtChannel();
    channel->uuid_.clear();
    std::vector<DstbMissionInfo> missionInfos;
    auto ret = DistributedSchedMissionManager::GetInstance().FetchCachedRemoteMissions(DEVICE_ID,
        NORMAL_NUM_MISSIONS, missionInfos);
    EXPECT_EQ(ret, INVALID_PARAMETERS_ERR);
    DTEST_LOG << "FetchCachedRemoteMissions_002 end" << std::endl;
}

/**
 * @tc.name: FetchCachedRemoteMissions_001
 * @tc.desc: FetchCachedRemoteMissions returns all missions when numMissions is larger than the cache size
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, FetchCachedRemoteMissions_001, TestSize.Level3)
{
    DTEST_LOG << "FetchCachedRemoteMissions_001 begin" << std::endl;
    PrepareMissionUtChannel();
    auto& mgr = DistributedSchedMissionManager::GetInstance();
    mgr.RebornMissionCache(DEVICE_ID, CreateUtDstbMissionInfos());
    std::vector<DstbMissionInfo> missionInfos;
    auto ret = mgr.FetchCachedRemoteMissions(DEVICE_ID, NUM_MISSIONS, missionInfos);
    EXPECT_EQ(ret, ERR_NONE);
    EXPECT_EQ(missionInfos.size(), UT_CACHED_MISSION_COUNT);
    DTEST_LOG << "FetchCachedRemoteMissions_001 end" << std::endl;
}

/**
 * @tc.name: RebornMissionCache_002
 * @tc.desc: RebornMissionCache skips caching when uuid is empty
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, RebornMissionCache_002, TestSize.Level3)
{
    DTEST_LOG << "RebornMissionCache_002 begin" << std::endl;
    auto channel = PrepareMissionUtChannel();
    channel->uuid_.clear();
    auto& mgr = DistributedSchedMissionManager::GetInstance();
    mgr.RebornMissionCache(DEVICE_ID, CreateUtDstbMissionInfos());
    EXPECT_TRUE(mgr.deviceMissionInfos_.empty());
    DTEST_LOG << "RebornMissionCache_002 end" << std::endl;
}

/**
 * @tc.name: RebornMissionCache_001
 * @tc.desc: RebornMissionCache replaces the missions cached for the same uuid
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, RebornMissionCache_001, TestSize.Level3)
{
    DTEST_LOG << "RebornMissionCache_001 begin" << std::endl;
    PrepareMissionUtChannel();
    auto& mgr = DistributedSchedMissionManager::GetInstance();
    mgr.RebornMissionCache(DEVICE_ID, CreateUtDstbMissionInfos());
    EXPECT_EQ(mgr.deviceMissionInfos_.size(), 1u);
    auto iter = mgr.deviceMissionInfos_.find(UT_MISSION_REMOTE_UUID);
    ASSERT_TRUE(iter != mgr.deviceMissionInfos_.end());
    EXPECT_EQ(iter->second.size(), UT_CACHED_MISSION_COUNT);

    mgr.RebornMissionCache(DEVICE_ID, std::vector<DstbMissionInfo>());
    EXPECT_EQ(mgr.deviceMissionInfos_.size(), 1u);
    iter = mgr.deviceMissionInfos_.find(UT_MISSION_REMOTE_UUID);
    ASSERT_TRUE(iter != mgr.deviceMissionInfos_.end());
    EXPECT_TRUE(iter->second.empty());
    DTEST_LOG << "RebornMissionCache_001 end" << std::endl;
}

/**
 * @tc.name: CleanMissionCache_002
 * @tc.desc: CleanMissionCache keeps the cache untouched when uuid is empty
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, CleanMissionCache_002, TestSize.Level3)
{
    DTEST_LOG << "CleanMissionCache_002 begin" << std::endl;
    auto channel = PrepareMissionUtChannel();
    auto& mgr = DistributedSchedMissionManager::GetInstance();
    mgr.RebornMissionCache(DEVICE_ID, CreateUtDstbMissionInfos());
    EXPECT_EQ(mgr.deviceMissionInfos_.size(), 1u);

    channel->uuid_.clear();
    mgr.CleanMissionCache(DEVICE_ID);
    EXPECT_EQ(mgr.deviceMissionInfos_.size(), 1u);
    DTEST_LOG << "CleanMissionCache_002 end" << std::endl;
}

/**
 * @tc.name: CleanMissionCache_001
 * @tc.desc: CleanMissionCache removes the missions cached for the uuid
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, CleanMissionCache_001, TestSize.Level3)
{
    DTEST_LOG << "CleanMissionCache_001 begin" << std::endl;
    PrepareMissionUtChannel();
    auto& mgr = DistributedSchedMissionManager::GetInstance();
    mgr.RebornMissionCache(DEVICE_ID, CreateUtDstbMissionInfos());
    EXPECT_EQ(mgr.deviceMissionInfos_.size(), 1u);

    mgr.CleanMissionCache(DEVICE_ID);
    EXPECT_TRUE(mgr.deviceMissionInfos_.empty());
    DTEST_LOG << "CleanMissionCache_001 end" << std::endl;
}

/**
 * @tc.name: IsDeviceIdValidated_001
 * @tc.desc: IsDeviceIdValidated returns false when the channel does not know the device
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, IsDeviceIdValidated_001, TestSize.Level3)
{
    DTEST_LOG << "IsDeviceIdValidated_001 begin" << std::endl;
    auto channel = PrepareMissionUtChannel(false);
    channel->uuid_ = UT_MISSION_REMOTE_UUID;
    EXPECT_FALSE(DistributedSchedMissionManager::GetInstance().IsDeviceIdValidated(DEVICE_ID));
    DTEST_LOG << "IsDeviceIdValidated_001 end" << std::endl;
}

/**
 * @tc.name: GetRemoteDms_001
 * @tc.desc: GetRemoteDms returns nullptr when deviceId is empty
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, GetRemoteDms_001, TestSize.Level3)
{
    DTEST_LOG << "GetRemoteDms_001 begin" << std::endl;
    PrepareMissionUtChannel();
    EXPECT_TRUE(DistributedSchedMissionManager::GetInstance().GetRemoteDms("") == nullptr);
    DTEST_LOG << "GetRemoteDms_001 end" << std::endl;
}

/**
 * @tc.name: EnqueueCachedSnapshotInfo_002
 * @tc.desc: EnqueueCachedSnapshotInfo keeps the entry when the cached snapshot is null
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, EnqueueCachedSnapshotInfo_002, TestSize.Level3)
{
    DTEST_LOG << "EnqueueCachedSnapshotInfo_002 begin" << std::endl;
    PrepareMissionUtChannel();
    auto& mgr = DistributedSchedMissionManager::GetInstance();
    const std::string key = CreateUtSnapshotKey(DEVICE_ID, UT_SNAPSHOT_MISSION_ID_FIRST);
    {
        std::lock_guard<std::mutex> autoLock(mgr.listenDeviceLock_);
        mgr.cachedSnapshotInfos_[key] = nullptr;
    }
    mgr.EnqueueCachedSnapshotInfo(DEVICE_ID, UT_SNAPSHOT_MISSION_ID_FIRST, CreateUtSnapshotWithPixelMap());
    EXPECT_TRUE(IsUtSnapshotCacheEntryNull(key));
    EXPECT_EQ(mgr.cachedSnapshotInfos_.size(), 1u);
    DTEST_LOG << "EnqueueCachedSnapshotInfo_002 end" << std::endl;
}

/**
 * @tc.name: EnqueueCachedSnapshotInfo_001
 * @tc.desc: EnqueueCachedSnapshotInfo replaces the snapshot cached for the same key
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, EnqueueCachedSnapshotInfo_001, TestSize.Level3)
{
    DTEST_LOG << "EnqueueCachedSnapshotInfo_001 begin" << std::endl;
    PrepareMissionUtChannel();
    auto& mgr = DistributedSchedMissionManager::GetInstance();
    mgr.EnqueueCachedSnapshotInfo(DEVICE_ID, UT_SNAPSHOT_MISSION_ID_FIRST, CreateUtSnapshotWithoutPixelMap());
    mgr.EnqueueCachedSnapshotInfo(DEVICE_ID, UT_SNAPSHOT_MISSION_ID_FIRST, CreateUtSnapshotWithPixelMap());
    EXPECT_EQ(mgr.cachedSnapshotInfos_.size(), 1u);
    auto cachedSnapshot = mgr.DequeueCachedSnapshotInfo(DEVICE_ID, UT_SNAPSHOT_MISSION_ID_FIRST);
    ASSERT_TRUE(cachedSnapshot != nullptr);
    EXPECT_NE(cachedSnapshot->pixelMap_.get(), nullptr);
    DTEST_LOG << "EnqueueCachedSnapshotInfo_001 end" << std::endl;
}

/**
 * @tc.name: EnqueueCachedSnapshotInfo_003
 * @tc.desc: EnqueueCachedSnapshotInfo evicts the oldest snapshot when the cache is full
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, EnqueueCachedSnapshotInfo_003, TestSize.Level3)
{
    DTEST_LOG << "EnqueueCachedSnapshotInfo_003 begin" << std::endl;
    PrepareMissionUtChannel();
    auto& mgr = DistributedSchedMissionManager::GetInstance();
    for (size_t index = 1; index <= UT_SNAPSHOT_CACHE_FILL_COUNT; ++index) {
        const int32_t missionId = static_cast<int32_t>(index);
        std::string indexStr = std::to_string(index);
        if (index < UT_SNAPSHOT_CACHE_FILL_COUNT) {
            indexStr = "0" + indexStr;
        }
        mgr.EnqueueCachedSnapshotInfo("ut-mission-full-" + indexStr, missionId,
            CreateUtSnapshotWithoutPixelMap());
    }
    EXPECT_EQ(mgr.cachedSnapshotInfos_.size(), UT_SNAPSHOT_CACHE_LIMIT);
    const std::string evictedKey = CreateUtSnapshotKey("ut-mission-full-01", 1);
    const std::string nullKey = CreateUtSnapshotKey(
        "ut-mission-full-0" + std::to_string(UT_SNAPSHOT_NULL_INDEX), UT_SNAPSHOT_NULL_INDEX);
    {
        std::lock_guard<std::mutex> autoLock(mgr.listenDeviceLock_);
        mgr.cachedSnapshotInfos_[nullKey] = nullptr;
    }
    EXPECT_TRUE(IsUtSnapshotCacheEntryNull(nullKey));

    mgr.EnqueueCachedSnapshotInfo("ut-mission-new", UT_SNAPSHOT_MISSION_ID_SECOND, CreateUtSnapshotWithPixelMap());
    EXPECT_EQ(mgr.cachedSnapshotInfos_.size(), UT_SNAPSHOT_CACHE_LIMIT);
    EXPECT_FALSE(HasUtSnapshotCacheEntry(evictedKey));
    EXPECT_TRUE(HasUtSnapshotCacheEntry(CreateUtSnapshotKey("ut-mission-new", UT_SNAPSHOT_MISSION_ID_SECOND)));
    DTEST_LOG << "EnqueueCachedSnapshotInfo_003 end" << std::endl;
}

/**
 * @tc.name: DequeueCachedSnapshotInfo_002
 * @tc.desc: DequeueCachedSnapshotInfo returns nullptr when the cached snapshot is null
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, DequeueCachedSnapshotInfo_002, TestSize.Level3)
{
    DTEST_LOG << "DequeueCachedSnapshotInfo_002 begin" << std::endl;
    PrepareMissionUtChannel();
    auto& mgr = DistributedSchedMissionManager::GetInstance();
    const std::string key = CreateUtSnapshotKey(DEVICE_ID, UT_SNAPSHOT_MISSION_ID_FIRST);
    {
        std::lock_guard<std::mutex> autoLock(mgr.listenDeviceLock_);
        mgr.cachedSnapshotInfos_[key] = nullptr;
    }
    EXPECT_EQ(mgr.DequeueCachedSnapshotInfo(DEVICE_ID, UT_SNAPSHOT_MISSION_ID_FIRST).get(), nullptr);
    DTEST_LOG << "DequeueCachedSnapshotInfo_002 end" << std::endl;
}

/**
 * @tc.name: DequeueCachedSnapshotInfo_001
 * @tc.desc: DequeueCachedSnapshotInfo returns the cached snapshot only once
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, DequeueCachedSnapshotInfo_001, TestSize.Level3)
{
    DTEST_LOG << "DequeueCachedSnapshotInfo_001 begin" << std::endl;
    PrepareMissionUtChannel();
    auto& mgr = DistributedSchedMissionManager::GetInstance();
    mgr.EnqueueCachedSnapshotInfo(DEVICE_ID, UT_SNAPSHOT_MISSION_ID_FIRST, CreateUtSnapshotWithPixelMap());
    auto cachedSnapshot = mgr.DequeueCachedSnapshotInfo(DEVICE_ID, UT_SNAPSHOT_MISSION_ID_FIRST);
    ASSERT_TRUE(cachedSnapshot != nullptr);
    EXPECT_NE(cachedSnapshot->pixelMap_.get(), nullptr);
    EXPECT_TRUE(mgr.cachedSnapshotInfos_.empty());
    EXPECT_EQ(mgr.DequeueCachedSnapshotInfo(DEVICE_ID, UT_SNAPSHOT_MISSION_ID_FIRST).get(), nullptr);
    DTEST_LOG << "DequeueCachedSnapshotInfo_001 end" << std::endl;
}

/**
 * @tc.name: DeleteCachedSnapshotInfo_002
 * @tc.desc: DeleteCachedSnapshotInfo keeps the cache untouched when networkId is empty
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, DeleteCachedSnapshotInfo_002, TestSize.Level3)
{
    DTEST_LOG << "DeleteCachedSnapshotInfo_002 begin" << std::endl;
    PrepareMissionUtChannel();
    auto& mgr = DistributedSchedMissionManager::GetInstance();
    mgr.EnqueueCachedSnapshotInfo(DEVICE_ID, UT_SNAPSHOT_MISSION_ID_FIRST, CreateUtSnapshotWithoutPixelMap());
    mgr.DeleteCachedSnapshotInfo("");
    EXPECT_EQ(mgr.cachedSnapshotInfos_.size(), 1u);
    DTEST_LOG << "DeleteCachedSnapshotInfo_002 end" << std::endl;
}

/**
 * @tc.name: DeleteCachedSnapshotInfo_001
 * @tc.desc: DeleteCachedSnapshotInfo removes only the snapshots belonging to the uuid
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, DeleteCachedSnapshotInfo_001, TestSize.Level3)
{
    DTEST_LOG << "DeleteCachedSnapshotInfo_001 begin" << std::endl;
    PrepareMissionUtChannel();
    auto& mgr = DistributedSchedMissionManager::GetInstance();
    mgr.EnqueueCachedSnapshotInfo(UT_MISSION_REMOTE_UUID, UT_SNAPSHOT_MISSION_ID_FIRST,
        CreateUtSnapshotWithoutPixelMap());
    mgr.EnqueueCachedSnapshotInfo(UT_MISSION_REMOTE_UUID, UT_SNAPSHOT_MISSION_ID_SECOND,
        CreateUtSnapshotWithoutPixelMap());
    mgr.EnqueueCachedSnapshotInfo(UT_MISSION_OTHER_UUID, UT_SNAPSHOT_MISSION_ID_FIRST,
        CreateUtSnapshotWithoutPixelMap());
    EXPECT_EQ(mgr.cachedSnapshotInfos_.size(), UT_SNAPSHOT_CACHE_ENTRY_COUNT);

    mgr.DeleteCachedSnapshotInfo(DEVICE_ID);
    EXPECT_EQ(mgr.cachedSnapshotInfos_.size(), 1u);
    EXPECT_TRUE(HasUtSnapshotCacheEntry(CreateUtSnapshotKey(UT_MISSION_OTHER_UUID, UT_SNAPSHOT_MISSION_ID_FIRST)));
    DTEST_LOG << "DeleteCachedSnapshotInfo_001 end" << std::endl;
}

/**
 * @tc.name: GetRemoteMissionSnapshotInfo_002
 * @tc.desc: GetRemoteMissionSnapshotInfo returns invalid parameters when uuid is empty
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, GetRemoteMissionSnapshotInfo_002, TestSize.Level3)
{
    DTEST_LOG << "GetRemoteMissionSnapshotInfo_002 begin" << std::endl;
    auto channel = PrepareMissionUtChannel();
    channel->uuid_.clear();
    std::unique_ptr<AAFwk::MissionSnapshot> missionSnapshot;
    auto ret = DistributedSchedMissionManager::GetInstance().GetRemoteMissionSnapshotInfo(DEVICE_ID,
        UT_SNAPSHOT_MISSION_ID_FIRST, missionSnapshot);
    EXPECT_EQ(ret, INVALID_PARAMETERS_ERR);
    DTEST_LOG << "GetRemoteMissionSnapshotInfo_002 end" << std::endl;
}

/**
 * @tc.name: GetRemoteMissionSnapshotInfo_001
 * @tc.desc: GetRemoteMissionSnapshotInfo returns the cached snapshot without touching the storage
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, GetRemoteMissionSnapshotInfo_001, TestSize.Level3)
{
    DTEST_LOG << "GetRemoteMissionSnapshotInfo_001 begin" << std::endl;
    PrepareMissionUtChannel();
    auto& mgr = DistributedSchedMissionManager::GetInstance();
    mgr.EnqueueCachedSnapshotInfo(UT_MISSION_REMOTE_UUID, UT_SNAPSHOT_MISSION_ID_FIRST,
        CreateUtSnapshotWithPixelMap());
    std::unique_ptr<AAFwk::MissionSnapshot> missionSnapshot = std::make_unique<AAFwk::MissionSnapshot>();
    auto ret = mgr.GetRemoteMissionSnapshotInfo(DEVICE_ID, UT_SNAPSHOT_MISSION_ID_FIRST, missionSnapshot);
    EXPECT_EQ(ret, ERR_NONE);
    ASSERT_TRUE(missionSnapshot != nullptr);
    EXPECT_NE(missionSnapshot->snapshot.get(), nullptr);
    EXPECT_TRUE(mgr.cachedSnapshotInfos_.empty());
    DTEST_LOG << "GetRemoteMissionSnapshotInfo_001 end" << std::endl;
}

/**
 * @tc.name: GetRemoteMissionSnapshotInfo_003
 * @tc.desc: GetRemoteMissionSnapshotInfo returns null object on cache miss without data storage
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, GetRemoteMissionSnapshotInfo_003, TestSize.Level3)
{
    DTEST_LOG << "GetRemoteMissionSnapshotInfo_003 begin" << std::endl;
    PrepareMissionUtChannel();
    std::unique_ptr<AAFwk::MissionSnapshot> missionSnapshot;
    auto ret = DistributedSchedMissionManager::GetInstance().GetRemoteMissionSnapshotInfo(DEVICE_ID,
        UT_SNAPSHOT_MISSION_ID_FIRST, missionSnapshot);
    EXPECT_EQ(ret, ERR_NULL_OBJECT);
    DTEST_LOG << "GetRemoteMissionSnapshotInfo_003 end" << std::endl;
}

/**
 * @tc.name: MissionSnapshotSequence_001
 * @tc.desc: MissionSnapshotSequence writes both the snapshot info and the pixel map
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, MissionSnapshotSequence_001, TestSize.Level3)
{
    DTEST_LOG << "MissionSnapshotSequence_001 begin" << std::endl;
    PrepareMissionUtChannel();
    Snapshot snapshot;
    snapshot.rect_ = std::make_unique<Rect>(0, 0, 0, 0);
    snapshot.windowBounds_ = std::make_unique<Rect>(0, 0, 0, 0);
    snapshot.pixelMap_ = CreateUtPixelMap();
    MessageParcel data;
    auto ret = DistributedSchedMissionManager::GetInstance().MissionSnapshotSequence(snapshot, data);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_GT(data.GetReadableBytes(), 0u);
    DTEST_LOG << "MissionSnapshotSequence_001 end" << std::endl;
}

/**
 * @tc.name: MissionSnapshotChanged_002
 * @tc.desc: MissionSnapshotChanged returns invalid parameters when the local device id is unavailable
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, MissionSnapshotChanged_002, TestSize.Level3)
{
    DTEST_LOG << "MissionSnapshotChanged_002 begin" << std::endl;
    auto channel = PrepareMissionUtChannel();
    channel->getLocalDeviceIdRet_ = false;
    auto ret = DistributedSchedMissionManager::GetInstance().MissionSnapshotChanged(UT_SNAPSHOT_MISSION_ID_FIRST);
    EXPECT_EQ(ret, INVALID_PARAMETERS_ERR);
    DTEST_LOG << "MissionSnapshotChanged_002 end" << std::endl;
}

/**
 * @tc.name: MissionSnapshotChanged_003
 * @tc.desc: MissionSnapshotChanged returns the error code of GetLocalMissionSnapshotInfo
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, MissionSnapshotChanged_003, TestSize.Level3)
{
    DTEST_LOG << "MissionSnapshotChanged_003 begin" << std::endl;
    auto channel = PrepareMissionUtChannel();
    channel->getLocalMissionSnapshotRet_ = UT_CHANNEL_FAIL_RET;
    auto ret = DistributedSchedMissionManager::GetInstance().MissionSnapshotChanged(UT_SNAPSHOT_MISSION_ID_FIRST);
    EXPECT_EQ(ret, UT_CHANNEL_FAIL_RET);
    DTEST_LOG << "MissionSnapshotChanged_003 end" << std::endl;
}

/**
 * @tc.name: MissionSnapshotChanged_001
 * @tc.desc: MissionSnapshotChanged returns null object when the data storage is unavailable
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, MissionSnapshotChanged_001, TestSize.Level3)
{
    DTEST_LOG << "MissionSnapshotChanged_001 begin" << std::endl;
    PrepareMissionUtChannel();
    auto ret = DistributedSchedMissionManager::GetInstance().MissionSnapshotChanged(UT_SNAPSHOT_MISSION_ID_FIRST);
    EXPECT_EQ(ret, ERR_NULL_OBJECT);
    DTEST_LOG << "MissionSnapshotChanged_001 end" << std::endl;
}

/**
 * @tc.name: MissionSnapshotDestroyed_002
 * @tc.desc: MissionSnapshotDestroyed returns invalid parameters when the local device id is unavailable
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, MissionSnapshotDestroyed_002, TestSize.Level3)
{
    DTEST_LOG << "MissionSnapshotDestroyed_002 begin" << std::endl;
    auto channel = PrepareMissionUtChannel();
    channel->getLocalDeviceIdRet_ = false;
    auto ret = DistributedSchedMissionManager::GetInstance().MissionSnapshotDestroyed(UT_SNAPSHOT_MISSION_ID_FIRST);
    EXPECT_EQ(ret, INVALID_PARAMETERS_ERR);
    DTEST_LOG << "MissionSnapshotDestroyed_002 end" << std::endl;
}

/**
 * @tc.name: MissionSnapshotDestroyed_001
 * @tc.desc: MissionSnapshotDestroyed returns null object when the data storage is unavailable
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, MissionSnapshotDestroyed_001, TestSize.Level3)
{
    DTEST_LOG << "MissionSnapshotDestroyed_001 begin" << std::endl;
    PrepareMissionUtChannel();
    auto ret = DistributedSchedMissionManager::GetInstance().MissionSnapshotDestroyed(UT_SNAPSHOT_MISSION_ID_FIRST);
    EXPECT_EQ(ret, ERR_NULL_OBJECT);
    DTEST_LOG << "MissionSnapshotDestroyed_001 end" << std::endl;
}

/**
 * @tc.name: InitAllSnapshots_001
 * @tc.desc: InitAllSnapshots walks every mission even when the snapshot cannot be stored
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, InitAllSnapshots_001, TestSize.Level3)
{
    DTEST_LOG << "InitAllSnapshots_001 begin" << std::endl;
    auto channel = PrepareMissionUtChannel();
    auto missionInfos = CreateUtDstbMissionInfos();
    EXPECT_NO_FATAL_FAILURE(DistributedSchedMissionManager::GetInstance().InitAllSnapshots(missionInfos));
    EXPECT_EQ(channel->snapshotQueryCount_.load(), static_cast<int32_t>(missionInfos.size()));
    DTEST_LOG << "InitAllSnapshots_001 end" << std::endl;
}

/**
 * @tc.name: StoreSnapshotInfo_001
 * @tc.desc: StoreSnapshotInfo returns null object when the data storage is unavailable
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, StoreSnapshotInfo_001, TestSize.Level3)
{
    DTEST_LOG << "StoreSnapshotInfo_001 begin" << std::endl;
    PrepareMissionUtChannel();
    std::vector<uint8_t> byteStream(BYTESTREAM_LENGTH, ONE_BYTE);
    auto ret = DistributedSchedMissionManager::GetInstance().StoreSnapshotInfo(DEVICE_ID,
        UT_SNAPSHOT_MISSION_ID_FIRST, byteStream.data(), byteStream.size());
    EXPECT_EQ(ret, ERR_NULL_OBJECT);
    DTEST_LOG << "StoreSnapshotInfo_001 end" << std::endl;
}

/**
 * @tc.name: RemoveSnapshotInfo_001
 * @tc.desc: RemoveSnapshotInfo returns null object when the data storage is unavailable
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, RemoveSnapshotInfo_001, TestSize.Level3)
{
    DTEST_LOG << "RemoveSnapshotInfo_001 begin" << std::endl;
    PrepareMissionUtChannel();
    auto ret = DistributedSchedMissionManager::GetInstance().RemoveSnapshotInfo(DEVICE_ID,
        UT_SNAPSHOT_MISSION_ID_FIRST);
    EXPECT_EQ(ret, ERR_NULL_OBJECT);
    DTEST_LOG << "RemoveSnapshotInfo_001 end" << std::endl;
}

/**
 * @tc.name: DeleteDataStorage_003
 * @tc.desc: DeleteDataStorage returns early when the data storage is unavailable
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, DeleteDataStorage_003, TestSize.Level3)
{
    DTEST_LOG << "DeleteDataStorage_003 begin" << std::endl;
    auto channel = PrepareMissionUtChannel();
    EXPECT_NO_FATAL_FAILURE(DistributedSchedMissionManager::GetInstance().DeleteDataStorage(DEVICE_ID, false));
    EXPECT_EQ(channel->uuidQueryCount_.load(), 0);
    DTEST_LOG << "DeleteDataStorage_003 end" << std::endl;
}

/**
 * @tc.name: DeleteDataStorage_001
 * @tc.desc: DeleteDataStorage deletes immediately when no handler is available
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, DeleteDataStorage_001, TestSize.Level3)
{
    DTEST_LOG << "DeleteDataStorage_001 begin" << std::endl;
    auto channel = PrepareMissionUtChannel();
    auto& mgr = DistributedSchedMissionManager::GetInstance();
    mgr.distributedDataStorage_ = std::make_shared<DistributedDataStorage>();
    EXPECT_NO_FATAL_FAILURE(mgr.DeleteDataStorage(DEVICE_ID, false));
    EXPECT_EQ(channel->uuidQueryCount_.load(), 1);
    DTEST_LOG << "DeleteDataStorage_001 end" << std::endl;
}

/**
 * @tc.name: DeleteDataStorage_002
 * @tc.desc: DeleteDataStorage does not post the delayed task when no handler is available
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, DeleteDataStorage_002, TestSize.Level3)
{
    DTEST_LOG << "DeleteDataStorage_002 begin" << std::endl;
    auto channel = PrepareMissionUtChannel();
    auto& mgr = DistributedSchedMissionManager::GetInstance();
    mgr.distributedDataStorage_ = std::make_shared<DistributedDataStorage>();
    EXPECT_NO_FATAL_FAILURE(mgr.DeleteDataStorage(DEVICE_ID, true));
    EXPECT_EQ(channel->uuidQueryCount_.load(), 1);
    DTEST_LOG << "DeleteDataStorage_002 end" << std::endl;
}

/**
 * @tc.name: DeviceOnlineNotify_002
 * @tc.desc: DeviceOnlineNotify returns early when networkId is empty
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, DeviceOnlineNotify_002, TestSize.Level3)
{
    DTEST_LOG << "DeviceOnlineNotify_002 begin" << std::endl;
    auto channel = PrepareMissionUtChannel();
    EXPECT_NO_FATAL_FAILURE(DistributedSchedMissionManager::GetInstance().DeviceOnlineNotify(""));
    EXPECT_EQ(channel->uuidQueryCount_.load(), 0);
    DTEST_LOG << "DeviceOnlineNotify_002 end" << std::endl;
}

/**
 * @tc.name: DeviceOnlineNotify_001
 * @tc.desc: DeviceOnlineNotify only looks up the uuid when no handler is available
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, DeviceOnlineNotify_001, TestSize.Level3)
{
    DTEST_LOG << "DeviceOnlineNotify_001 begin" << std::endl;
    auto channel = PrepareMissionUtChannel();
    EXPECT_NO_FATAL_FAILURE(DistributedSchedMissionManager::GetInstance().DeviceOnlineNotify(DEVICE_ID));
    EXPECT_EQ(channel->uuidQueryCount_.load(), 1);
    DTEST_LOG << "DeviceOnlineNotify_001 end" << std::endl;
}

/**
 * @tc.name: DeviceOfflineNotify_002
 * @tc.desc: DeviceOfflineNotify returns early when networkId is empty
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, DeviceOfflineNotify_002, TestSize.Level3)
{
    DTEST_LOG << "DeviceOfflineNotify_002 begin" << std::endl;
    auto channel = PrepareMissionUtChannel();
    EXPECT_NO_FATAL_FAILURE(DistributedSchedMissionManager::GetInstance().DeviceOfflineNotify(""));
    EXPECT_EQ(channel->uuidQueryCount_.load(), 0);
    DTEST_LOG << "DeviceOfflineNotify_002 end" << std::endl;
}

/**
 * @tc.name: DeviceOfflineNotify_001
 * @tc.desc: DeviceOfflineNotify handles a device without cached resources
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, DeviceOfflineNotify_001, TestSize.Level3)
{
    DTEST_LOG << "DeviceOfflineNotify_001 begin" << std::endl;
    PrepareMissionUtChannel();
    auto& mgr = DistributedSchedMissionManager::GetInstance();
    mgr.Init();
    EXPECT_NO_FATAL_FAILURE(mgr.DeviceOfflineNotify(DEVICE_ID));
    EXPECT_TRUE(mgr.remoteDmsMap_.empty());
    EXPECT_TRUE(mgr.listenDeviceMap_.empty());
    DTEST_LOG << "DeviceOfflineNotify_001 end" << std::endl;
}

/**
 * @tc.name: RegisterMissionListener_003
 * @tc.desc: RegisterMissionListener returns invalid parameters when the listener is null
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, RegisterMissionListener_003, TestSize.Level3)
{
    DTEST_LOG << "RegisterMissionListener_003 begin" << std::endl;
    PrepareMissionUtChannel();
    auto& mgr = DistributedSchedMissionManager::GetInstance();
    mgr.Init();
    sptr<IRemoteObject> nullListener = nullptr;
    EXPECT_EQ(mgr.RegisterMissionListener(U16DEVICE_ID, nullListener), INVALID_PARAMETERS_ERR);
    EXPECT_TRUE(mgr.listenDeviceMap_.empty());
    DTEST_LOG << "RegisterMissionListener_003 end" << std::endl;
}

/**
 * @tc.name: RegisterMissionListener_001
 * @tc.desc: RegisterMissionListener keeps a single entry when the same listener is registered twice
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, RegisterMissionListener_001, TestSize.Level3)
{
    DTEST_LOG << "RegisterMissionListener_001 begin" << std::endl;
    PrepareMissionUtChannel();
    auto& mgr = DistributedSchedMissionManager::GetInstance();
    mgr.Init();
    sptr<IRemoteObject> listener = new RemoteMissionListenerTest();
    EXPECT_EQ(mgr.RegisterMissionListener(U16DEVICE_ID, listener), ERR_NONE);
    EXPECT_EQ(mgr.RegisterMissionListener(U16DEVICE_ID, listener), ERR_NONE);
    auto iter = mgr.listenDeviceMap_.find(U16DEVICE_ID);
    ASSERT_TRUE(iter != mgr.listenDeviceMap_.end());
    EXPECT_EQ(iter->second.Size(), 1);
    DTEST_LOG << "RegisterMissionListener_001 end" << std::endl;
}

/**
 * @tc.name: RegisterMissionListener_002
 * @tc.desc: RegisterMissionListener accepts a second listener for the same device
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, RegisterMissionListener_002, TestSize.Level3)
{
    DTEST_LOG << "RegisterMissionListener_002 begin" << std::endl;
    PrepareMissionUtChannel();
    auto& mgr = DistributedSchedMissionManager::GetInstance();
    mgr.Init();
    sptr<IRemoteObject> firstListener = new RemoteMissionListenerTest();
    sptr<IRemoteObject> secondListener = new RemoteMissionListenerTest();
    EXPECT_EQ(mgr.RegisterMissionListener(U16DEVICE_ID, firstListener), ERR_NONE);
    EXPECT_EQ(mgr.RegisterMissionListener(U16DEVICE_ID, secondListener), ERR_NONE);
    auto iter = mgr.listenDeviceMap_.find(U16DEVICE_ID);
    ASSERT_TRUE(iter != mgr.listenDeviceMap_.end());
    EXPECT_EQ(iter->second.Size(), 2);
    DTEST_LOG << "RegisterMissionListener_002 end" << std::endl;
}

/**
 * @tc.name: UnRegisterMissionListener_001
 * @tc.desc: UnRegisterMissionListener returns success when the device has no registered listener
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, UnRegisterMissionListener_001, TestSize.Level3)
{
    DTEST_LOG << "UnRegisterMissionListener_001 begin" << std::endl;
    PrepareMissionUtChannel();
    auto& mgr = DistributedSchedMissionManager::GetInstance();
    mgr.Init();
    sptr<IRemoteObject> listener = new RemoteMissionListenerTest();
    EXPECT_EQ(mgr.UnRegisterMissionListener(U16DEVICE_ID, listener), ERR_NONE);
    EXPECT_TRUE(mgr.listenDeviceMap_.empty());
    DTEST_LOG << "UnRegisterMissionListener_001 end" << std::endl;
}

/**
 * @tc.name: UnRegisterMissionListener_002
 * @tc.desc: UnRegisterMissionListener keeps the device entry when another listener is unregistered
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, UnRegisterMissionListener_002, TestSize.Level3)
{
    DTEST_LOG << "UnRegisterMissionListener_002 begin" << std::endl;
    PrepareMissionUtChannel();
    auto& mgr = DistributedSchedMissionManager::GetInstance();
    mgr.Init();
    sptr<IRemoteObject> registeredListener = new RemoteMissionListenerTest();
    sptr<IRemoteObject> unknownListener = new RemoteMissionListenerTest();
    EXPECT_EQ(mgr.RegisterMissionListener(U16DEVICE_ID, registeredListener), ERR_NONE);
    EXPECT_EQ(mgr.UnRegisterMissionListener(U16DEVICE_ID, unknownListener), ERR_NONE);
    auto iter = mgr.listenDeviceMap_.find(U16DEVICE_ID);
    ASSERT_TRUE(iter != mgr.listenDeviceMap_.end());
    EXPECT_EQ(iter->second.Size(), 1);
    EXPECT_TRUE(iter->second.Find(registeredListener));
    DTEST_LOG << "UnRegisterMissionListener_002 end" << std::endl;
}

/**
 * @tc.name: UnRegisterMissionListener_003
 * @tc.desc: UnRegisterMissionListener keeps the device entry when one of two listeners is unregistered
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, UnRegisterMissionListener_003, TestSize.Level3)
{
    DTEST_LOG << "UnRegisterMissionListener_003 begin" << std::endl;
    PrepareMissionUtChannel();
    auto& mgr = DistributedSchedMissionManager::GetInstance();
    mgr.Init();
    sptr<IRemoteObject> firstListener = new RemoteMissionListenerTest();
    sptr<IRemoteObject> secondListener = new RemoteMissionListenerTest();
    EXPECT_EQ(mgr.RegisterMissionListener(U16DEVICE_ID, firstListener), ERR_NONE);
    EXPECT_EQ(mgr.RegisterMissionListener(U16DEVICE_ID, secondListener), ERR_NONE);
    EXPECT_EQ(mgr.UnRegisterMissionListener(U16DEVICE_ID, firstListener), ERR_NONE);
    auto iter = mgr.listenDeviceMap_.find(U16DEVICE_ID);
    ASSERT_TRUE(iter != mgr.listenDeviceMap_.end());
    EXPECT_EQ(iter->second.Size(), 1);
    EXPECT_TRUE(iter->second.Find(secondListener));
    DTEST_LOG << "UnRegisterMissionListener_003 end" << std::endl;
}

/**
 * @tc.name: StartSyncRemoteMissions_003
 * @tc.desc: StartSyncRemoteMissions returns invalid parameters when the device is offline
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, StartSyncRemoteMissions_003, TestSize.Level3)
{
    DTEST_LOG << "StartSyncRemoteMissions_003 begin" << std::endl;
    PrepareMissionUtChannel(false);
    auto ret = DistributedSchedMissionManager::GetInstance().StartSyncRemoteMissions(DEVICE_ID, false, 0, 0);
    EXPECT_EQ(ret, INVALID_PARAMETERS_ERR);
    DTEST_LOG << "StartSyncRemoteMissions_003 end" << std::endl;
}

/**
 * @tc.name: StartSyncRemoteMissions_004
 * @tc.desc: StartSyncRemoteMissions returns invalid parameters when the target device is the local one
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, StartSyncRemoteMissions_004, TestSize.Level3)
{
    DTEST_LOG << "StartSyncRemoteMissions_004 begin" << std::endl;
    auto channel = PrepareMissionUtChannel();
    channel->localDeviceId_ = DEVICE_ID;
    auto ret = DistributedSchedMissionManager::GetInstance().StartSyncRemoteMissions(DEVICE_ID, false, 0, 0);
    EXPECT_EQ(ret, INVALID_PARAMETERS_ERR);
    DTEST_LOG << "StartSyncRemoteMissions_004 end" << std::endl;
}

/**
 * @tc.name: StartSyncRemoteMissions_001
 * @tc.desc: StartSyncRemoteMissions returns success when the device has no registered listener
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, StartSyncRemoteMissions_001, TestSize.Level3)
{
    DTEST_LOG << "StartSyncRemoteMissions_001 begin" << std::endl;
    PrepareMissionUtChannel();
    auto& mgr = DistributedSchedMissionManager::GetInstance();
    auto ret = mgr.StartSyncRemoteMissions(DEVICE_ID, false, 0, 0);
    EXPECT_EQ(ret, ERR_NONE);
    EXPECT_TRUE(mgr.remoteDmsMap_.empty());
    DTEST_LOG << "StartSyncRemoteMissions_001 end" << std::endl;
}

/**
 * @tc.name: StartSyncRemoteMissions_002
 * @tc.desc: StartSyncRemoteMissions returns success without connecting when the device was already synced
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, StartSyncRemoteMissions_002, TestSize.Level3)
{
    DTEST_LOG << "StartSyncRemoteMissions_002 begin" << std::endl;
    PrepareMissionUtChannel();
    auto& mgr = DistributedSchedMissionManager::GetInstance();
    {
        std::lock_guard<std::mutex> autoLock(mgr.listenDeviceLock_);
        ListenerInfo listenerInfo;
        listenerInfo.called = true;
        mgr.listenDeviceMap_[U16DEVICE_ID] = listenerInfo;
    }
    auto ret = mgr.StartSyncRemoteMissions(DEVICE_ID, false, 0, 0);
    EXPECT_EQ(ret, ERR_NONE);
    EXPECT_TRUE(mgr.remoteDmsMap_.empty());
    DTEST_LOG << "StartSyncRemoteMissions_002 end" << std::endl;
}

/**
 * @tc.name: StartSyncRemoteMissions_005
 * @tc.desc: StartSyncRemoteMissions returns invalid parameters when the remote dms is null
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, StartSyncRemoteMissions_005, TestSize.Level3)
{
    DTEST_LOG << "StartSyncRemoteMissions_005 begin" << std::endl;
    PrepareMissionUtChannel();
    auto ret = DistributedSchedMissionManager::GetInstance().StartSyncRemoteMissions(DEVICE_ID,
        sptr<IDistributedSched>(nullptr), 0);
    EXPECT_EQ(ret, INVALID_PARAMETERS_ERR);
    DTEST_LOG << "StartSyncRemoteMissions_005 end" << std::endl;
}

/**
 * @tc.name: StartSyncRemoteMissions_006
 * @tc.desc: StartSyncRemoteMissions returns local device error when the caller info cannot be generated
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, StartSyncRemoteMissions_006, TestSize.Level3)
{
    DTEST_LOG << "StartSyncRemoteMissions_006 begin" << std::endl;
    auto channel = PrepareMissionUtChannel();
    channel->getLocalDeviceIdRet_ = false;
    auto ret = DistributedSchedMissionManager::GetInstance().StartSyncRemoteMissions(DEVICE_ID,
        CreateUtMockRemoteDms(), 0);
    EXPECT_EQ(ret, GET_LOCAL_DEVICE_ERR);
    DTEST_LOG << "StartSyncRemoteMissions_006 end" << std::endl;
}

/**
 * @tc.name: StopSyncRemoteMissions_001
 * @tc.desc: StopSyncRemoteMissions returns success directly when the device goes offline
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, StopSyncRemoteMissions_001, TestSize.Level3)
{
    DTEST_LOG << "StopSyncRemoteMissions_001 begin" << std::endl;
    PrepareMissionUtChannel();
    auto& mgr = DistributedSchedMissionManager::GetInstance();
    mgr.RebornMissionCache(DEVICE_ID, CreateUtDstbMissionInfos());
    EXPECT_EQ(mgr.deviceMissionInfos_.size(), 1u);
    auto ret = mgr.StopSyncRemoteMissions(DEVICE_ID, true);
    EXPECT_EQ(ret, ERR_NONE);
    EXPECT_TRUE(mgr.deviceMissionInfos_.empty());
    DTEST_LOG << "StopSyncRemoteMissions_001 end" << std::endl;
}

/**
 * @tc.name: StopSyncRemoteMissions_002
 * @tc.desc: StopSyncRemoteMissions returns local device error when the caller info cannot be generated
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, StopSyncRemoteMissions_002, TestSize.Level3)
{
    DTEST_LOG << "StopSyncRemoteMissions_002 begin" << std::endl;
    auto channel = PrepareMissionUtChannel();
    auto& mgr = DistributedSchedMissionManager::GetInstance();
    {
        std::lock_guard<std::mutex> autoLock(mgr.remoteDmsLock_);
        mgr.remoteDmsMap_[DEVICE_ID] = CreateUtMockRemoteDms();
    }
    channel->getLocalDeviceIdRet_ = false;
    auto ret = mgr.StopSyncRemoteMissions(DEVICE_ID, false);
    EXPECT_EQ(ret, GET_LOCAL_DEVICE_ERR);
    DTEST_LOG << "StopSyncRemoteMissions_002 end" << std::endl;
}

/**
 * @tc.name: StartSyncMissionsFromRemote_001
 * @tc.desc: StartSyncMissionsFromRemote skips the listener registration when no handler exists
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, StartSyncMissionsFromRemote_001, TestSize.Level3)
{
    DTEST_LOG << "StartSyncMissionsFromRemote_001 begin" << std::endl;
    auto channel = PrepareMissionUtChannel();
    channel->localMissionInfos_ = CreateUtDstbMissionInfos();
    CallerInfo callerInfo;
    callerInfo.sourceDeviceId = DEVICE_ID;
    std::vector<DstbMissionInfo> missionInfos;
    auto ret = DistributedSchedMissionManager::GetInstance().StartSyncMissionsFromRemote(callerInfo, missionInfos);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(missionInfos.size(), UT_CACHED_MISSION_COUNT);
    EXPECT_EQ(channel->registerCount_.load(), 0);
    EXPECT_EQ(DistributedSchedMissionManager::GetInstance().remoteSyncDeviceSet_.count(DEVICE_ID), 1u);
    DTEST_LOG << "StartSyncMissionsFromRemote_001 end" << std::endl;
}

/**
 * @tc.name: StartSyncMissionsFromRemote_002
 * @tc.desc: StartSyncMissionsFromRemote returns invalid parameters when the access control check fails
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, StartSyncMissionsFromRemote_002, TestSize.Level3)
{
    DTEST_LOG << "StartSyncMissionsFromRemote_002 begin" << std::endl;
    auto channel = PrepareMissionUtChannel();
    channel->getLocalDeviceIdRet_ = false;
    CallerInfo callerInfo;
    callerInfo.sourceDeviceId = DEVICE_ID;
    nlohmann::json extraInfoJson;
    extraInfoJson[Constants::EXTRO_INFO_JSON_KEY_ACCOUNT_ID] = "ut-account-id";
    extraInfoJson[Constants::EXTRO_INFO_JSON_KEY_USERID_ID] = UT_ACCOUNT_USER_ID;
    callerInfo.extraInfoJson = extraInfoJson;
    std::vector<DstbMissionInfo> missionInfos;
    auto ret = DistributedSchedMissionManager::GetInstance().StartSyncMissionsFromRemote(callerInfo, missionInfos);
    EXPECT_EQ(ret, INVALID_PARAMETERS_ERR);
    DTEST_LOG << "StartSyncMissionsFromRemote_002 end" << std::endl;
}

/**
 * @tc.name: StartSyncMissionsFromRemote_003
 * @tc.desc: StartSyncMissionsFromRemote returns the error of GetLocalMissionInfos
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, StartSyncMissionsFromRemote_003, TestSize.Level3)
{
    DTEST_LOG << "StartSyncMissionsFromRemote_003 begin" << std::endl;
    auto channel = PrepareMissionUtChannel();
    channel->getLocalMissionInfosRet_ = INVALID_PARAMETERS_ERR;
    CallerInfo callerInfo;
    callerInfo.sourceDeviceId = DEVICE_ID;
    std::vector<DstbMissionInfo> missionInfos;
    auto ret = DistributedSchedMissionManager::GetInstance().StartSyncMissionsFromRemote(callerInfo, missionInfos);
    EXPECT_EQ(ret, INVALID_PARAMETERS_ERR);
    DTEST_LOG << "StartSyncMissionsFromRemote_003 end" << std::endl;
}

/**
 * @tc.name: CheckAccessControlForMissions_001
 * @tc.desc: CheckAccessControlForMissions returns false when the local device id is unavailable
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, CheckAccessControlForMissions_001, TestSize.Level3)
{
    DTEST_LOG << "CheckAccessControlForMissions_001 begin" << std::endl;
    auto channel = PrepareMissionUtChannel();
    channel->getLocalDeviceIdRet_ = false;
    CallerInfo callerInfo;
    callerInfo.sourceDeviceId = DEVICE_ID;
    EXPECT_FALSE(DistributedSchedMissionManager::GetInstance().CheckAccessControlForMissions(callerInfo));
    DTEST_LOG << "CheckAccessControlForMissions_001 end" << std::endl;
}

/**
 * @tc.name: CheckAccessControlForMissions_002
 * @tc.desc: CheckAccessControlForMissions ignores account fields with unexpected json types
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, CheckAccessControlForMissions_002, TestSize.Level3)
{
    DTEST_LOG << "CheckAccessControlForMissions_002 begin" << std::endl;
    auto channel = PrepareMissionUtChannel();
    channel->getLocalDeviceIdRet_ = false;
    CallerInfo callerInfo;
    callerInfo.sourceDeviceId = DEVICE_ID;
    nlohmann::json extraInfoJson;
    extraInfoJson[Constants::EXTRO_INFO_JSON_KEY_ACCOUNT_ID] = UT_ACCOUNT_USER_ID;
    extraInfoJson[Constants::EXTRO_INFO_JSON_KEY_USERID_ID] = "ut-user-id";
    callerInfo.extraInfoJson = extraInfoJson;
    EXPECT_FALSE(DistributedSchedMissionManager::GetInstance().CheckAccessControlForMissions(callerInfo));
    DTEST_LOG << "CheckAccessControlForMissions_002 end" << std::endl;
}

/**
 * @tc.name: StopSyncMissionsFromRemote_002
 * @tc.desc: StopSyncMissionsFromRemote does nothing when no device is syncing
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, StopSyncMissionsFromRemote_002, TestSize.Level3)
{
    DTEST_LOG << "StopSyncMissionsFromRemote_002 begin" << std::endl;
    PrepareMissionUtChannel();
    auto& mgr = DistributedSchedMissionManager::GetInstance();
    EXPECT_NO_FATAL_FAILURE(mgr.StopSyncMissionsFromRemote(DEVICE_ID));
    EXPECT_TRUE(mgr.remoteSyncDeviceSet_.empty());
    DTEST_LOG << "StopSyncMissionsFromRemote_002 end" << std::endl;
}

/**
 * @tc.name: StopSyncMissionsFromRemote_001
 * @tc.desc: StopSyncMissionsFromRemote only removes the requested device from the sync set
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, StopSyncMissionsFromRemote_001, TestSize.Level3)
{
    DTEST_LOG << "StopSyncMissionsFromRemote_001 begin" << std::endl;
    PrepareMissionUtChannel();
    auto& mgr = DistributedSchedMissionManager::GetInstance();
    {
        std::lock_guard<std::mutex> autoLock(mgr.remoteSyncDeviceLock_);
        mgr.remoteSyncDeviceSet_.insert(DEVICE_ID);
        mgr.remoteSyncDeviceSet_.insert(DEVICE_ID_FALSE);
    }
    mgr.StopSyncMissionsFromRemote(DEVICE_ID);
    EXPECT_EQ(mgr.remoteSyncDeviceSet_.size(), 1u);
    EXPECT_EQ(mgr.remoteSyncDeviceSet_.count(DEVICE_ID_FALSE), 1u);
    DTEST_LOG << "StopSyncMissionsFromRemote_001 end" << std::endl;
}

/**
 * @tc.name: StopSyncMissionsFromRemote_003
 * @tc.desc: StopSyncMissionsFromRemote clears the sync set without unregistering when no handler exists
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, StopSyncMissionsFromRemote_003, TestSize.Level3)
{
    DTEST_LOG << "StopSyncMissionsFromRemote_003 begin" << std::endl;
    auto channel = PrepareMissionUtChannel();
    auto& mgr = DistributedSchedMissionManager::GetInstance();
    mgr.isRegMissionChange_ = true;
    {
        std::lock_guard<std::mutex> autoLock(mgr.remoteSyncDeviceLock_);
        mgr.remoteSyncDeviceSet_.insert(DEVICE_ID);
    }
    mgr.StopSyncMissionsFromRemote(DEVICE_ID);
    EXPECT_TRUE(mgr.remoteSyncDeviceSet_.empty());
    EXPECT_TRUE(mgr.isRegMissionChange_.load());
    EXPECT_EQ(channel->unregisterCount_.load(), 0);
    DTEST_LOG << "StopSyncMissionsFromRemote_003 end" << std::endl;
}

/**
 * @tc.name: NeedSyncDevice_002
 * @tc.desc: NeedSyncDevice returns false when deviceId is empty
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, NeedSyncDevice_002, TestSize.Level3)
{
    DTEST_LOG << "NeedSyncDevice_002 begin" << std::endl;
    PrepareMissionUtChannel();
    EXPECT_FALSE(DistributedSchedMissionManager::GetInstance().NeedSyncDevice(""));
    DTEST_LOG << "NeedSyncDevice_002 end" << std::endl;
}

/**
 * @tc.name: NeedSyncDevice_001
 * @tc.desc: NeedSyncDevice returns true when the device is syncing
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, NeedSyncDevice_001, TestSize.Level3)
{
    DTEST_LOG << "NeedSyncDevice_001 begin" << std::endl;
    PrepareMissionUtChannel();
    auto& mgr = DistributedSchedMissionManager::GetInstance();
    {
        std::lock_guard<std::mutex> autoLock(mgr.remoteSyncDeviceLock_);
        mgr.remoteSyncDeviceSet_.insert(DEVICE_ID);
    }
    EXPECT_TRUE(mgr.NeedSyncDevice(DEVICE_ID));
    DTEST_LOG << "NeedSyncDevice_001 end" << std::endl;
}

/**
 * @tc.name: HasSyncListener_002
 * @tc.desc: HasSyncListener returns false when the device has no listener entry
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, HasSyncListener_002, TestSize.Level3)
{
    DTEST_LOG << "HasSyncListener_002 begin" << std::endl;
    PrepareMissionUtChannel();
    EXPECT_FALSE(DistributedSchedMissionManager::GetInstance().HasSyncListener(DEVICE_ID));
    DTEST_LOG << "HasSyncListener_002 end" << std::endl;
}

/**
 * @tc.name: HasSyncListener_001
 * @tc.desc: HasSyncListener returns false when the listener was never synced
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, HasSyncListener_001, TestSize.Level3)
{
    DTEST_LOG << "HasSyncListener_001 begin" << std::endl;
    PrepareMissionUtChannel();
    auto& mgr = DistributedSchedMissionManager::GetInstance();
    {
        std::lock_guard<std::mutex> autoLock(mgr.listenDeviceLock_);
        ListenerInfo listenerInfo;
        mgr.listenDeviceMap_[U16DEVICE_ID] = listenerInfo;
    }
    EXPECT_FALSE(mgr.HasSyncListener(DEVICE_ID));
    DTEST_LOG << "HasSyncListener_001 end" << std::endl;
}

/**
 * @tc.name: NotifySnapshotChanged_002
 * @tc.desc: NotifySnapshotChanged does nothing when the device has no listener
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, NotifySnapshotChanged_002, TestSize.Level3)
{
    DTEST_LOG << "NotifySnapshotChanged_002 begin" << std::endl;
    PrepareMissionUtChannel();
    auto& mgr = DistributedSchedMissionManager::GetInstance();
    EXPECT_NO_FATAL_FAILURE(mgr.NotifySnapshotChanged(DEVICE_ID, UT_SNAPSHOT_MISSION_ID_FIRST));
    EXPECT_TRUE(mgr.listenDeviceMap_.empty());
    DTEST_LOG << "NotifySnapshotChanged_002 end" << std::endl;
}

/**
 * @tc.name: NotifySnapshotChanged_001
 * @tc.desc: NotifySnapshotChanged skips a null listener
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, NotifySnapshotChanged_001, TestSize.Level3)
{
    DTEST_LOG << "NotifySnapshotChanged_001 begin" << std::endl;
    PrepareMissionUtChannel();
    auto& mgr = DistributedSchedMissionManager::GetInstance();
    sptr<IRemoteObject> nullListener = nullptr;
    {
        std::lock_guard<std::mutex> autoLock(mgr.listenDeviceLock_);
        ListenerInfo listenerInfo;
        listenerInfo.Emplace(nullListener);
        mgr.listenDeviceMap_[U16DEVICE_ID] = listenerInfo;
    }
    EXPECT_NO_FATAL_FAILURE(mgr.NotifySnapshotChanged(DEVICE_ID, UT_SNAPSHOT_MISSION_ID_FIRST));
    EXPECT_EQ(mgr.listenDeviceMap_.size(), 1u);
    auto iter = mgr.listenDeviceMap_.find(U16DEVICE_ID);
    ASSERT_TRUE(iter != mgr.listenDeviceMap_.end());
    EXPECT_EQ(iter->second.Size(), 1);
    DTEST_LOG << "NotifySnapshotChanged_001 end" << std::endl;
}

/**
 * @tc.name: NotifyNetDisconnectOffline_001
 * @tc.desc: NotifyNetDisconnectOffline skips null listeners of every device
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, NotifyNetDisconnectOffline_001, TestSize.Level3)
{
    DTEST_LOG << "NotifyNetDisconnectOffline_001 begin" << std::endl;
    PrepareMissionUtChannel();
    auto& mgr = DistributedSchedMissionManager::GetInstance();
    sptr<IRemoteObject> nullListener = nullptr;
    {
        std::lock_guard<std::mutex> autoLock(mgr.listenDeviceLock_);
        ListenerInfo firstInfo;
        firstInfo.Emplace(nullListener);
        mgr.listenDeviceMap_[U16DEVICE_ID] = firstInfo;
        ListenerInfo secondInfo;
        secondInfo.Emplace(nullListener);
        mgr.listenDeviceMap_[Str8ToStr16(DEVICE_ID_FALSE)] = secondInfo;
    }
    EXPECT_NO_FATAL_FAILURE(mgr.NotifyNetDisconnectOffline());
    EXPECT_EQ(mgr.listenDeviceMap_.size(), 2u);
    DTEST_LOG << "NotifyNetDisconnectOffline_001 end" << std::endl;
}

/**
 * @tc.name: NotifyMissionsChangedFromRemote_001
 * @tc.desc: NotifyMissionsChangedFromRemote returns invalid parameters when no handler exists
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, NotifyMissionsChangedFromRemote_001, TestSize.Level3)
{
    DTEST_LOG << "NotifyMissionsChangedFromRemote_001 begin" << std::endl;
    PrepareMissionUtChannel();
    auto& mgr = DistributedSchedMissionManager::GetInstance();
    sptr<IRemoteObject> nullListener = nullptr;
    {
        std::lock_guard<std::mutex> autoLock(mgr.listenDeviceLock_);
        ListenerInfo listenerInfo;
        listenerInfo.Emplace(nullListener);
        mgr.listenDeviceMap_[U16DEVICE_ID] = listenerInfo;
    }
    CallerInfo callerInfo;
    callerInfo.sourceDeviceId = DEVICE_ID;
    auto ret = mgr.NotifyMissionsChangedFromRemote(callerInfo, CreateUtDstbMissionInfos());
    EXPECT_EQ(ret, INVALID_PARAMETERS_ERR);
    EXPECT_EQ(mgr.deviceMissionInfos_.size(), 1u);
    DTEST_LOG << "NotifyMissionsChangedFromRemote_001 end" << std::endl;
}

/**
 * @tc.name: NotifyLocalMissionsChanged_001
 * @tc.desc: NotifyLocalMissionsChanged returns early when the change handler is unavailable
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, NotifyLocalMissionsChanged_001, TestSize.Level3)
{
    DTEST_LOG << "NotifyLocalMissionsChanged_001 begin" << std::endl;
    auto channel = PrepareMissionUtChannel();
    EXPECT_NO_FATAL_FAILURE(DistributedSchedMissionManager::GetInstance().NotifyLocalMissionsChanged());
    EXPECT_EQ(channel->localMissionInfosCount_.load(), 0);
    DTEST_LOG << "NotifyLocalMissionsChanged_001 end" << std::endl;
}

/**
 * @tc.name: NotifyMissionSnapshotCreated_001
 * @tc.desc: NotifyMissionSnapshotCreated returns early when the change handler is unavailable
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, NotifyMissionSnapshotCreated_001, TestSize.Level3)
{
    DTEST_LOG << "NotifyMissionSnapshotCreated_001 begin" << std::endl;
    auto channel = PrepareMissionUtChannel();
    EXPECT_NO_FATAL_FAILURE(DistributedSchedMissionManager::GetInstance().NotifyMissionSnapshotCreated(
        UT_SNAPSHOT_MISSION_ID_FIRST));
    EXPECT_EQ(channel->snapshotQueryCount_.load(), 0);
    DTEST_LOG << "NotifyMissionSnapshotCreated_001 end" << std::endl;
}

/**
 * @tc.name: NotifyMissionSnapshotChanged_001
 * @tc.desc: NotifyMissionSnapshotChanged returns early when the change handler is unavailable
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, NotifyMissionSnapshotChanged_001, TestSize.Level3)
{
    DTEST_LOG << "NotifyMissionSnapshotChanged_001 begin" << std::endl;
    auto channel = PrepareMissionUtChannel();
    EXPECT_NO_FATAL_FAILURE(DistributedSchedMissionManager::GetInstance().NotifyMissionSnapshotChanged(
        UT_SNAPSHOT_MISSION_ID_FIRST));
    EXPECT_EQ(channel->snapshotQueryCount_.load(), 0);
    DTEST_LOG << "NotifyMissionSnapshotChanged_001 end" << std::endl;
}

/**
 * @tc.name: NotifyMissionsChangedToRemote_002
 * @tc.desc: NotifyMissionsChangedToRemote returns success when no device is syncing
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, NotifyMissionsChangedToRemote_002, TestSize.Level3)
{
    DTEST_LOG << "NotifyMissionsChangedToRemote_002 begin" << std::endl;
    PrepareMissionUtChannel();
    auto ret = DistributedSchedMissionManager::GetInstance().NotifyMissionsChangedToRemote(
        CreateUtDstbMissionInfos());
    EXPECT_EQ(ret, ERR_NONE);
    DTEST_LOG << "NotifyMissionsChangedToRemote_002 end" << std::endl;
}

/**
 * @tc.name: NotifyMissionsChangedToRemote_003
 * @tc.desc: NotifyMissionsChangedToRemote returns local device error when the caller info cannot be generated
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, NotifyMissionsChangedToRemote_003, TestSize.Level3)
{
    DTEST_LOG << "NotifyMissionsChangedToRemote_003 begin" << std::endl;
    auto channel = PrepareMissionUtChannel();
    channel->getLocalDeviceIdRet_ = false;
    auto ret = DistributedSchedMissionManager::GetInstance().NotifyMissionsChangedToRemote(
        CreateUtDstbMissionInfos());
    EXPECT_EQ(ret, GET_LOCAL_DEVICE_ERR);
    DTEST_LOG << "NotifyMissionsChangedToRemote_003 end" << std::endl;
}

/**
 * @tc.name: NotifyMissionsChangedToRemote_001
 * @tc.desc: NotifyMissionsChangedToRemote skips devices whose handler cannot be fetched
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, NotifyMissionsChangedToRemote_001, TestSize.Level3)
{
    DTEST_LOG << "NotifyMissionsChangedToRemote_001 begin" << std::endl;
    PrepareMissionUtChannel(false);
    auto& mgr = DistributedSchedMissionManager::GetInstance();
    {
        std::lock_guard<std::mutex> autoLock(mgr.remoteSyncDeviceLock_);
        mgr.remoteSyncDeviceSet_.insert(DEVICE_ID);
    }
    auto ret = mgr.NotifyMissionsChangedToRemote(CreateUtDstbMissionInfos());
    EXPECT_EQ(ret, ERR_NONE);
    EXPECT_TRUE(mgr.deviceHandle_.empty());
    DTEST_LOG << "NotifyMissionsChangedToRemote_001 end" << std::endl;
}

/**
 * @tc.name: FetchDeviceHandler_001
 * @tc.desc: FetchDeviceHandler returns nullptr when the device is offline
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, FetchDeviceHandler_001, TestSize.Level3)
{
    DTEST_LOG << "FetchDeviceHandler_001 begin" << std::endl;
    PrepareMissionUtChannel(false);
    EXPECT_EQ(DistributedSchedMissionManager::GetInstance().FetchDeviceHandler(DEVICE_ID).get(), nullptr);
    DTEST_LOG << "FetchDeviceHandler_001 end" << std::endl;
}

/**
 * @tc.name: FetchDeviceHandler_002
 * @tc.desc: FetchDeviceHandler returns nullptr when the device uuid is empty
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, FetchDeviceHandler_002, TestSize.Level3)
{
    DTEST_LOG << "FetchDeviceHandler_002 begin" << std::endl;
    auto channel = PrepareMissionUtChannel();
    channel->uuid_.clear();
    EXPECT_EQ(DistributedSchedMissionManager::GetInstance().FetchDeviceHandler(DEVICE_ID).get(), nullptr);
    DTEST_LOG << "FetchDeviceHandler_002 end" << std::endl;
}

/**
 * @tc.name: GenerateCallerInfo_001
 * @tc.desc: GenerateCallerInfo returns false when the local device id is unavailable
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, GenerateCallerInfo_001, TestSize.Level3)
{
    DTEST_LOG << "GenerateCallerInfo_001 begin" << std::endl;
    auto channel = PrepareMissionUtChannel();
    channel->getLocalDeviceIdRet_ = false;
    CallerInfo callerInfo;
    EXPECT_FALSE(DistributedSchedMissionManager::GetInstance().GenerateCallerInfo(callerInfo));
    DTEST_LOG << "GenerateCallerInfo_001 end" << std::endl;
}

/**
 * @tc.name: OnRemoteDied_001
 * @tc.desc: OnRemoteDied ignores a remote object that cannot be promoted
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, OnRemoteDied_001, TestSize.Level3)
{
    DTEST_LOG << "OnRemoteDied_001 begin" << std::endl;
    PrepareMissionUtChannel();
    auto& mgr = DistributedSchedMissionManager::GetInstance();
    wptr<IRemoteObject> remote = nullptr;
    EXPECT_NO_FATAL_FAILURE(mgr.OnRemoteDied(remote));
    EXPECT_TRUE(mgr.listenDeviceMap_.empty());
    DTEST_LOG << "OnRemoteDied_001 end" << std::endl;
}

/**
 * @tc.name: OnMissionListenerDied_001
 * @tc.desc: OnMissionListenerDied erases the device entry without stopping the sync when not called
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, OnMissionListenerDied_001, TestSize.Level3)
{
    DTEST_LOG << "OnMissionListenerDied_001 begin" << std::endl;
    PrepareMissionUtChannel();
    auto& mgr = DistributedSchedMissionManager::GetInstance();
    mgr.Init();
    sptr<IRemoteObject> diedListener = new RemoteMissionListenerTest();
    {
        std::lock_guard<std::mutex> autoLock(mgr.listenDeviceLock_);
        ListenerInfo listenerInfo;
        listenerInfo.Emplace(diedListener);
        mgr.listenDeviceMap_[U16DEVICE_ID] = listenerInfo;
    }
    mgr.OnMissionListenerDied(diedListener);
    EXPECT_TRUE(mgr.listenDeviceMap_.empty());
    EXPECT_TRUE(mgr.remoteDmsMap_.empty());
    DTEST_LOG << "OnMissionListenerDied_001 end" << std::endl;
}

/**
 * @tc.name: OnRemoteDmsDied_002
 * @tc.desc: OnRemoteDmsDied ignores a remote dms that cannot be promoted
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, OnRemoteDmsDied_002, TestSize.Level3)
{
    DTEST_LOG << "OnRemoteDmsDied_002 begin" << std::endl;
    PrepareMissionUtChannel();
    auto& mgr = DistributedSchedMissionManager::GetInstance();
    wptr<IRemoteObject> remote = nullptr;
    EXPECT_NO_FATAL_FAILURE(mgr.OnRemoteDmsDied(remote));
    EXPECT_TRUE(mgr.remoteDmsMap_.empty());
    DTEST_LOG << "OnRemoteDmsDied_002 end" << std::endl;
}

/**
 * @tc.name: OnRemoteDmsDied_001
 * @tc.desc: OnRemoteDmsDied does not post the died task when no handler exists
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, OnRemoteDmsDied_001, TestSize.Level3)
{
    DTEST_LOG << "OnRemoteDmsDied_001 begin" << std::endl;
    PrepareMissionUtChannel();
    auto& mgr = DistributedSchedMissionManager::GetInstance();
    sptr<IRemoteObject> remoteObject = new MockDistributedSched();
    wptr<IRemoteObject> remote = remoteObject;
    EXPECT_NO_FATAL_FAILURE(mgr.OnRemoteDmsDied(remote));
    EXPECT_TRUE(mgr.remoteDmsMap_.empty());
    EXPECT_TRUE(mgr.listenDeviceMap_.empty());
    DTEST_LOG << "OnRemoteDmsDied_001 end" << std::endl;
}

/**
 * @tc.name: RetryStartSyncRemoteMissions_001
 * @tc.desc: RetryStartSyncRemoteMissions does not post the retry task when no handler exists
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, RetryStartSyncRemoteMissions_001, TestSize.Level3)
{
    DTEST_LOG << "RetryStartSyncRemoteMissions_001 begin" << std::endl;
    auto channel = PrepareMissionUtChannel();
    EXPECT_NO_FATAL_FAILURE(DistributedSchedMissionManager::GetInstance().RetryStartSyncRemoteMissions(DEVICE_ID,
        UT_MISSION_LOCAL_NETWORK_ID, 0));
    EXPECT_EQ(channel->localDeviceIdCount_.load(), 0);
    DTEST_LOG << "RetryStartSyncRemoteMissions_001 end" << std::endl;
}

/**
 * @tc.name: RetryRegisterMissionChange_001
 * @tc.desc: RetryRegisterMissionChange does not post the task when the mission change was not registered
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, RetryRegisterMissionChange_001, TestSize.Level3)
{
    DTEST_LOG << "RetryRegisterMissionChange_001 begin" << std::endl;
    auto channel = PrepareMissionUtChannel();
    auto& mgr = DistributedSchedMissionManager::GetInstance();
    EXPECT_NO_FATAL_FAILURE(mgr.RetryRegisterMissionChange(0));
    EXPECT_FALSE(mgr.isRegMissionChange_.load());
    EXPECT_EQ(channel->registerCount_.load(), 0);
    DTEST_LOG << "RetryRegisterMissionChange_001 end" << std::endl;
}

/**
 * @tc.name: RetryRegisterMissionChange_002
 * @tc.desc: RetryRegisterMissionChange does not post the task when no handler exists
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, RetryRegisterMissionChange_002, TestSize.Level3)
{
    DTEST_LOG << "RetryRegisterMissionChange_002 begin" << std::endl;
    auto channel = PrepareMissionUtChannel();
    auto& mgr = DistributedSchedMissionManager::GetInstance();
    mgr.isRegMissionChange_ = true;
    mgr.RetryRegisterMissionChange(0);
    EXPECT_EQ(channel->registerCount_.load(), 0);
    DTEST_LOG << "RetryRegisterMissionChange_002 end" << std::endl;
}

/**
 * @tc.name: NotifyDmsProxyProcessDied_001
 * @tc.desc: NotifyDmsProxyProcessDied does not retry when no handler exists
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, NotifyDmsProxyProcessDied_001, TestSize.Level3)
{
    DTEST_LOG << "NotifyDmsProxyProcessDied_001 begin" << std::endl;
    auto channel = PrepareMissionUtChannel();
    auto& mgr = DistributedSchedMissionManager::GetInstance();
    mgr.isRegMissionChange_ = true;
    mgr.NotifyDmsProxyProcessDied();
    EXPECT_EQ(channel->registerCount_.load(), 0);
    DTEST_LOG << "NotifyDmsProxyProcessDied_001 end" << std::endl;
}

/**
 * @tc.name: OnDnetDied_001
 * @tc.desc: OnDnetDied does not post the clean task when no handler exists
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, OnDnetDied_001, TestSize.Level3)
{
    DTEST_LOG << "OnDnetDied_001 begin" << std::endl;
    auto channel = PrepareMissionUtChannel();
    auto& mgr = DistributedSchedMissionManager::GetInstance();
    mgr.isRegMissionChange_ = true;
    {
        std::lock_guard<std::mutex> autoLock(mgr.remoteSyncDeviceLock_);
        mgr.remoteSyncDeviceSet_.insert(DEVICE_ID);
    }
    mgr.OnDnetDied();
    EXPECT_EQ(mgr.remoteSyncDeviceSet_.size(), 1u);
    EXPECT_EQ(channel->unregisterCount_.load(), 0);
    DTEST_LOG << "OnDnetDied_001 end" << std::endl;
}

/**
 * @tc.name: CleanMissionResources_001
 * @tc.desc: CleanMissionResources returns early when the device has no listener
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, CleanMissionResources_001, TestSize.Level3)
{
    DTEST_LOG << "CleanMissionResources_001 begin" << std::endl;
    auto channel = PrepareMissionUtChannel();
    auto& mgr = DistributedSchedMissionManager::GetInstance();
    mgr.CleanMissionResources(DEVICE_ID);
    EXPECT_TRUE(mgr.listenDeviceMap_.empty());
    EXPECT_EQ(channel->uuidQueryCount_.load(), 0);
    DTEST_LOG << "CleanMissionResources_001 end" << std::endl;
}

/**
 * @tc.name: CleanMissionResources_002
 * @tc.desc: CleanMissionResources erases the device entry with a null listener
 * @tc.type: FUNC
 */
HWTEST_F(DMSMissionManagerTest, CleanMissionResources_002, TestSize.Level3)
{
    DTEST_LOG << "CleanMissionResources_002 begin" << std::endl;
    auto channel = PrepareMissionUtChannel();
    auto& mgr = DistributedSchedMissionManager::GetInstance();
    sptr<IRemoteObject> nullListener = nullptr;
    {
        std::lock_guard<std::mutex> autoLock(mgr.listenDeviceLock_);
        ListenerInfo listenerInfo;
        listenerInfo.Emplace(nullListener);
        mgr.listenDeviceMap_[U16DEVICE_ID] = listenerInfo;
    }
    mgr.CleanMissionResources(DEVICE_ID);
    EXPECT_TRUE(mgr.listenDeviceMap_.empty());
    EXPECT_GT(channel->uuidQueryCount_.load(), 0);
    DTEST_LOG << "CleanMissionResources_002 end" << std::endl;
}

} // namespace DistributedSchedule
} // namespace OHOS
