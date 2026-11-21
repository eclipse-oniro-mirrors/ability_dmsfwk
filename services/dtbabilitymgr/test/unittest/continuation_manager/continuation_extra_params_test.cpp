/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "continuation_extra_params_test.h"

#include "message_parcel.h"
#include "test_log.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace DistributedSchedule {
namespace {
const std::string MOCK_TARGET_BUNDLE = "com.example.test";
const std::string MOCK_DESCRIPTION = "description";
const std::string MOCK_FILTER = "filter";
const std::string MOCK_AUTH_INFO = "authInfo";
}

void ContinuationExtraParamsTest::SetUpTestCase()
{
    DTEST_LOG << "ContinuationExtraParamsTest::SetUpTestCase" << std::endl;
}

void ContinuationExtraParamsTest::TearDownTestCase()
{
    DTEST_LOG << "ContinuationExtraParamsTest::TearDownTestCase" << std::endl;
}

void ContinuationExtraParamsTest::SetUp()
{
    DTEST_LOG << "ContinuationExtraParamsTest::SetUp" << std::endl;
}

void ContinuationExtraParamsTest::TearDown()
{
    DTEST_LOG << "ContinuationExtraParamsTest::TearDown" << std::endl;
}

/**
 * @tc.name: ContinuationExtraParamsTest_001
 * @tc.desc: default construct and getters return empty/default values
 * @tc.type: FUNC
 */
HWTEST_F(ContinuationExtraParamsTest, ContinuationExtraParamsTest_001, TestSize.Level3)
{
    DTEST_LOG << "ContinuationExtraParamsTest_001 begin" << std::endl;
    ContinuationExtraParams params;
    EXPECT_TRUE(params.GetDeviceType().empty());
    EXPECT_EQ(params.GetTargetBundle(), "");
    EXPECT_EQ(params.GetDescription(), "");
    EXPECT_EQ(params.GetFilter(), "");
    EXPECT_EQ(params.GetAuthInfo(), "");
    DTEST_LOG << "ContinuationExtraParamsTest_001 end" << std::endl;
}

/**
 * @tc.name: ContinuationExtraParamsTest_002
 * @tc.desc: setters/getters round trip for all string fields
 * @tc.type: FUNC
 */
HWTEST_F(ContinuationExtraParamsTest, ContinuationExtraParamsTest_002, TestSize.Level3)
{
    DTEST_LOG << "ContinuationExtraParamsTest_002 begin" << std::endl;
    ContinuationExtraParams params;
    std::vector<std::string> deviceTypes = {"tablet", "phone"};
    params.SetDeviceType(deviceTypes);
    params.SetTargetBundle(MOCK_TARGET_BUNDLE);
    params.SetDescription(MOCK_DESCRIPTION);
    params.SetFilter(MOCK_FILTER);
    params.SetContinuationMode(ContinuationMode::COLLABORATION_SINGLE);
    params.SetAuthInfo(MOCK_AUTH_INFO);

    EXPECT_EQ(params.GetDeviceType(), deviceTypes);
    EXPECT_EQ(params.GetTargetBundle(), MOCK_TARGET_BUNDLE);
    EXPECT_EQ(params.GetDescription(), MOCK_DESCRIPTION);
    EXPECT_EQ(params.GetFilter(), MOCK_FILTER);
    EXPECT_EQ(params.GetContinuationMode(), ContinuationMode::COLLABORATION_SINGLE);
    EXPECT_EQ(params.GetAuthInfo(), MOCK_AUTH_INFO);
    DTEST_LOG << "ContinuationExtraParamsTest_002 end" << std::endl;
}

/**
 * @tc.name: ContinuationExtraParamsTest_003
 * @tc.desc: Marshalling then Unmarshalling round trip preserves values
 * @tc.type: FUNC
 */
