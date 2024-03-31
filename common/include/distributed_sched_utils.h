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

#ifndef OHOS_DMS_UTILS_H
#define OHOS_DMS_UTILS_H

#include <string>

namespace OHOS {
namespace DistributedSchedule {
constexpr static int32_t INVALID_MISSION_ID = -1;

bool IsValidPath(const std::string &inFilePath, std::string &realFilePath);
bool UpdateAllowAppList(const std::string &cfgJsonStr);
int32_t LoadContinueConfig();
bool CheckBundleContinueConfig(const std::string &bundleName);
int32_t GetCurrentMissionId();
} // namespace DistributedSchedule
} // namespace OHOS
#endif // OHOS_DISTRIBUTED_SCHED_SERVICE_H