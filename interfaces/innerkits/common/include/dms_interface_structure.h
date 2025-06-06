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

#ifndef OHOS_DMS_INTERFACE_STRUCTURE_H
#define OHOS_DMS_INTERFACE_STRUCTURE_H

#include <string>

namespace OHOS {
namespace DistributedCollab {
typedef struct {
    int32_t (*SetFileSchema)(int32_t socketId);
} ISoftbusFileAdpater;
} // namespace DistributedCollab
} // namespace OHOS
#endif // OHOS_DMS_INTERFACE_STRUCTURE_H

