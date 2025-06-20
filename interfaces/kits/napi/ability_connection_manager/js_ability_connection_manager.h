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

#ifndef OHOS_DISTRIBUTED_COLLAB_SESSION_MANAGER_JS_ABILITY_CONNECTION_MANAGER_H
#define OHOS_DISTRIBUTED_COLLAB_SESSION_MANAGER_JS_ABILITY_CONNECTION_MANAGER_H

#include <utility>

#include "ability_connection_info.h"
#include "ability_info.h"
#include "js_ability_connection_session_listener.h"
#include "native_engine/native_engine.h"
#include "pixel_map_napi.h"

namespace OHOS {
namespace DistributedCollab {
using JsAbilityConnectionSessionListenerPtr = std::shared_ptr<JsAbilityConnectionSessionListener>;
struct AsyncConnectCallbackInfo {
    napi_async_work asyncWork = nullptr;
    napi_deferred deferred = nullptr;
    napi_threadsafe_function tsfn = nullptr;
    int32_t sessionId;
    ConnectResult result;
    // Indicates whether ConnectThreadsafeFunctionCallback has been executed.
    bool connectCallbackExecuted = false;
    // Marks whether CompleteAsyncConnectWork has been executed
    bool completeAsyncworkExecuted = false;
};

struct AsyncCallbackInfo {
    napi_async_work asyncWork = nullptr;
    napi_deferred deferred = nullptr;
    napi_threadsafe_function tsfn;
    int32_t result;
    int32_t sessionId;
    std::string token;
    std::string msg;
    std::shared_ptr<AVTransDataBuffer> buffer = nullptr;
    std::shared_ptr<Media::PixelMap> image = nullptr;
    int32_t streamId;
    StreamParams streamParam;
    int32_t imageQuality = 30;
};

enum BussinessErrorCode {
    // Permission verification failed.
    ERR_INVALID_PERMISSION = 201,
    // The caller is not a system application.
    ERR_NOT_SYSTEM_APP = 202,
    // Input parameter error.
    ERR_INVALID_PARAMS = 401,
    // Multiple streams can not be created.
    ERR_ONLY_SUPPORT_ONE_STREAM = 32300001,
    // The stream at the receive end is not started.
    ERR_RECEIVE_STREAM_NOT_START = 32300002,
    // Multiple streams can not be created.
    ERR_BITATE_NOT_SUPPORTED = 32300003,
    // The stream at the receive end is not started.
    ERR_COLOR_SPACE_NOT_SUPPORTED = 32300004,
};

class JsAbilityConnectionManager final {
public:
    JsAbilityConnectionManager() = default;
    ~JsAbilityConnectionManager() = default;

    static napi_value CreateAbilityConnectionSession(napi_env env, napi_callback_info info);
    static napi_value DestroyAbilityConnectionSession(napi_env env, napi_callback_info info);
    static napi_value GetPeerInfoById(napi_env env, napi_callback_info info);
    static napi_value RegisterAbilityConnectionSessionCallback(napi_env env, napi_callback_info info);
    static napi_value UnregisterAbilityConnectionSessionCallback(napi_env env, napi_callback_info info);
    static napi_value Connect(napi_env env, napi_callback_info info);
    static napi_value ConnectInner(napi_env env, int32_t sessionId, const std::string &transId, time_t beginTime,
        int64_t processorId);
    static napi_value DisConnect(napi_env env, napi_callback_info info);
    static napi_value AcceptConnect(napi_env env, napi_callback_info info);
    static napi_value Reject(napi_env env, napi_callback_info info);
    static napi_value SendMessage(napi_env env, napi_callback_info info);
    static napi_value SendData(napi_env env, napi_callback_info info);
    static napi_value SendImage(napi_env env, napi_callback_info info);
    static napi_value CreateStream(napi_env env, napi_callback_info info);
    static napi_value SetSurfaceId(napi_env env, napi_callback_info info);
    static napi_value GetSurfaceId(napi_env env, napi_callback_info info);
    static napi_value UpdateSurfaceParam(napi_env env, napi_callback_info info);
    static napi_value DestroyStream(napi_env env, napi_callback_info info);
    static napi_value StartStream(napi_env env, napi_callback_info info);
    static napi_value StopStream(napi_env env, napi_callback_info info);

