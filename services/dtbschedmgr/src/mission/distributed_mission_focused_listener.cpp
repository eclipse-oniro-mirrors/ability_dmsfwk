/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "mission/distributed_mission_focused_listener.h"

#include "dtbschedmgr_log.h"
#include "mission/distributed_sched_continue_manager.h"

namespace OHOS {
namespace DistributedSchedule {
namespace {
const std::string TAG = "DistributedMissionFocusedListener";
}
void DistributedMissionFocusedListener::OnMissionCreated(int32_t missionId)
{
    HILOGI("OnMissionCreated, missionId = %{public}d", missionId);
}

void DistributedMissionFocusedListener::OnMissionDestroyed(int32_t missionId)
{
    HILOGI("OnMissionDestroyed, missionId = %{public}d", missionId);
}

void DistributedMissionFocusedListener::OnMissionSnapshotChanged(int32_t missionId)
{
    HILOGI("OnMissionSnapshotChanged, missionId = %{public}d", missionId);
}

void DistributedMissionFocusedListener::OnMissionMovedToFront(int32_t missionId)
{
    HILOGI("OnMissionMovedToFront, missionId = %{public}d", missionId);
}

void DistributedMissionFocusedListener::OnMissionFocused(int32_t missionId)
{
    HILOGI("OnMissionFocused, missionId = %{public}d", missionId);
    DistributedSchedContinueManager::GetInstance().NotifyMissionFocused(missionId);
}

void DistributedMissionFocusedListener::OnMissionUnfocused(int32_t missionId)
{
    HILOGI("OnMissionUnFocused, missionId = %{public}d", missionId);
    DistributedSchedContinueManager::GetInstance().NotifyMissionUnfocused(missionId);
}

#ifdef SUPPORT_DISTRIBUTED_MISSION_MANAGER
void DistributedMissionFocusedListener::OnMissionIconUpdated(int32_t missionId,
    const std::shared_ptr<OHOS::Media::PixelMap> &icon)
{
}
#endif

void DistributedMissionFocusedListener::OnMissionClosed(int32_t missionId)
{
}

void DistributedMissionFocusedListener::OnMissionLabelUpdated(int32_t missionId)
{
}
}
}