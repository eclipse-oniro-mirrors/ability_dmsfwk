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

#include "av_trans_stream_data_test.h"
#include "dtbcollabmgr_log.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace DistributedCollab {
    namespace {
        static const std::string TAG = "DSchedCollabAVTransStreamDataTest";
    }
    void AVTransStreamDataTest::SetUpTestCase()
    {
        HILOGI("AVTransStreamDataTest::SetUpTestCase");
    }

    void AVTransStreamDataTest::TearDownTestCase()
    {
        HILOGI("AVTransStreamDataTest::TearDownTestCase");
    }

    void AVTransStreamDataTest::SetUp()
    {
        HILOGI("AVTransStreamDataTest::SetUp");
        auto buffer = std::make_shared<AVTransDataBuffer>(1024); // 设置容量
        extData_ = { AvCodecBufferFlag::AVCODEC_BUFFER_FLAG_SYNC_FRAME, 10, 1000, 2000, 3000, 4000 };
        streamData_ = std::make_shared<AVTransStreamData>(buffer, extData_);
    }

    void AVTransStreamDataTest::TearDown()
    {
        HILOGI("AVTransStreamDataTest::TearDown");
        streamData_.reset();
    }

    /**
     * @tc.name: SerializeStreamDataExt_001
     * @tc.desc: test serialize stream data ext
     * @tc.type: FUNC
     */
    HWTEST_F(AVTransStreamDataTest, SerializeStreamDataExt_Success, TestSize.Level1)
    {
        cJSON* jsonData = streamData_->SerializeStreamDataExt();
        ASSERT_NE(jsonData, nullptr);

        cJSON* flag = cJSON_GetObjectItem(jsonData, "flag");
        ASSERT_NE(flag, nullptr);
        EXPECT_EQ(flag->valueint, static_cast<int>(extData_.flag_));

        cJSON* index = cJSON_GetObjectItem(jsonData, "index");
        ASSERT_NE(index, nullptr);
        EXPECT_EQ(index->valueint, extData_.index_);

        cJSON_Delete(jsonData);
    }

    /**
     * @tc.name: DeserializeStreamDataExt_Success
     * @tc.desc: DeserializeStreamDataExt
     * @tc.type: FUNC
     */
    HWTEST_F(AVTransStreamDataTest, DeserializeStreamDataExt_Success, TestSize.Level1)
    {
        cJSON* jsonData = streamData_->SerializeStreamDataExt();
        int32_t indexVal = 10;
        char* jsonString = cJSON_PrintUnformatted(jsonData);

        int32_t result = streamData_->DeserializeStreamDataExt(jsonString);
        EXPECT_EQ(result, ERR_OK);
        EXPECT_EQ(indexVal, streamData_->GetStreamDataExt().index_);
        cJSON_Delete(jsonData);
        free(jsonString);
    }

    /**
     * @tc.name: DeserializeStreamDataExt_Null_Pointer
     * @tc.desc: DeserializeStreamDataExt when nullptr
     * @tc.type: FUNC
     */
    HWTEST_F(AVTransStreamDataTest, DeserializeStreamDataExt_Null_Pointer, TestSize.Level1)
    {
        int32_t result = streamData_->DeserializeStreamDataExt(nullptr);
        EXPECT_EQ(result, NULL_POINTER_ERROR);
    }

    /**
     * @tc.name: SeAndDeserializeStreamDataExt_PixelMap_Success
     * @tc.desc: DeserializeStreamDataExt when nullptr
     * @tc.type: FUNC
     */
    HWTEST_F(AVTransStreamDataTest, SeAndDeserializeStreamDataExt_PixelMap_Success, TestSize.Level1)
    {
        auto buffer = std::make_shared<AVTransDataBuffer>(1024); // 设置容量
        PixelMapPackOption option = { 100 };
        extData_.flag_ = AvCodecBufferFlag::AVCODEC_BUFFER_FLAG_PIXEL_MAP;
        extData_.index_ = 1;
        extData_.pixelMapOption_ = option;
        streamData_ = std::make_shared<AVTransStreamData>(buffer, extData_);

        cJSON* jsonData = streamData_->SerializeStreamDataExt();
        ASSERT_NE(jsonData, nullptr);

        char* jsonString = cJSON_PrintUnformatted(jsonData);

        int32_t result = streamData_->DeserializeStreamDataExt(jsonString);
        EXPECT_EQ(result, ERR_OK);
        EXPECT_EQ(option.quality, streamData_->GetStreamDataExt().pixelMapOption_.quality);
        cJSON_Delete(jsonData);
        free(jsonString);
    }

    /**
     * @tc.name: SeAndDeserializeStreamDataExt_SurfaceParam_Success
     * @tc.desc: DeserializeStreamDataExt when nullptr
     * @tc.type: FUNC
     */
    HWTEST_F(AVTransStreamDataTest, SeAndDeserializeStreamDataExt_SurfaceParam_Success, TestSize.Level1)
    {
        auto buffer = std::make_shared<AVTransDataBuffer>(1024); // 设置容量
        SurfaceParam param = { SurfaceRotate::ROTATE_90, SurfaceFilp::FLIP_H };
        extData_.flag_ = AvCodecBufferFlag::AVCODEC_BUFFER_FLAG_SURFACE_PARAM;
        extData_.index_ = 1;
        extData_.surfaceParam_ = param;
        streamData_ = std::make_shared<AVTransStreamData>(buffer, extData_);

        cJSON* jsonData = streamData_->SerializeStreamDataExt();
        ASSERT_NE(jsonData, nullptr);

        char* jsonString = cJSON_PrintUnformatted(jsonData);

        int32_t result = streamData_->DeserializeStreamDataExt(jsonString);
        EXPECT_EQ(result, ERR_OK);
        EXPECT_EQ(param.rotate, streamData_->GetStreamDataExt().surfaceParam_.rotate);
        EXPECT_EQ(param.filp, streamData_->GetStreamDataExt().surfaceParam_.filp);
        cJSON_Delete(jsonData);
        free(jsonString);
    }
} // namespace DistributedCollab
}