    static napi_value OnCreateAbilityConnectionSession(napi_env env, napi_callback_info info);

private:
    static bool JsToServiceName(const napi_env &env, const napi_value &jsValue,
        std::string& serviceName);
    static bool JsToAbilityInfo(const napi_env &env, const napi_value &jsValue,
        std::shared_ptr<OHOS::AppExecFwk::AbilityInfo>& abilityInfo);
    static bool JsToPeerInfo(const napi_env &env, const napi_value &jsValue, PeerInfo &peerInfo);
    static int32_t JSToConnectOption(const napi_env &env, const napi_value &jsValue, ConnectOption &option);
    static bool JsToStreamParam(const napi_env &env, const napi_value &jsValue, StreamParams &streamParam);
    static bool GetStreamParamBitrate(const napi_env &env, const napi_value &jsValue, StreamParams &streamParam);
    static bool JsToSurfaceParam(const napi_env &env, const napi_value &jsValue, SurfaceParams &surfaceParam);

    static void ConnectThreadsafeFunctionCallback(napi_env env, napi_value js_callback, void* context, void* data);
    static napi_status CreateConnectThreadsafeFunction(napi_env env, napi_value js_func,
        napi_threadsafe_function* tsfn);
    static void ExecuteConnect(napi_env env, void *data);
    static void CompleteAsyncConnectWork(napi_env env, napi_status status, void* data);
    static void ExecuteAcceptConnect(napi_env env, void *data);
    static void ExecuteSendMessage(napi_env env, void *data);
    static void ExecuteSendData(napi_env env, void *data);
    static void ExecuteSendImage(napi_env env, void *data);
    static void CompleteAsyncWork(napi_env env, napi_status status, void* data);
    static void ExecuteCreateStream(napi_env env, void *data);
    static void CompleteAsyncCreateStreamWork(napi_env env, napi_status status, void* data);
    static void CreateSendDataAsyncWork(napi_env env, AsyncCallbackInfo* asyncCallbackInfo);
    static void CreateStreamAsyncWork(napi_env env, AsyncCallbackInfo* asyncCallbackInfo);
    static napi_value CreateSendImageAsyncWork(napi_env env, AsyncCallbackInfo* asyncCallbackInfo);

    static bool IsSystemApp();
    static bool JsToInt32(const napi_env &env, const napi_value &value,
        const std::string &valueName, int32_t &strValue);
    static bool JsToString(const napi_env &env, const napi_value &value, const std::string &valueName,
        std::string &strValue);
    static bool JsObjectToString(const napi_env &env, const napi_value &object, const std::string &fieldStr,
        std::string& fieldRef);
    static bool JsObjectToBool(const napi_env &env, const napi_value &object, const std::string &fieldStr,
        bool &fieldRef);
    static bool JsObjectToInt(const napi_env &env, const napi_value &object, const std::string &fieldStr,
        int32_t &fieldRef);
    static int32_t CheckEventType(const std::string& eventType);
    static bool UnwrapOptions(napi_env env, napi_value options, ConnectOption &connectOption);
    static bool UnwrapStartOptions(napi_env env, napi_value startOptionsVal, ConnectOption &connectOption);
    static bool UnwrapParameters(napi_env env, napi_value parameters, ConnectOption &option);
    static bool IsTypeForNapiValue(napi_env env, napi_value param, napi_valuetype expectType);
    static napi_value WrapPeerInfo(napi_env& env, const PeerInfo& peerInfo);
    static int32_t CheckConnectOption(const ConnectOption &connectOption);
    static napi_value ExecuteCreateSession(const std::string& serviceName,
        std::shared_ptr<OHOS::AppExecFwk::AbilityInfo>& abilityInfo, PeerInfo& peerInfo,
        ConnectOption& connectOption, const napi_env& env);
    static void CleanupConnectionResources(napi_env env, AsyncConnectCallbackInfo* asyncData,
        napi_threadsafe_function tsfn);
    static bool IsVaildBitrate(int32_t bitrate);
    static bool UnwrapColorSpace(const napi_env &env, const napi_value &jsValue,
        StreamParams &streamParam);
};

napi_value JsAbilityConnectionManagerInit(napi_env env, napi_value exportObj);
} // namespace DistributedCollab
} // namespace OHOS
#endif // OHOS_DISTRIBUTED_COLLAB_SESSION_MANAGER_JS_ABILITY_CONNECTION_MANAGER_H