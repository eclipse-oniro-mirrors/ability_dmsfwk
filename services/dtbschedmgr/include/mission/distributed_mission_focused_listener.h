/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_DISTRIBUTED_MISSION_FOCUSED_LISTENER_H
#define OHOS_DISTRIBUTED_MISSION_FOCUSED_LISTENER_H

#include <mutex>
#include <queue>
#include <thread>
#include <cstdint>

#include "event_handler.h"
#include "mission_listener_stub.h"

namespace OHOS {
namespace DistributedSchedule {
class DistributedMissionFocusedListener : public AAFwk::MissionListenerStub {
public:
    DistributedMissionFocusedListener() = default;
    virtual ~DistributedMissionFocusedListener() = default;

    /**
     * @brief When a mission is created, AbilityMs notifies the listener of the mission id
     *
     * @param missionId, mission Id.
     */
    void OnMissionCreated(int32_t missionId) override;

    /**
     * @brief When a mission is destroyed, AbilityMs notifies the listener of the mission id
     *
     * @param missionId, mission Id.
     */
    void OnMissionDestroyed(int32_t missionId) override;

    /**
     * @brief When the snapshot of a mission changes, AbilityMs notifies the listener of the mission id
     *
     * @param missionId, mission Id.
     */
    void OnMissionSnapshotChanged(int32_t missionId) override;

    /**
     * @brief When a mission is moved to front, AbilityMs notifies the listener of the mission id
     *
     * @param missionId, mission Id.
     */
    void OnMissionMovedToFront(int32_t missionId) override;

    /**
     * @brief When a mission get focused, AbilityMs notifies the listener of the mission id
     *
     * @param missionId, mission Id.
     */
    void OnMissionFocused(int32_t missionId) override;

    /**
     * @brief When a mission lose focused, AbilityMs notifies the listener of the mission id
     *
     * @param missionId, mission Id.
     */
    void OnMissionUnfocused(int32_t missionId) override;

    /**
     * @brief When a mission moved to background, AbilityMs notifies the listener of the mission id
     *
     * @param missionId, mission Id.
     */
    void OnMissionMovedToBackground(int32_t missionId) override;

#ifdef SUPPORT_DISTRIBUTED_MISSION_MANAGER
    /**
     * @brief Called when a mission has changed it's icon.
     *
     * @param missionId, mission Id.
     * @param icon, mission icon.
     */
    void OnMissionIconUpdated(int32_t missionId, const std::shared_ptr<OHOS::Media::PixelMap> &icon) override;
#endif

    /**
     * @brief When a mission is closed, AbilityMs notifies the listener of the mission id
     *
     * @param missionId, mission Id.
     */
    virtual void OnMissionClosed(int32_t missionId) override;

    /**
     * @brief When a mission's label changed, AbilityMs notifies the listener of the mission id
     *
     * @param missionId, mission Id.
     */
    virtual void OnMissionLabelUpdated(int32_t missionId) override;

    void Init();
    void UnInit();

private:
    void StartEvent();
    std::thread eventThread_;
    std::condition_variable eventCon_;
    std::mutex eventMutex_;
    std::shared_ptr<OHOS::AppExecFwk::EventHandler> eventHandler_;
};
} // namespace DistributedSchedule
} // namespace OHOS
#endif // OHOS_DISTRIBUTED_MISSION_FOCUSED_LISTENER_H