HWTEST_F(ContinuationExtraParamsTest, ContinuationExtraParamsTest_003, TestSize.Level3)
{
    DTEST_LOG << "ContinuationExtraParamsTest_003 begin" << std::endl;
    ContinuationExtraParams params;
    std::vector<std::string> deviceTypes = {"tablet", "phone"};
    params.SetDeviceType(deviceTypes);
    params.SetTargetBundle(MOCK_TARGET_BUNDLE);
    params.SetDescription(MOCK_DESCRIPTION);
    params.SetFilter(MOCK_FILTER);
    params.SetContinuationMode(ContinuationMode::COLLABORATION_MUTIPLE);
    params.SetAuthInfo(MOCK_AUTH_INFO);

    MessageParcel parcel;
    EXPECT_TRUE(params.Marshalling(parcel));

    ContinuationExtraParams* restored = ContinuationExtraParams::Unmarshalling(parcel);
    ASSERT_NE(restored, nullptr);
    EXPECT_EQ(restored->GetDeviceType(), deviceTypes);
    EXPECT_EQ(restored->GetTargetBundle(), MOCK_TARGET_BUNDLE);
    EXPECT_EQ(restored->GetDescription(), MOCK_DESCRIPTION);
    EXPECT_EQ(restored->GetFilter(), MOCK_FILTER);
    EXPECT_EQ(restored->GetContinuationMode(), ContinuationMode::COLLABORATION_MUTIPLE);
    EXPECT_EQ(restored->GetAuthInfo(), MOCK_AUTH_INFO);
    delete restored;
    DTEST_LOG << "ContinuationExtraParamsTest_003 end" << std::endl;
}

/**
 * @tc.name: ContinuationExtraParamsTest_004
 * @tc.desc: Marshalling/Unmarshalling with empty device type vector
 * @tc.type: FUNC
 */
HWTEST_F(ContinuationExtraParamsTest, ContinuationExtraParamsTest_004, TestSize.Level3)
{
    DTEST_LOG << "ContinuationExtraParamsTest_004 begin" << std::endl;
    ContinuationExtraParams params;
    params.SetTargetBundle("");
    params.SetDescription("");
    params.SetFilter("");
    params.SetContinuationMode(ContinuationMode::COLLABORATION_SINGLE);
    params.SetAuthInfo("");

    MessageParcel parcel;
    EXPECT_TRUE(params.Marshalling(parcel));
    ContinuationExtraParams* restored = ContinuationExtraParams::Unmarshalling(parcel);
    ASSERT_NE(restored, nullptr);
    EXPECT_TRUE(restored->GetDeviceType().empty());
    delete restored;
    DTEST_LOG << "ContinuationExtraParamsTest_004 end" << std::endl;
}

/**
 * @tc.name: ContinuationExtraParamsTest_005
 * @tc.desc: ContinationManagerUtils Str8VecToStr16Vec / Str16VecToStr8Vec round trip
 * @tc.type: FUNC
 */
HWTEST_F(ContinuationExtraParamsTest, ContinuationExtraParamsTest_005, TestSize.Level3)
{
    DTEST_LOG << "ContinuationExtraParamsTest_005 begin" << std::endl;
    std::vector<std::string> input = {"a", "b", "c"};
    auto u16vec = ContinationManagerUtils::Str8VecToStr16Vec(input);
    EXPECT_EQ(u16vec.size(), input.size());
    auto back = ContinationManagerUtils::Str16VecToStr8Vec(u16vec);
    EXPECT_EQ(back, input);
    DTEST_LOG << "ContinuationExtraParamsTest_005 end" << std::endl;
}

/**
 * @tc.name: ContinuationExtraParamsTest_006
 * @tc.desc: ContinationManagerUtils with empty input vector
 * @tc.type: FUNC
 */
HWTEST_F(ContinuationExtraParamsTest, ContinuationExtraParamsTest_006, TestSize.Level3)
{
    DTEST_LOG << "ContinuationExtraParamsTest_006 begin" << std::endl;
    std::vector<std::string> empty;
    auto u16vec = ContinationManagerUtils::Str8VecToStr16Vec(empty);
    EXPECT_TRUE(u16vec.empty());
    auto back = ContinationManagerUtils::Str16VecToStr8Vec(u16vec);
    EXPECT_TRUE(back.empty());
    DTEST_LOG << "ContinuationExtraParamsTest_006 end" << std::endl;
}
} // namespace DistributedSchedule
} // namespace OHOS
