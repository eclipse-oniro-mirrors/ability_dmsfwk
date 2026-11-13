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

#ifndef OHOS_DISTRIBUTED_ABILITY_CONNECTION_MANAGER_NAPI_ABILITY_CONNECTION_SESSION_LISTENER_H
#define OHOS_DISTRIBUTED_ABILITY_CONNECTION_MANAGER_NAPI_ABILITY_CONNECTION_SESSION_LISTENER_H

#include <atomic>
#include <map>
#include <memory>
#include <mutex>

#include "ability_connection_info.h"
#include "native_engine/native_engine.h"
#include "js_ability_connection_session_listener.h"

namespace OHOS {
namespace DistributedCollab {
class NapiAbilityConnectionSessionListener : public JsAbilityConnectionSessionListener,
    public std::enable_shared_from_this<NapiAbilityConnectionSessionListener> {
public:
    explicit NapiAbilityConnectionSessionListener(napi_env env);
    NapiAbilityConnectionSessionListener();
    ~NapiAbilityConnectionSessionListener() override;
    void CallJsMethod(const EventCallbackInfo& eventCallbackInfo) override;
    void CallJsMethod(const CollaborateEventInfo& collaborateEventInfo) override;
    void SetCallback(const napi_value& jsListenerObj);

private:
    // EventCallbackInfo
    void CallJsMethodInner(const EventCallbackInfo& eventCallbackInfo);
    napi_value WrapEventCallbackInfo(napi_env& env, const EventCallbackInfo& eventCallbackInfo);
    // CollaborateEventInfo
    void CallJsMethodInner(const CollaborateEventInfo& collaborateEventInfo);
    napi_value WrapEventCallbackInfo(napi_env& env, const CollaborateEventInfo& collaborateEventInfo);

    template <typename T>
    void CallJsMethodTemplate(const T& callbackInfo);
    template <typename T>
    void CallJsMethodInnerTemplate(const T& callbackInfo);
    napi_value WrapAVTransDataBuffer(napi_env& env, const std::shared_ptr<AVTransDataBuffer>& dataBuffer);

private:
    struct RefHolder {
        std::mutex mtx;
        napi_env env = nullptr;
        napi_ref ref = nullptr;
        bool hookFired = false;
        void Release()
        {
            std::lock_guard<std::mutex> lock(mtx);
            if (env != nullptr && ref != nullptr) {
                napi_delete_reference(env, ref);
                ref = nullptr;
            }
        }
    };
    struct EnvCleanupCtx {
        std::shared_ptr<RefHolder> holder;
    };
    static void EnvCleanupHook(void* arg);

    napi_env env_ = nullptr;
    napi_ref callbackRef_ = nullptr;
    std::shared_ptr<RefHolder> refHolder_;
    EnvCleanupCtx* envCleanupCtx_ = nullptr;
};
} // namespace DistributedCollab
} // namespace OHOS
#endif // OHOS_DISTRIBUTED_ABILITY_CONNECTION_MANAGER_NAPI_ABILITY_CONNECTION_SESSION_LISTENER_H