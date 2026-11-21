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

#include "continuation_result_test.h"

#include "message_parcel.h"
#include "test_log.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace DistributedSchedule {
namespace {
const std::string MOCK_DEVICE_ID = "deviceId123";
const std::string MOCK_DEVICE_TYPE = "tablet";
const std::string MOCK_DEVICE_NAME = "MyTablet";
}

void ContinuationResultTest::SetUpTestCase()
{
    DTEST_LOG << "ContinuationResultTest::SetUpTestCase" << std::endl;
}

void ContinuationResultTest::TearDownTestCase()
{
    DTEST_LOG << "ContinuationResultTest::TearDownTestCase" << std::endl;
}

void ContinuationResultTest::SetUp()
{
    DTEST_LOG << "ContinuationResultTest::SetUp" << std::endl;
}

void ContinuationResultTest::TearDown()
{
    DTEST_LOG << "ContinuationResultTest::TearDown" << std::endl;
}

/**
 * @tc.name: ContinuationResultTest_001
 * @tc.desc: default construct and getters return empty
 * @tc.type: FUNC
 */
HWTEST_F(ContinuationResultTest, ContinuationResultTest_001, TestSize.Level3)
{
    DTEST_LOG << "ContinuationResultTest_001 begin" << std::endl;
    ContinuationResult result;
    EXPECT_EQ(result.GetDeviceId(), "");
    EXPECT_EQ(result.GetDeviceType(), "");
    EXPECT_EQ(result.GetDeviceName(), "");
    DTEST_LOG << "ContinuationResultTest_001 end" << std::endl;
}

/**
 * @tc.name: ContinuationResultTest_002
 * @tc.desc: setters/getters round trip
 * @tc.type: FUNC
 */
HWTEST_F(ContinuationResultTest, ContinuationResultTest_002, TestSize.Level3)
{
    DTEST_LOG << "ContinuationResultTest_002 begin" << std::endl;
    ContinuationResult result;
    result.SetDeviceId(MOCK_DEVICE_ID);
    result.SetDeviceType(MOCK_DEVICE_TYPE);
    result.SetDeviceName(MOCK_DEVICE_NAME);
    EXPECT_EQ(result.GetDeviceId(), MOCK_DEVICE_ID);
    EXPECT_EQ(result.GetDeviceType(), MOCK_DEVICE_TYPE);
    EXPECT_EQ(result.GetDeviceName(), MOCK_DEVICE_NAME);
    DTEST_LOG << "ContinuationResultTest_002 end" << std::endl;
}

/**
 * @tc.name: ContinuationResultTest_003
 * @tc.desc: Marshalling then Unmarshalling preserves values
 * @tc.type: FUNC
 */
HWTEST_F(ContinuationResultTest, ContinuationResultTest_003, TestSize.Level3)
{
    DTEST_LOG << "ContinuationResultTest_003 begin" << std::endl;
    ContinuationResult result;
    result.SetDeviceId(MOCK_DEVICE_ID);
    result.SetDeviceType(MOCK_DEVICE_TYPE);
    result.SetDeviceName(MOCK_DEVICE_NAME);

    MessageParcel parcel;
    EXPECT_TRUE(result.Marshalling(parcel));
    ContinuationResult* restored = ContinuationResult::Unmarshalling(parcel);
    ASSERT_NE(restored, nullptr);
    EXPECT_EQ(restored->GetDeviceId(), MOCK_DEVICE_ID);
    EXPECT_EQ(restored->GetDeviceType(), MOCK_DEVICE_TYPE);
    EXPECT_EQ(restored->GetDeviceName(), MOCK_DEVICE_NAME);
    delete restored;
    DTEST_LOG << "ContinuationResultTest_003 end" << std::endl;
}

/**
 * @tc.name: ContinuationResultTest_004
 * @tc.desc: WriteContinuationResultsToParcel with empty vector writes VALUE_NULL
 * @tc.type: FUNC
 */
HWTEST_F(ContinuationResultTest, ContinuationResultTest_004, TestSize.Level3)
{
    DTEST_LOG << "ContinuationResultTest_004 begin" << std::endl;
    MessageParcel parcel;
    std::vector<ContinuationResult> results;
    EXPECT_TRUE(ContinuationResult::WriteContinuationResultsToParcel(parcel, results));
    std::vector<ContinuationResult> readResults;
    EXPECT_TRUE(ContinuationResult::ReadContinuationResultsFromParcel(parcel, readResults));
    EXPECT_TRUE(readResults.empty());
    DTEST_LOG << "ContinuationResultTest_004 end" << std::endl;
}

/**
 * @tc.name: ContinuationResultTest_005
 * @tc.desc: Write/Read a vector of multiple ContinuationResults
 * @tc.type: FUNC
 */
HWTEST_F(ContinuationResultTest, ContinuationResultTest_005, TestSize.Level3)
{
    DTEST_LOG << "ContinuationResultTest_005 begin" << std::endl;
    std::vector<ContinuationResult> results;
    ContinuationResult r1;
    r1.SetDeviceId("dev1");
    r1.SetDeviceType("phone");
    r1.SetDeviceName("Phone1");
    results.push_back(r1);
    ContinuationResult r2;
    r2.SetDeviceId("dev2");
    r2.SetDeviceType("tablet");
    r2.SetDeviceName("Tablet2");
    results.push_back(r2);

    MessageParcel parcel;
    EXPECT_TRUE(ContinuationResult::WriteContinuationResultsToParcel(parcel, results));
    std::vector<ContinuationResult> readResults;
    EXPECT_TRUE(ContinuationResult::ReadContinuationResultsFromParcel(parcel, readResults));
    EXPECT_EQ(readResults.size(), results.size());
    EXPECT_EQ(readResults[0].GetDeviceId(), "dev1");
    EXPECT_EQ(readResults[1].GetDeviceName(), "Tablet2");
    DTEST_LOG << "ContinuationResultTest_005 end" << std::endl;
}
} // namespace DistributedSchedule
} // namespace OHOS
