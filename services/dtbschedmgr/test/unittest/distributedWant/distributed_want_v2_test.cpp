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

#include <gtest/gtest.h>

#include "string_wrapper.h"
#include "base_obj.h"
#include "bool_wrapper.h"
#include "int_wrapper.h"
#include "float_wrapper.h"
#include "long_wrapper.h"
#include "array_wrapper.h"

#define private public
#define protected public
#include "distributed_operation.h"
#include "distributed_want_v2.h"
#include "want.h"
#include "want_params.h"
#undef private
#undef protected

using namespace testing::ext;
using namespace OHOS::AAFwk;
using namespace OHOS;
using OHOS::Parcel;
using OHOS::AppExecFwk::ElementName;

namespace OHOS {
namespace DistributedSchedule {
class DistributedWantV2BaseTest : public testing::Test {
public:
    DistributedWantV2BaseTest()
    {}
    ~DistributedWantV2BaseTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::shared_ptr<DistributedWantV2> want_ = nullptr;

    void CompareWant(const std::shared_ptr<DistributedWantV2> &want1,
        const std::shared_ptr<DistributedWantV2> &want2) const;
    bool CompareWant(const std::shared_ptr<DistributedWantV2> &want1,
        const std::shared_ptr<DistributedWantV2> &want2, std::map<std::string, std::string> &keys) const;
    bool CompareWantOthers(const std::shared_ptr<DistributedWantV2>& want1,
        const std::shared_ptr<DistributedWantV2>& want2, std::map<std::string, std::string>::iterator it) const;
    bool CompareWantNumber(const std::shared_ptr<DistributedWantV2>& want1,
        const std::shared_ptr<DistributedWantV2>& want2, std::map<std::string, std::string>::iterator it) const;
    bool CompareWantArray(const std::shared_ptr<DistributedWantV2>& want1,
        const std::shared_ptr<DistributedWantV2>& want2, std::map<std::string, std::string>::iterator it) const;
    void SendParcelTest(const std::shared_ptr<DistributedWantV2> &want,
        std::map<std::string, std::string> &keys) const;
    void AddBoolParams(DistributedWantV2 &want,
        std::map<std::string, std::string> &keys, int loop, unsigned int flag) const;
    void AddByteParams(DistributedWantV2 &want,
        std::map<std::string, std::string> &keys, int loop, unsigned int flag) const;
    void AddCharParams(DistributedWantV2 &want,
        std::map<std::string, std::string> &keys, int loop, unsigned int flag) const;
    void AddShortParams(DistributedWantV2 &want,
        std::map<std::string, std::string> &keys, int loop, unsigned int flag) const;
    void AddIntParams(DistributedWantV2 &want,
        std::map<std::string, std::string> &keys, int loop, unsigned int flag) const;
    void AddLongParams(DistributedWantV2 &want,
        std::map<std::string, std::string> &keys, int loop, unsigned int flag) const;
    void AddFloatParams(DistributedWantV2 &want,
        std::map<std::string, std::string> &keys, int loop, unsigned int flag) const;
    void AddDoubleParams(DistributedWantV2 &want,
        std::map<std::string, std::string> &keys, int loop, unsigned int flag) const;
    void AddStringParams(DistributedWantV2 &want,
        std::map<std::string, std::string> &keys, int loop, unsigned int flag) const;
    void TestStringForParseUri(std::string uri,
        std::string keyString, std::size_t &length, std::string valueStringOrigin);
    void TestFloatForParseUri(std::string uri,
        std::string keyFloat, std::size_t &length, float valueFloatOrigin);
    void TestFloatArrayForParseUri(std::string uri,
        std::string keyFloatArray, std::size_t &length, std::vector<float> valueFloatArrayOrigin);
    void TestStringArrayForParseUri(std::string uri,
        std::string keyStringArray, std::size_t &length, std::vector<std::string> valueStringArrayOrigin);

    std::string boolType = "bool";
    std::string boolArrayType = "boolArray";
    std::string byteType = "byte";
    std::string byteArrayType = "byteArray";
    std::string charType = "char";
    std::string charArrayType = "charArray";
    std::string shortType = "short";
    std::string shortArrayType = "shortArray";
    std::string intType = "int";
    std::string intArrayType = "intArray";
    std::string longType = "long";
    std::string longArrayType = "longArray";
    std::string floatType = "float";
    std::string floatArrayType = "floatArray";
    std::string doubleType = "double";
    std::string doubleArrayType = "doubleArray";
    std::string stringType = "string";
    std::string stringArrayType = "stringArray";

    static const std::string URI_STRING_HEAD;
    static const std::string URI_STRING_END;
};

void DistributedWantV2BaseTest::SetUpTestCase(void)
{}

void DistributedWantV2BaseTest::TearDownTestCase(void)
{}

void DistributedWantV2BaseTest::SetUp(void)
{
    want_ = std::make_shared<DistributedWantV2>();
}

void DistributedWantV2BaseTest::TearDown(void)
{}

template<typename T>
bool CompareArrayData(const std::vector<T> &arr1, const std::vector<T> &arr2)
{
    if (arr1.size() != arr2.size()) {
        return false;
    }
    for (std::uint32_t i = 0; i < arr1.size(); i++) {
        if (arr1[i] != arr2[i]) {
            return false;
        }
    }
    return true;
};

enum Type { FLAG_TEST_SINGLE = 0x01, FLAG_TEST_ARRAY, FLAG_TEST_BOTH };
const std::string DistributedWantV2BaseTest::URI_STRING_HEAD("#Intent;");
const std::string DistributedWantV2BaseTest::URI_STRING_END(";end");

/**
 * @tc.number: DistributedSchedule_DistributedWantV2_Convert_0100
 * @tc.name: distributedwantV2
 * @tc.desc: Verifying successful conversion of want to distributedwantV2.
 */
HWTEST_F(DistributedWantV2BaseTest, DistributedSchedule_DistributedWantV2_Convert_0100, Function | MediumTest | Level3)
{
    Want want;
    std::string description = "liuuy";
    want.SetType(description);
    want.SetFlags(16);
    want.SetElementName("deviceID", "com.bundle.com", "com.bundle.com.ability");
    want.SetUri("wantUri");
    want.SetAction("Action");
    want.AddEntity("Entity1");
    want.SetParam(boolType, true);
    want.SetParam(byteType, 1);
    want.SetParam(charType, 6);
    want.SetParam(shortType, 444);
    want.SetParam(intType, 1111);
    want.SetParam(longType, 12345);
    want.SetParam(floatType, 1.1);
    want.SetParam(doubleType, 1.11);
    want.SetParam(stringType, std::string("string..."));
    std::vector<double> dbv;
    dbv.emplace_back(1111.11);
    dbv.emplace_back(2222.22);
    dbv.emplace_back(3333.33);
    dbv.emplace_back(4444.44);
    want.SetParam(doubleArrayType, dbv);
    std::vector<std::string> strv;
    strv.emplace_back("1111.11");
    strv.emplace_back("2222.22");
    want.SetParam(stringArrayType, strv);
    DistributedWantV2 dstbWant(want);
    EXPECT_STREQ(want.GetType().c_str(), dstbWant.GetType().c_str());
    EXPECT_STREQ(want.GetElement().GetDeviceID().c_str(), dstbWant.GetElement().GetDeviceID().c_str());
    EXPECT_STREQ(want.GetElement().GetBundleName().c_str(), dstbWant.GetElement().GetBundleName().c_str());
    EXPECT_STREQ(want.GetElement().GetAbilityName().c_str(), dstbWant.GetElement().GetAbilityName().c_str());
    EXPECT_EQ(want.GetFlags(), dstbWant.GetFlags());
    EXPECT_STREQ(want.GetUriString().c_str(), dstbWant.GetUriString().c_str());
    EXPECT_STREQ(want.GetAction().c_str(), dstbWant.GetAction().c_str());
    EXPECT_EQ(want.GetEntities().size(), dstbWant.GetEntities().size());
    EXPECT_EQ(want.GetBoolParam(boolType, false), dstbWant.GetBoolParam(boolType, true));
    EXPECT_EQ(want.GetByteParam(byteType, 0), dstbWant.GetByteParam(byteType, 0));
    EXPECT_EQ(want.GetCharParam(charType, 0), dstbWant.GetCharParam(charType, 0));
    EXPECT_EQ(want.GetShortParam(shortType, 0), dstbWant.GetShortParam(shortType, 0));
    EXPECT_EQ(want.GetIntParam(intType, 0), dstbWant.GetIntParam(intType, 0));
    EXPECT_EQ(want.GetLongParam(longType, 0), dstbWant.GetLongParam(longType, 0));
    EXPECT_EQ(want.GetFloatParam(floatType, 0), dstbWant.GetFloatParam(floatType, 0));
    EXPECT_EQ(want.GetDoubleParam(doubleType, 0), dstbWant.GetDoubleParam(doubleType, 0));
    EXPECT_STREQ(want.GetStringParam(stringType).c_str(), dstbWant.GetStringParam(stringType).c_str());
    EXPECT_EQ(want.GetDoubleArrayParam(doubleArrayType).size(), dstbWant.GetDoubleArrayParam(doubleArrayType).size());
    EXPECT_EQ(want.GetStringArrayParam(stringArrayType).size(), dstbWant.GetStringArrayParam(stringArrayType).size());
}

/**
 * @tc.number: DistributedSchedule_DistributedWantV2_Convert_0101
 * @tc.name: distributedwantV2
 * @tc.desc: Verifying successful conversion of want to distributedwantV2.
 */
HWTEST_F(DistributedWantV2BaseTest, DistributedSchedule_DistributedWantV2_Convert_0101, Function | MediumTest | Level3)
{
    Want want;
    std::vector<bool> bv;
    bv.emplace_back(true);
    bv.emplace_back(false);
    want.SetParam(boolArrayType, bv);
    std::vector<byte> byv;
    byv.emplace_back(2);
    byv.emplace_back(3);
    want.SetParam(byteArrayType, byv);
    std::vector<zchar> chv;
    chv.emplace_back('\n');
    chv.emplace_back('p');
    chv.emplace_back('i');
    want.SetParam(charArrayType, chv);
    std::vector<short> shv;
    shv.emplace_back(111);
    shv.emplace_back(222);
    shv.emplace_back(333);
    want.SetParam(shortArrayType, shv);
    std::vector<int> inv;
    inv.emplace_back(1111);
    inv.emplace_back(2222);
    inv.emplace_back(3333);
    inv.emplace_back(4444);
    want.SetParam(intArrayType, inv);
    std::vector<long> lgv;
    lgv.emplace_back(1111);
    lgv.emplace_back(2222);
    lgv.emplace_back(3333);
    lgv.emplace_back(4444);
    want.SetParam(longArrayType, lgv);
    std::vector<float> ftv;
    ftv.emplace_back(1111.1);
    ftv.emplace_back(2222.2);
    ftv.emplace_back(3333.3);
    ftv.emplace_back(4444.4);
    want.SetParam(floatArrayType, ftv);
    DistributedWantV2 dstbWant(want);
    EXPECT_EQ(want.GetBoolArrayParam(boolArrayType).size(), dstbWant.GetBoolArrayParam(boolArrayType).size());
    EXPECT_EQ(want.GetByteArrayParam(byteArrayType).size(), dstbWant.GetByteArrayParam(byteArrayType).size());
    EXPECT_EQ(want.GetCharArrayParam(charArrayType).size(), dstbWant.GetCharArrayParam(charArrayType).size());
    EXPECT_EQ(want.GetShortArrayParam(shortArrayType).size(), dstbWant.GetShortArrayParam(shortArrayType).size());
    EXPECT_EQ(want.GetIntArrayParam(intArrayType).size(), dstbWant.GetIntArrayParam(intArrayType).size());
    EXPECT_EQ(want.GetLongArrayParam(longArrayType).size(), dstbWant.GetLongArrayParam(longArrayType).size());
    EXPECT_EQ(want.GetFloatArrayParam(floatArrayType).size(), dstbWant.GetFloatArrayParam(floatArrayType).size());
}

/**
 * @tc.number: DistributedSchedule_DistributedWantV2_Convert_0200
 * @tc.name: ToWant
 * @tc.desc: Verifying successful conversion of distributedwantV2 to want.
 */
HWTEST_F(DistributedWantV2BaseTest, DistributedSchedule_Distributedwant_Convert_0200, Function | MediumTest | Level3)
{
    std::shared_ptr<DistributedWantV2> dwant = std::make_shared<DistributedWantV2>();
    ASSERT_NE(dwant, nullptr);
    std::string description = "liuuy";
    dwant->SetType(description);
    dwant->SetFlags(16);
    dwant->SetElementName("deviceID", "com.bundle.com", "com.bundle.com.ability");
    dwant->SetUri("wantUri");
    dwant->SetAction("Action");
    dwant->AddEntity("Entity1");
    dwant->SetParam(boolType, true);
    dwant->SetParam(byteType, 1);
    dwant->SetParam(charType, 6);
    dwant->SetParam(shortType, 444);
    dwant->SetParam(intType, 1111);
    dwant->SetParam(longType, 12345);
    dwant->SetParam(floatType, 1.1);
    dwant->SetParam(doubleType, 1.11);
    dwant->SetParam(stringType, std::string("string..."));
    std::vector<double> dbv;
    dbv.emplace_back(1111.11);
    dbv.emplace_back(2222.22);
    dbv.emplace_back(3333.33);
    dbv.emplace_back(4444.44);
    dwant->SetParam(doubleArrayType, dbv);
    std::vector<std::string> strv;
    strv.emplace_back("1111.11");
    strv.emplace_back("2222.22");
    dwant->SetParam(stringArrayType, strv);
    std::shared_ptr<Want> want = dwant->ToWant();
    EXPECT_STREQ(want->GetType().c_str(), dwant->GetType().c_str());
    EXPECT_STREQ(want->GetElement().GetDeviceID().c_str(), dwant->GetElement().GetDeviceID().c_str());
    EXPECT_STREQ(want->GetElement().GetBundleName().c_str(), dwant->GetElement().GetBundleName().c_str());
    EXPECT_STREQ(want->GetElement().GetAbilityName().c_str(), dwant->GetElement().GetAbilityName().c_str());
    EXPECT_EQ(want->GetFlags(), dwant->GetFlags());
    EXPECT_STREQ(want->GetUriString().c_str(), dwant->GetUriString().c_str());
    EXPECT_STREQ(want->GetAction().c_str(), dwant->GetAction().c_str());
    EXPECT_EQ(want->GetEntities().size(), dwant->GetEntities().size());
    EXPECT_EQ(want->GetBoolParam(boolType, false), dwant->GetBoolParam(boolType, true));
    EXPECT_EQ(want->GetByteParam(byteType, 0), dwant->GetByteParam(byteType, 0));
    EXPECT_EQ(want->GetCharParam(charType, 0), dwant->GetCharParam(charType, 0));
    EXPECT_EQ(want->GetShortParam(shortType, 0), dwant->GetShortParam(shortType, 0));
    EXPECT_EQ(want->GetIntParam(intType, 0), dwant->GetIntParam(intType, 0));
    EXPECT_EQ(want->GetLongParam(longType, 0), dwant->GetLongParam(longType, 0));
    EXPECT_EQ(want->GetFloatParam(floatType, 0), dwant->GetFloatParam(floatType, 0));
    EXPECT_EQ(want->GetDoubleParam(doubleType, 0), dwant->GetDoubleParam(doubleType, 0));
    EXPECT_STREQ(want->GetStringParam(stringType).c_str(), dwant->GetStringParam(stringType).c_str());
    EXPECT_EQ(want->GetDoubleArrayParam(doubleArrayType).size(), dwant->GetDoubleArrayParam(doubleArrayType).size());
    EXPECT_EQ(want->GetStringArrayParam(stringArrayType).size(), dwant->GetStringArrayParam(stringArrayType).size());
}

/**
 * @tc.number: DistributedSchedule_DistributedWantV2_Convert_0201
 * @tc.name: ToWant
 * @tc.desc: Verifying successful conversion of distributedwantV2 to want.
 */
HWTEST_F(DistributedWantV2BaseTest, DistributedSchedule_Distributedwant_Convert_0201, Function | MediumTest | Level3)
{
    std::shared_ptr<DistributedWantV2> dwant = std::make_shared<DistributedWantV2>();
    ASSERT_NE(dwant, nullptr);
    std::vector<bool> bv;
    bv.emplace_back(true);
    bv.emplace_back(false);
    dwant->SetParam(boolArrayType, bv);
    std::vector<byte> byv;
    byv.emplace_back(2);
    byv.emplace_back(3);
    dwant->SetParam(byteArrayType, byv);
    std::vector<zchar> chv;
    chv.emplace_back('\n');
    chv.emplace_back('p');
    chv.emplace_back('i');
    dwant->SetParam(charArrayType, chv);
    std::vector<short> shv;
    shv.emplace_back(111);
    shv.emplace_back(222);
    shv.emplace_back(333);
    dwant->SetParam(shortArrayType, shv);
    std::vector<int> inv;
    inv.emplace_back(1111);
    inv.emplace_back(2222);
    inv.emplace_back(3333);
    inv.emplace_back(4444);
    dwant->SetParam(intArrayType, inv);
    std::vector<long> lgv;
    lgv.emplace_back(1111);
    lgv.emplace_back(2222);
    lgv.emplace_back(3333);
    lgv.emplace_back(4444);
    dwant->SetParam(longArrayType, lgv);
    std::vector<float> ftv;
    ftv.emplace_back(1111.1);
    ftv.emplace_back(2222.2);
    ftv.emplace_back(3333.3);
    ftv.emplace_back(4444.4);
    dwant->SetParam(floatArrayType, ftv);
    std::shared_ptr<Want> want = dwant->ToWant();
    EXPECT_EQ(want->GetBoolArrayParam(boolArrayType).size(), dwant->GetBoolArrayParam(boolArrayType).size());
    EXPECT_EQ(want->GetByteArrayParam(byteArrayType).size(), dwant->GetByteArrayParam(byteArrayType).size());
    EXPECT_EQ(want->GetCharArrayParam(charArrayType).size(), dwant->GetCharArrayParam(charArrayType).size());
    EXPECT_EQ(want->GetShortArrayParam(shortArrayType).size(), dwant->GetShortArrayParam(shortArrayType).size());
    EXPECT_EQ(want->GetIntArrayParam(intArrayType).size(), dwant->GetIntArrayParam(intArrayType).size());
    EXPECT_EQ(want->GetLongArrayParam(longArrayType).size(), dwant->GetLongArrayParam(longArrayType).size());
    EXPECT_EQ(want->GetFloatArrayParam(floatArrayType).size(), dwant->GetFloatArrayParam(floatArrayType).size());
}

/**
 * @tc.number: DistributedScheduleWant_Type_0100
 * @tc.name: SetType/GetType
 * @tc.desc: Validate when normally entering a string
 */
HWTEST_F(DistributedWantV2BaseTest, DistributedScheduleWant_Type_0100, Function | MediumTest | Level3)
{
    if (want_ != nullptr) {
        std::string description = "liuuy";
        want_->SetType(description);
        EXPECT_STREQ(description.c_str(), want_->GetType().c_str());
    }
}

/**
 * @tc.number: DistributedScheduleWant_Action_0100
 * @tc.name: SetAction/GetAction
 * @tc.desc: Validate when normally entering a string
 */
HWTEST_F(DistributedWantV2BaseTest, DistributedScheduleWant_Action_0100, Function | MediumTest | Level3)
{
    if (want_ != nullptr) {
        std::string actiondescription = "liuuy";
        want_->SetAction(actiondescription);
        EXPECT_STREQ(actiondescription.c_str(), want_->GetAction().c_str());
    }
}

/**
 * @tc.number: DistributedScheduleWant_Bundle_0100
 * @tc.name: SetBundle/GetBundle
 * @tc.desc: Validate when normally entering a string
 */
HWTEST_F(DistributedWantV2BaseTest, DistributedScheduleWant_Bundle_0100, Function | MediumTest | Level3)
{
    if (want_ != nullptr) {
        std::string bundleName = "liuuy";
        want_->SetBundle(bundleName);
        EXPECT_STREQ(bundleName.c_str(), want_->GetBundle().c_str());
    }
}

/**
 * @tc.number: DistributedScheduleWant_Parcelable_0100
 * @tc.name: Marshalling/Unmarshalling
 * @tc.desc: marshalling Want, and then check result.
 */
HWTEST_F(DistributedWantV2BaseTest, DistributedScheduleWant_Parcelable_0100, Function | MediumTest | Level3)
{
    std::shared_ptr<DistributedWantV2> WantIn_ = std::make_shared<DistributedWantV2>();
    ASSERT_NE(nullptr, WantIn_);

    WantIn_->SetAction("12345");
    WantIn_->SetFlags(123);
    WantIn_->AddEntity("12345");
    AAFwk::WantParams wantParams;
    std::string keyStr = "12345667";
    bool valueBool = true;
    wantParams.SetParam(keyStr, Boolean::Box(valueBool));
    WantIn_->SetParams(wantParams);
    OHOS::AppExecFwk::ElementName element;
    element.SetAbilityName("12345");
    element.SetBundleName("12345");
    element.SetDeviceID("12345");
    WantIn_->SetElement(element);
    WantIn_->SetType("12345");

    Parcel in;
    size_t pos1 = in.GetWritePosition();
    bool result = WantIn_->Marshalling(in);
    size_t pos2 = in.GetWritePosition();
    EXPECT_EQ(result, true);
    GTEST_LOG_(INFO) << " Marshalling: pos1: " << pos1 << ", pos2: " << pos2 << ", result: " << result;

    pos1 = in.GetWritePosition();
    result = WantIn_->Marshalling(in);
    pos2 = in.GetWritePosition();
    EXPECT_EQ(result, true);
    GTEST_LOG_(INFO) << " Marshalling: pos1: " << pos1 << ", pos2: " << pos2 << ", result: " << result;

    pos1 = in.GetReadPosition();
    std::shared_ptr<DistributedWantV2> WantOut_(DistributedWantV2::Unmarshalling(in));
    if (WantOut_ != nullptr) {
        pos2 = in.GetReadPosition();
        CompareWant(WantIn_, WantOut_);
        EXPECT_EQ(valueBool, Boolean::Unbox(IBoolean::Query(WantOut_->GetParams().GetParam(keyStr))));
        GTEST_LOG_(INFO) << " Unmarshalling: pos1: " << pos1 << ", pos2: " << pos2 << ", result: " << result;
    }

    pos1 = in.GetReadPosition();
    std::shared_ptr<DistributedWantV2> WantOut2_(DistributedWantV2::Unmarshalling(in));
    if (WantOut2_ != nullptr) {
        pos2 = in.GetReadPosition();
        CompareWant(WantIn_, WantOut2_);
        EXPECT_EQ(valueBool, Boolean::Unbox(IBoolean::Query(WantOut2_->GetParams().GetParam(keyStr))));
        GTEST_LOG_(INFO) << " Unmarshalling: pos1: " << pos1 << ", pos2: " << pos2 << ", result: " << result;
    }
}
/**
 * @tc.number: DistributedScheduleWant_Parcelable_0200
 * @tc.name: Marshalling/Unmarshalling
 * @tc.desc: marshalling Want, and then check result.
 */
HWTEST_F(DistributedWantV2BaseTest, DistributedScheduleWant_Parcelable_0200, Function | MediumTest | Level3)
{
    std::shared_ptr<DistributedWantV2> WantIn_ = std::make_shared<DistributedWantV2>();
    ASSERT_NE(nullptr, WantIn_);
    WantIn_->SetAction("@#￥#3243adsafdf_中文");
    WantIn_->SetFlags(123);
    WantIn_->AddEntity("@#￥#3243adsafdf_中文");
    AAFwk::WantParams wantParams;
    std::string keyStr = "@#￥#3243adsafdf_中文";
    std::string valueStr = "123";
    wantParams.SetParam(keyStr, String::Box(valueStr));
    WantIn_->SetParams(wantParams);
    OHOS::AppExecFwk::ElementName element;
    element.SetAbilityName("@#￥#3243adsafdf_中文");
    element.SetBundleName("@#￥#3243adsafdf_中文");
    element.SetDeviceID("@#￥#3243adsafdf_中文");
    WantIn_->SetElement(element);
    WantIn_->SetType("@#￥#3243adsafdf_中文");
    size_t pos1;
    size_t pos2;
    bool result = false;
    Parcel in;
    pos1 = in.GetWritePosition();
    result = WantIn_->Marshalling(in);
    pos2 = in.GetWritePosition();
    EXPECT_EQ(result, true);
    GTEST_LOG_(INFO) << "Marshalling: pos1: " << pos1 << ", pos2: " << pos2 << ", result: " << result;
    pos1 = in.GetWritePosition();
    result = WantIn_->Marshalling(in);
    pos2 = in.GetWritePosition();
    EXPECT_EQ(result, true);
    pos1 = in.GetReadPosition();
    std::shared_ptr<DistributedWantV2> WantOut_(DistributedWantV2::Unmarshalling(in));
    if (WantOut_ != nullptr) {
        pos2 = in.GetReadPosition();
        CompareWant(WantIn_, WantOut_);
        std::string outString(String::Unbox(IString::Query(WantOut_->GetParams().GetParam(keyStr))));
        EXPECT_STREQ(valueStr.c_str(), outString.c_str());
        GTEST_LOG_(INFO) << "Unmarshalling: pos1: " << pos1 << ", pos2: " << pos2 << ", result: " << result;
    }
    pos1 = in.GetReadPosition();
    std::shared_ptr<DistributedWantV2> WantOut2_(DistributedWantV2::Unmarshalling(in));
    if (WantOut2_ != nullptr) {
        pos2 = in.GetReadPosition();
        CompareWant(WantIn_, WantOut2_);
        std::string outString2(String::Unbox(IString::Query(WantOut2_->GetParams().GetParam(keyStr))));
        EXPECT_STREQ(valueStr.c_str(), outString2.c_str());
    }
}

/**
 * @tc.number: DistributedScheduleWant_Parcelable_0300
 * @tc.name: Marshalling/Unmarshalling
 * @tc.desc: marshalling Want, and then check result.
 */
HWTEST_F(DistributedWantV2BaseTest, DistributedScheduleWant_Parcelable_0300, Function | MediumTest | Level3)
{
    std::shared_ptr<DistributedWantV2> WantIn_ = std::make_shared<DistributedWantV2>();
    ASSERT_NE(nullptr, WantIn_);

    WantIn_->SetAction("");
    WantIn_->SetFlags(123);
    WantIn_->AddEntity("");
    AAFwk::WantParams wantParams;
    std::string keyStr = "";
    int valueInt = 123;
    wantParams.SetParam(keyStr, Integer::Box(valueInt));
    WantIn_->SetParams(wantParams);
    OHOS::AppExecFwk::ElementName element;
    element.SetAbilityName("");
    element.SetBundleName("");
    element.SetDeviceID("");
    WantIn_->SetElement(element);
    WantIn_->SetType("");

    size_t pos1;
    size_t pos2;
    bool result = false;

    Parcel in;
    pos1 = in.GetWritePosition();
    result = WantIn_->Marshalling(in);
    pos2 = in.GetWritePosition();
    EXPECT_EQ(result, true);
    GTEST_LOG_(INFO) << "Marshalling: pos1: " << pos1 << ", pos2: " << pos2 << ", result: " << result;

    pos1 = in.GetWritePosition();
    result = WantIn_->Marshalling(in);
    pos2 = in.GetWritePosition();
    EXPECT_EQ(result, true);
    GTEST_LOG_(INFO) << "Marshalling: pos1: " << pos1 << ", pos2: " << pos2 << ", result: " << result;

    pos1 = in.GetReadPosition();
    std::shared_ptr<DistributedWantV2> WantOut_(DistributedWantV2::Unmarshalling(in));
    if (WantOut_ != nullptr) {
        pos2 = in.GetReadPosition();
        CompareWant(WantIn_, WantOut_);
        EXPECT_EQ(valueInt, Integer::Unbox(IInteger::Query(WantOut_->GetParams().GetParam(keyStr))));
        GTEST_LOG_(INFO) << "Unmarshalling: pos1: " << pos1 << ", pos2: " << pos2 << ", result: " << result;
    }

    pos1 = in.GetReadPosition();
    std::shared_ptr<DistributedWantV2> WantOut2_(DistributedWantV2::Unmarshalling(in));
    if (WantOut2_ != nullptr) {
        pos2 = in.GetReadPosition();
        CompareWant(WantIn_, WantOut2_);
        EXPECT_EQ(valueInt, Integer::Unbox(IInteger::Query(WantOut2_->GetParams().GetParam(keyStr))));
        GTEST_LOG_(INFO) << "Unmarshalling: pos1: " << pos1 << ", pos2: " << pos2 << ", result: " << result;
    }
}

/**
 * @tc.number: DistributedScheduleWant_Parcelable_0400
 * @tc.name: Marshalling/Unmarshalling
 * @tc.desc: marshalling Want, and then check result.
 */
HWTEST_F(DistributedWantV2BaseTest, DistributedScheduleWant_Parcelable_0400, Function | MediumTest | Level3)
{
    std::shared_ptr<DistributedWantV2> WantIn_ = std::make_shared<DistributedWantV2>();
    ASSERT_NE(nullptr, WantIn_);

    WantIn_->SetAction("12345");
    WantIn_->SetFlags(123);
    WantIn_->AddEntity("12345");
    WantIn_->AddEntity("@#￥#3243adsafdf_中文");
    WantIn_->AddEntity("");
    AAFwk::WantParams wantParams;
    std::string keyStr = "12345667";
    std::string valueString = "123";
    wantParams.SetParam(keyStr, String::Box(valueString));
    WantIn_->SetParams(wantParams);
    OHOS::AppExecFwk::ElementName element;
    element.SetAbilityName("12345");
    element.SetBundleName("12345");
    element.SetDeviceID("12345");
    WantIn_->SetElement(element);
    WantIn_->SetType("12345");

    Parcel in;
    size_t pos1 = in.GetWritePosition();
    bool result = WantIn_->Marshalling(in);
    size_t pos2 = in.GetWritePosition();
    EXPECT_EQ(result, true);
    GTEST_LOG_(INFO) << "Marshalling: pos1: " << pos1 << ", pos2: " << pos2 << ", result: " << result;

    pos1 = in.GetWritePosition();
    result = WantIn_->Marshalling(in);
    pos2 = in.GetWritePosition();
    EXPECT_EQ(result, true);
    GTEST_LOG_(INFO) << "Marshalling: pos1: " << pos1 << ", pos2: " << pos2 << ", result: " << result;

    pos1 = in.GetReadPosition();
    std::shared_ptr<DistributedWantV2> WantOut_(DistributedWantV2::Unmarshalling(in));
    if (WantOut_ != nullptr) {
        pos2 = in.GetReadPosition();
        CompareWant(WantIn_, WantOut_);
        EXPECT_EQ(valueString, String::Unbox(IString::Query(WantOut_->GetParams().GetParam(keyStr))));
        GTEST_LOG_(INFO) << "Unmarshalling: pos1: " << pos1 << ", pos2: " << pos2 << ", result: " << result;
    }

    pos1 = in.GetReadPosition();
    std::shared_ptr<DistributedWantV2> WantOut2_(DistributedWantV2::Unmarshalling(in));
    if (WantOut2_ != nullptr) {
        pos2 = in.GetReadPosition();
        CompareWant(WantIn_, WantOut2_);
        EXPECT_EQ(valueString, String::Unbox(IString::Query(WantOut2_->GetParams().GetParam(keyStr))));
        GTEST_LOG_(INFO) << "Unmarshalling: pos1: " << pos1 << ", pos2: " << pos2 << ", result: " << result;
    }
}

/**
 * @tc.number: DistributedScheduleWant_Parcelable_0500
 * @tc.name: Marshalling/Unmarshalling
 * @tc.desc: marshalling Want, and then check result.
 */
HWTEST_F(DistributedWantV2BaseTest, DistributedScheduleWant_Parcelable_0500, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "DistributedScheduleWant_Parcelable_0500 start";
    std::shared_ptr<DistributedWantV2> WantIn_ = std::make_shared<DistributedWantV2>();
    ASSERT_NE(nullptr, WantIn_);

    WantIn_->AddEntity("system.test.entity");

    OHOS::AppExecFwk::ElementName element;
    element.SetAbilityName("system.test.abilityname");
    element.SetBundleName("system.test.bundlename");
    element.SetDeviceID("system.test.deviceid");
    WantIn_->SetElement(element);

    AAFwk::WantParams wantParams;
    std::string keyStr = "system.test.wantparams.key";
    std::string MIMEKEY = "mime-type";
    wantParams.SetParam(MIMEKEY, String::Box("system.test.uritype"));

    std::string valueString = "system.wantparams.value.content.test";
    wantParams.SetParam(keyStr, String::Box(valueString));
    WantIn_->SetParams(wantParams);

    Parcel in;
    bool result = false;
    result = WantIn_->Marshalling(in);
    EXPECT_EQ(result, true);
    std::shared_ptr<DistributedWantV2> WantOut_(DistributedWantV2::Unmarshalling(in));
    if (WantOut_ != nullptr) {

        bool hasentity = WantOut_->HasEntity("system.test.entity");
        GTEST_LOG_(INFO) << "WantOut_->HasEntity(system.test.entity)" << hasentity;
        EXPECT_EQ(hasentity, true);

        WantOut_->RemoveEntity(std::string("system.test.entity"));
        hasentity = WantOut_->HasEntity(std::string("system.test.entity"));
        GTEST_LOG_(INFO) << "WantOut_->RemoveEntity(system.test.entity)" << hasentity;
        EXPECT_EQ(hasentity, false);

        std::string outtype = WantOut_->GetType();
        GTEST_LOG_(INFO) << "WantOut_->GetType()" << outtype.c_str();
        EXPECT_STREQ(outtype.c_str(), std::string("system.test.uritype").c_str());

        element = WantOut_->GetElement();
        GTEST_LOG_(INFO) << "element.GetAbilityName().c_str(): " << element.GetAbilityName().c_str();
        EXPECT_STREQ(element.GetAbilityName().c_str(), std::string("system.test.abilityname").c_str());

        GTEST_LOG_(INFO) << "element->GetBundleName().c_str(): " << element.GetBundleName().c_str();
        EXPECT_STREQ(element.GetBundleName().c_str(), std::string("system.test.bundlename").c_str());

        EXPECT_STREQ(element.GetDeviceID().c_str(), std::string("system.test.deviceid").c_str());

        std::string param_content = WantOut_->GetStringParam(keyStr);
        GTEST_LOG_(INFO) << "WantOut_->GetStringParam(keyStr): " << param_content.c_str();

        GTEST_LOG_(INFO) << "DistributedScheduleWant_Parcelable_005 end";
    }
}

/**
 * @tc.number: DistributedScheduleWant_Parcelable_0501
 * @tc.name: Marshalling/Unmarshalling
 * @tc.desc: marshalling Want, and then check result.
 */
HWTEST_F(DistributedWantV2BaseTest, DistributedScheduleWant_Parcelable_0501, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "DistributedScheduleWant_Parcelable_0501 start";
    std::shared_ptr<DistributedWantV2> WantIn_ = std::make_shared<DistributedWantV2>();
    ASSERT_NE(nullptr, WantIn_);

    WantIn_->SetAction("system.test.action");

    std::vector<bool> boolArrayValue = {true, false, true};
    WantIn_->SetParam(std::string("bool_arraykey"), boolArrayValue);

    std::vector<byte> byteArrayValue = {'?', 'a', '\\'};
    WantIn_->SetParam(std::string("byte_arraykey"), byteArrayValue);

    std::vector<zchar> charArrayValue = {U'e', U'l', U'l', U'o'};
    WantIn_->SetParam(std::string("char_arraykey"), charArrayValue);

    std::vector<std::string> stringArrayValue = {"stringtest1", "string@test2", "string@!#test2"};
    WantIn_->SetParam(std::string("string_arraykey"), stringArrayValue);

    DistributedWantV2 wantCopy(*WantIn_);
    std::vector<std::string> teststringArrayValue1 = WantIn_->GetStringArrayParam(std::string("string_arraykey"));
    std::vector<std::string> teststringArrayValue2 = wantCopy.GetStringArrayParam(std::string("string_arraykey"));
    bool copyarraycompare = CompareArrayData<std::string>(teststringArrayValue1, teststringArrayValue1);
    EXPECT_EQ(copyarraycompare, true);
    std::string str = (copyarraycompare == true) ? "true" : "false";
    GTEST_LOG_(INFO) << "copyarraycompare=" << str.c_str();

    Parcel in;
    bool result = false;
    result = WantIn_->Marshalling(in);
    EXPECT_EQ(result, true);
    std::shared_ptr<DistributedWantV2> WantOut_(DistributedWantV2::Unmarshalling(in));
    if (WantOut_ != nullptr) {
        GTEST_LOG_(INFO) << "WantOut_->GetAction().c_str(): " << WantOut_->GetAction().c_str();
        EXPECT_STREQ(WantOut_->GetAction().c_str(), std::string("system.test.action").c_str());

        std::vector<bool> retboolArray;
        retboolArray = WantOut_->GetBoolArrayParam(std::string("bool_arraykey"));

        bool arraycompare = CompareArrayData<bool>(retboolArray, boolArrayValue);
        EXPECT_EQ(arraycompare, true);

        std::vector<byte> retbyteArrayValue;
        retbyteArrayValue = WantOut_->GetByteArrayParam(std::string("byte_arraykey"));
        arraycompare = CompareArrayData<byte>(retbyteArrayValue, byteArrayValue);
        EXPECT_EQ(arraycompare, true);

        std::vector<zchar> retcharArrayValue;
        retcharArrayValue = WantOut_->GetCharArrayParam(std::string("char_arraykey"));
        arraycompare = CompareArrayData<zchar>(retcharArrayValue, charArrayValue);
        EXPECT_EQ(arraycompare, true);

        std::vector<std::string> retstringArrayValue;
        retstringArrayValue = WantOut_->GetStringArrayParam(std::string("string_arraykey"));
        arraycompare = CompareArrayData<std::string>(retstringArrayValue, stringArrayValue);
        EXPECT_EQ(arraycompare, true);
    }
}

/**
 * @tc.number: DistributedScheduleWant_Parcelable_0502
 * @tc.name: Marshalling/Unmarshalling
 * @tc.desc: marshalling Want, and then check result.
 */
HWTEST_F(DistributedWantV2BaseTest, DistributedScheduleWant_Parcelable_0502, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "DistributedScheduleWant_Parcelable_0502 start";
    std::shared_ptr<DistributedWantV2> WantIn_ = std::make_shared<DistributedWantV2>();
    ASSERT_NE(nullptr, WantIn_);

    WantIn_->SetFlags(64);

    std::vector<short> shortArrayValue = {-1, 0, 1};
    WantIn_->SetParam(std::string("short_arraykey"), shortArrayValue);

    std::vector<int> intArrayValue = {-10, 0, 10};
    WantIn_->SetParam(std::string("int_arraykey"), intArrayValue);

    std::vector<long> longArrayValue = {-100, 0, 100};
    WantIn_->SetParam(std::string("long_arraykey"), longArrayValue);

    std::vector<float> floatArrayValue = {-100.1, 0.1, 100.1};
    WantIn_->SetParam(std::string("float_arraykey"), floatArrayValue);

    std::vector<double> doubleArrayValue = {-1000.1, 0.1, 1000.1};
    WantIn_->SetParam(std::string("double_arraykey"), doubleArrayValue);

    Parcel in;
    bool result = false;
    result = WantIn_->Marshalling(in);
    EXPECT_EQ(result, true);
    std::shared_ptr<DistributedWantV2> WantOut_(DistributedWantV2::Unmarshalling(in));
    if (WantOut_ != nullptr) {
        int flags = WantOut_->GetFlags();
        GTEST_LOG_(INFO) << "WantOut_->GetFlags(): " << flags;
        EXPECT_EQ(static_cast<int>(flags), 64);

        std::vector<short> retshortArrayValue;
        retshortArrayValue = WantOut_->GetShortArrayParam(std::string("short_arraykey"));
        bool arraycompare = CompareArrayData<short>(retshortArrayValue, shortArrayValue);
        EXPECT_EQ(arraycompare, true);

        std::vector<int> retintArrayValue;
        retintArrayValue = WantOut_->GetIntArrayParam(std::string("int_arraykey"));
        arraycompare = CompareArrayData<int>(retintArrayValue, intArrayValue);
        EXPECT_EQ(arraycompare, true);

        std::vector<long> retlonArrayValue;
        retlonArrayValue = WantOut_->GetLongArrayParam(std::string("long_arraykey"));
        arraycompare = CompareArrayData<long>(retlonArrayValue, longArrayValue);
        EXPECT_EQ(arraycompare, true);

        std::vector<float> retfloatArrayValue;
        retfloatArrayValue = WantOut_->GetFloatArrayParam(std::string("float_arraykey"));
        arraycompare = CompareArrayData<float>(retfloatArrayValue, floatArrayValue);
        EXPECT_EQ(arraycompare, true);

        std::vector<double> retdoubleArrayValue;
        retdoubleArrayValue = WantOut_->GetDoubleArrayParam(std::string("double_arraykey"));
        arraycompare = CompareArrayData<double>(retdoubleArrayValue, doubleArrayValue);
        EXPECT_EQ(arraycompare, true);
    }
}

void DistributedWantV2BaseTest::CompareWant(const std::shared_ptr<DistributedWantV2>& want1,
    const std::shared_ptr<DistributedWantV2>& want2) const
{
    DistributedOperation opt1 = want1->GetOperation();
    DistributedOperation opt2 = want2->GetOperation();
    EXPECT_EQ(opt1.GetDeviceId(), opt2.GetDeviceId());
    EXPECT_EQ(opt1.GetBundleName(), opt2.GetBundleName());
    EXPECT_EQ(opt1.GetAbilityName(), opt2.GetAbilityName());

    EXPECT_EQ(want1->GetAction(), want2->GetAction());
    EXPECT_EQ(want1->GetFlags(), want2->GetFlags());
    EXPECT_EQ(want1->GetType(), want2->GetType());
    EXPECT_EQ(want1->CountEntities(), want2->CountEntities());

    int count = want1->CountEntities();
    std::vector<std::string> entities1 = want1->GetEntities();
    std::vector<std::string> entities2 = want2->GetEntities();
    for (int i = 0; i < count; i++) {
        EXPECT_EQ(entities1.at(i), entities2.at(i));
    }

    OHOS::AppExecFwk::ElementName element1 = want1->GetElement();
    OHOS::AppExecFwk::ElementName element2 = want2->GetElement();
    EXPECT_EQ(element1.GetURI(), element1.GetURI());

    std::set<std::string> key1;
    std::set<std::string> key2;
    key1 = want1->GetParams().KeySet();
    key2 = want2->GetParams().KeySet();
    EXPECT_EQ(key1.size(), key2.size());

    std::set<std::string>::iterator iter1 = key1.begin();
    std::set<std::string>::iterator iter2 = key2.begin();
    for (; (iter1 != key1.end() && iter2 != key2.end()); iter1++, iter2++) {
        EXPECT_EQ(*iter1, *iter2);
    }
}

bool DistributedWantV2BaseTest::CompareWant(const std::shared_ptr<DistributedWantV2>& want1,
    const std::shared_ptr<DistributedWantV2>& want2, std::map<std::string, std::string>& keys) const
{
    if (want1 == nullptr || want2 == nullptr) {
        return false;
    }
    EXPECT_STREQ(want1->GetAction().c_str(), want2->GetAction().c_str());
    EXPECT_EQ(want1->CountEntities(), want2->CountEntities());

    if (want1->CountEntities() != want2->CountEntities()) {
        return false;
    }

    int count = want1->CountEntities();

    std::vector<std::string> entities1 = want1->GetEntities();
    std::vector<std::string> entities2 = want2->GetEntities();
    for (int i = 0; i < count; i++) {
        EXPECT_EQ(entities1.at(i), entities2.at(i));
    }
    EXPECT_EQ(want1->GetFlags(), want2->GetFlags());
    EXPECT_EQ(want1->GetElement(), want2->GetElement());

    for (auto it = keys.begin(); it != keys.end(); it++) {
        if (!CompareWantNumber(want1, want2, it) && !CompareWantOthers(want1, want2, it) &&
            !CompareWantArray(want1, want2, it)) {};
    }

    return true;
}

bool DistributedWantV2BaseTest::CompareWantNumber(const std::shared_ptr<DistributedWantV2>& want1,
    const std::shared_ptr<DistributedWantV2>& want2, std::map<std::string, std::string>::iterator it) const
{
    if (it->second == shortType) {
        short default1 = 123;
        short default2 = 456;
        short v1 = want1->GetShortParam(it->first, default1);
        short v2 = want2->GetShortParam(it->first, default2);
        EXPECT_EQ(v1, v2);
        return true;
    } else if (it->second == intType) {
        int default1 = 1230000;
        int default2 = 4560000;
        int v1 = want1->GetIntParam(it->first, default1);
        int v2 = want2->GetIntParam(it->first, default2);
        EXPECT_EQ(v1, v2);
        return true;
    } else if (it->second == longType) {
        long default1 = 1e8;
        long default2 = 2e8;
        long v1 = want1->GetLongParam(it->first, default1);
        long v2 = want2->GetLongParam(it->first, default2);
        EXPECT_EQ(v1, v2);
        return true;
    } else if (it->second == floatType) {
        float default1 = 12.3;
        float default2 = 45.6;
        float v1 = want1->GetFloatParam(it->first, default1);
        float v2 = want2->GetFloatParam(it->first, default2);
        EXPECT_EQ(v1, v2);
        return true;
    } else if (it->second == doubleType) {
        double default1 = 12.3;
        double default2 = 45.6;
        double v1 = want1->GetDoubleParam(it->first, default1);
        double v2 = want2->GetDoubleParam(it->first, default2);
        EXPECT_EQ(v1, v2);
        return true;
    }
    return false;
}

bool DistributedWantV2BaseTest::CompareWantOthers(const std::shared_ptr<DistributedWantV2>& want1,
    const std::shared_ptr<DistributedWantV2>& want2, std::map<std::string, std::string>::iterator it) const
{
    if (it->second == boolType) {
        bool v1 = want1->GetBoolParam(it->first, false);
        bool v2 = want2->GetBoolParam(it->first, false);
        EXPECT_EQ(v1, v2);
        EXPECT_EQ(v1, true);
        return true;
    } else if (it->second == byteType) {
        byte v1 = want1->GetByteParam(it->first, 'j');
        byte v2 = want2->GetByteParam(it->first, 'k');
        EXPECT_EQ(v1, v2);
        return true;
    } else if (it->second == charType) {
        zchar v1 = want1->GetCharParam(it->first, 0x01AB);
        zchar v2 = want2->GetCharParam(it->first, 0x02CD);
        EXPECT_EQ(v1, v2);
        return true;
    } else if (it->second == stringType) {
        std::string v1 = want1->GetStringParam(it->first);
        std::string v2 = want2->GetStringParam(it->first);
        EXPECT_EQ(v1, v2);
        return true;
    }
    return false;
}

bool DistributedWantV2BaseTest::CompareWantArray(const std::shared_ptr<DistributedWantV2>& want1,
    const std::shared_ptr<DistributedWantV2>& want2, std::map<std::string, std::string>::iterator it) const
{
    if (it->second == boolArrayType) {
        std::vector<bool> v1 = want1->GetBoolArrayParam(it->first);
        std::vector<bool> v2 = want2->GetBoolArrayParam(it->first);
        EXPECT_EQ(v1, v2);
        return true;
    } else if (it->second == byteArrayType) {
        std::vector<byte> v1 = want1->GetByteArrayParam(it->first);
        std::vector<byte> v2 = want2->GetByteArrayParam(it->first);
        EXPECT_EQ(v1, v2);
        return true;
    } else if (it->second == charArrayType) {
        std::vector<zchar> v1 = want1->GetCharArrayParam(it->first);
        std::vector<zchar> v2 = want2->GetCharArrayParam(it->first);
        EXPECT_EQ(v1, v2);
        return true;
    } else if (it->second == shortArrayType) {
        std::vector<short> v1 = want1->GetShortArrayParam(it->first);
        std::vector<short> v2 = want2->GetShortArrayParam(it->first);
        EXPECT_EQ(v1, v2);
    } else if (it->second == intArrayType) {
        std::vector<int> v1 = want1->GetIntArrayParam(it->first);
        std::vector<int> v2 = want2->GetIntArrayParam(it->first);
        EXPECT_EQ(v1, v2);
        return true;
    } else if (it->second == longArrayType) {
        std::vector<long> v1 = want1->GetLongArrayParam(it->first);
        std::vector<long> v2 = want2->GetLongArrayParam(it->first);
        EXPECT_EQ(v1, v2);
        return true;
    } else if (it->second == floatArrayType) {
        std::vector<float> v1 = want1->GetFloatArrayParam(it->first);
        std::vector<float> v2 = want2->GetFloatArrayParam(it->first);
        EXPECT_EQ(v1, v2);
        return true;
    } else if (it->second == doubleArrayType) {
        std::vector<double> v1 = want1->GetDoubleArrayParam(it->first);
        std::vector<double> v2 = want2->GetDoubleArrayParam(it->first);
        EXPECT_EQ(v1, v2);
        return true;
    }  else if (it->second == stringArrayType) {
        std::vector<std::string> v1 = want1->GetStringArrayParam(it->first);
        std::vector<std::string> v2 = want2->GetStringArrayParam(it->first);
        EXPECT_EQ(v1, v2);
        return true;
    }
    return false;
}

void DistributedWantV2BaseTest::SendParcelTest(const std::shared_ptr<DistributedWantV2> &want,
    std::map<std::string, std::string> &keys) const
{
    size_t pos1;
    size_t pos2;
    Parcel data;
    bool result = false;

    pos1 = data.GetWritePosition();
    result = data.WriteParcelable(want.get());
    pos2 = data.GetWritePosition();
    EXPECT_EQ(result, true);
    GTEST_LOG_(INFO) << "SendParcelTest: pos1: " << pos1 << ", pos2: " << pos2 << ", result: " << result;

    std::shared_ptr<DistributedWantV2> wantNew(data.ReadParcelable<DistributedWantV2>());
    EXPECT_NE(wantNew, nullptr);

    if (wantNew != nullptr) {
        result = CompareWant(want, wantNew, keys);
        EXPECT_EQ(result, true);
    }
}

void DistributedWantV2BaseTest::AddBoolParams(DistributedWantV2& want,
    std::map<std::string, std::string>& keys, int loop, unsigned int flag) const
{
    std::string key;
    std::string boolKey = "boolKey";
    std::string boolArrayKey = "boolArrayKey";
    for (int i = 0; i < loop; i++) {
        if (flag & FLAG_TEST_SINGLE) {
            bool boolValue = true;
            keys[boolKey + std::to_string(i)] = boolType;
            want.SetParam(boolKey + std::to_string(i), boolValue);
        }

        if (flag & FLAG_TEST_ARRAY) {
            std::vector<bool> boolArrayValue = {true, false, true};
            keys[key] = boolArrayType;
            want.SetParam(key, boolArrayValue);
        }
    }
}

void DistributedWantV2BaseTest::AddByteParams(DistributedWantV2& want,
    std::map<std::string, std::string>& keys, int loop, unsigned int flag) const
{
    std::string key;
    std::string byteKey = "byteKey";
    std::string byteArrayKey = "byteArrayKey";
    for (int i = 0; i < loop; i++) {
        if (flag & FLAG_TEST_SINGLE) {
            byte byteValue = 'z';
            key = byteKey + std::to_string(i);
            keys[key] = byteType;
            want.SetParam(key, byteValue);
        }

        if (flag & FLAG_TEST_ARRAY) {
            std::vector<byte> byteArrayValue = {'?', 'a', '\\'};
            key = byteArrayKey + std::to_string(i);
            keys[key] = byteArrayType;
            want.SetParam(key, byteArrayValue);
        }
    }
}

void DistributedWantV2BaseTest::AddCharParams(DistributedWantV2& want,
    std::map<std::string, std::string>& keys, int loop, unsigned int flag) const
{
    std::string key;
    std::string charKey = "charKey";
    std::string charArrayKey = "charArrayKey";
    for (int i = 0; i < loop; i++) {
        if (flag & FLAG_TEST_SINGLE) {
            zchar charValue = U'h';
            key = charKey + std::to_string(i);
            keys[key] = charType;
            want.SetParam(key, charValue);
        }

        if (flag & FLAG_TEST_ARRAY) {
            std::vector<zchar> charArrayValue = {U'e', U'l', U'l', U'o'};
            key = charArrayKey + std::to_string(i);
            keys[key] = charArrayType;
            want.SetParam(key, charArrayValue);
        }
    }
}

void DistributedWantV2BaseTest::AddShortParams(DistributedWantV2& want,
    std::map<std::string, std::string>& keys, int loop, unsigned int flag) const
{
    std::string key;
    std::string shortKey = "shortKey";
    std::string shortArrayKey = "shortArrayKey";
    for (int i = 0; i < loop; i++) {
        if (flag & FLAG_TEST_SINGLE) {
            short shortValue = 1;
            key = shortKey + std::to_string(i);
            keys[key] = shortType;
            want.SetParam(key, shortValue);
        }

        if (flag & FLAG_TEST_ARRAY) {
            std::vector<short> shortArrayValue = {-1, 0, 1};
            key = shortArrayKey + std::to_string(i);
            keys[key] = shortArrayType;
            want.SetParam(key, shortArrayValue);
        }
    }
}

void DistributedWantV2BaseTest::AddIntParams(DistributedWantV2& want,
    std::map<std::string, std::string>& keys, int loop, unsigned int flag) const
{
    std::string key;
    std::string intKey = "intKey";
    std::string intArrayKey = "intArrayKey";
    for (int i = 0; i < loop; i++) {
        if (flag & FLAG_TEST_SINGLE) {
            int intValue = 10;
            key = intKey + std::to_string(i);
            keys[key] = intType;
            want.SetParam(key, intValue);
        }

        if (flag & FLAG_TEST_ARRAY) {
            std::vector<int> intArrayValue = {-10, 0, 10};
            key = intArrayKey + std::to_string(i);
            keys[key] = intArrayType;
            want.SetParam(key, intArrayValue);
        }
    }
}

void DistributedWantV2BaseTest::AddLongParams(DistributedWantV2& want,
    std::map<std::string, std::string>& keys, int loop, unsigned int flag) const
{
    std::string key;
    std::string longKey = "longKey";
    std::string longArrayKey = "longArrayKey";
    for (int i = 0; i < loop; i++) {
        if (flag & FLAG_TEST_SINGLE) {
            long longValue = 100L;
            key = longKey + std::to_string(i);
            keys[key] = longType;
            want.SetParam(key, longValue);
        }

        if (flag & FLAG_TEST_ARRAY) {
            std::vector<long> longArrayValue = {-100, 0, 100};
            key = longArrayKey + std::to_string(i);
            keys[key] = longArrayType;
            want.SetParam(key, longArrayValue);
        }
    }
}

void DistributedWantV2BaseTest::AddFloatParams(DistributedWantV2& want,
    std::map<std::string, std::string>& keys, int loop, unsigned int flag) const
{
    std::string key;
    std::string floatKey = "floatKey";
    std::string floatArrayKey = "floatArrayKey";
    for (int i = 0; i < loop; i++) {
        if (flag & FLAG_TEST_SINGLE) {
            float floatValue = 100.1f;
            key = floatKey + std::to_string(i);
            keys[key] = floatType;
            want.SetParam(key, floatValue);
        }

        if (flag & FLAG_TEST_ARRAY) {
            std::vector<float> floatArrayValue = {-100.1, 0.1, 100.1};
            key = floatArrayKey + std::to_string(i);
            keys[key] = floatArrayType;
            want.SetParam(key, floatArrayValue);
        }
    }
}

void DistributedWantV2BaseTest::AddDoubleParams(DistributedWantV2& want,
    std::map<std::string, std::string>& keys, int loop, unsigned int flag) const
{
    std::string key;
    std::string doubleKey = "doubleKey";
    std::string doubleArrayKey = "doubleArrayKey";
    for (int i = 0; i < loop; i++) {
        if (flag & FLAG_TEST_SINGLE) {
            double doubleValue = 1000.1;
            key = doubleKey + std::to_string(i);
            keys[key] = doubleType;
            want.SetParam(key, doubleValue);
        }

        if (flag & FLAG_TEST_ARRAY) {
            std::vector<double> doubleArrayValue = {-1000.1, 0.1, 1000.1};
            key = doubleArrayKey + std::to_string(i);
            keys[key] = doubleArrayType;
            want.SetParam(key, doubleArrayValue);
        }
    }
}

void DistributedWantV2BaseTest::AddStringParams(DistributedWantV2& want,
    std::map<std::string, std::string>& keys, int loop, unsigned int flag) const
{
    std::string key;
    std::string stringKey = "stringKey";
    std::string stringArrayKey = "stringArrayKey";
    for (int i = 0; i < loop; i++) {
        if (flag & FLAG_TEST_SINGLE) {
            std::string stringValue = "zzzz";
            key = stringKey + std::to_string(i);
            keys[key] = stringType;
            want.SetParam(key, stringValue);
        }

        if (flag & FLAG_TEST_ARRAY) {
            std::vector<std::string> stringArrayValue = {"??", "aa", "\\\\"};
            key = stringArrayKey + std::to_string(i);
            keys[key] = stringArrayType;
            want.SetParam(key, stringArrayValue);
        }
    }
}

void DistributedWantV2BaseTest::TestStringForParseUri(std::string uri,
    std::string keyString, std::size_t &length, std::string valueStringOrigin)
{
    std::size_t head = length;
    std::string search = String::SIGNATURE + std::string(".") + keyString + std::string("=");
    std::size_t result = uri.find(search);
    EXPECT_NE(result, std::string::npos);
    EXPECT_GE(result, head);
    length += search.length();
    if (result != std::string::npos) {
        std::size_t pos = result + search.length();
        std::size_t delims = uri.find(";", pos);
        if (delims != std::string::npos) {
            std::string substring = uri.substr(pos, delims - pos);
            std::string valueStringNew = String::Unbox(String::Parse(substring));
            EXPECT_EQ(valueStringNew, valueStringOrigin);
            length += substring.length() + 1;
        }
    }
}

void DistributedWantV2BaseTest::TestFloatForParseUri(std::string uri,
    std::string keyFloat, std::size_t &length, float valueFloatOrigin)
{
    std::size_t head = length;
    std::string search = Float::SIGNATURE + std::string(".") + keyFloat + std::string("=");
    std::size_t result = uri.find(search);
    EXPECT_NE(result, std::string::npos);
    EXPECT_GE(result, head);
    length += search.length();
    if (result != std::string::npos) {
        std::size_t pos = result + search.length();
        std::size_t delims = uri.find(";", pos);
        if (delims != std::string::npos) {
            std::string substring = uri.substr(pos, delims - pos);
            float valueFloatNew = Float::Unbox(Float::Parse(substring));
            EXPECT_EQ(valueFloatNew, valueFloatOrigin);
            length += substring.length() + 1;
        }
    }
}

void DistributedWantV2BaseTest::TestFloatArrayForParseUri(std::string uri,
    std::string keyFloatArray, std::size_t &length, std::vector<float> valueFloatArrayOrigin)
{
    std::size_t head = length;
    std::string search = Array::SIGNATURE + std::string(".") + keyFloatArray + std::string("=");
    std::size_t result = uri.find(search);
    EXPECT_NE(result, std::string::npos);
    EXPECT_GE(result, head);
    length += search.length();
    if (result != std::string::npos) {
        std::size_t pos = result + search.length();
        std::size_t delims = uri.find(";", result);
        if (delims != std::string::npos) {
            std::string substring = uri.substr(pos, delims - pos);
            sptr<IArray> array = Array::Parse(substring);
            std::vector<float> valueFloatArrayNew;
            auto func = [&valueFloatArrayNew](
                            IInterface *object) { valueFloatArrayNew.push_back(Float::Unbox(IFloat::Query(object))); };
            Array::ForEach(array, func);
            EXPECT_EQ(valueFloatArrayNew, valueFloatArrayOrigin);
            length += substring.length() + 1;
        }
    }
}

void DistributedWantV2BaseTest::TestStringArrayForParseUri(std::string uri,
    std::string keyStringArray, std::size_t &length, std::vector<std::string> valueStringArrayOrigin)
{
    std::size_t head = length;
    std::string search = Array::SIGNATURE + std::string(".") + keyStringArray + std::string("=");
    std::size_t result = uri.find(search);
    EXPECT_NE(result, std::string::npos);
    EXPECT_GE(result, head);
    length += search.length();
    if (result != std::string::npos) {
        std::size_t pos = result + search.length();
        std::size_t delims = uri.find(";", result);
        if (delims != std::string::npos) {
            std::string substring = uri.substr(pos, delims - pos);
            sptr<IArray> array = Array::Parse(substring);
            std::vector<std::string> valueStringArrayNew;
            auto func = [&valueStringArrayNew](IInterface *object) {
                valueStringArrayNew.push_back(String::Unbox(IString::Query(object)));
            };
            Array::ForEach(array, func);
            EXPECT_EQ(valueStringArrayNew, valueStringArrayOrigin);
            length += substring.length() + 1;
        }
    }
}

/**
 * @tc.number: DistributedScheduleWant_Parcelable_0600
 * @tc.name: parcelable
 * @tc.desc: Verify parcelable.
 */
HWTEST_F(DistributedWantV2BaseTest, DistributedScheduleWant_Parcelable_0600, Function | MediumTest | Level3)
{
    std::string action = "want.action.test";
    unsigned int flag = 0x789;
    std::string entity = "want.entity.test";
    OHOS::AppExecFwk::ElementName element("bundlename", "appname", "abilityname");

    std::shared_ptr<DistributedWantV2> want = std::make_shared<DistributedWantV2>();
    if (want != nullptr) {
        want->SetAction(action);
        want->AddEntity(entity);
        want->AddFlags(flag);
        want->SetElement(element);

        std::map<std::string, std::string> keys;

        SendParcelTest(want, keys);
    }
}

/**
 * @tc.number: DistributedScheduleWant_Parcelable_0700
 * @tc.name: parcelable
 * @tc.desc: Verify parcelable.
 */
HWTEST_F(DistributedWantV2BaseTest, DistributedScheduleWant_Parcelable_0700, Function | MediumTest | Level3)
{
    std::string action = "want.action.test";
    unsigned int flag = 0x789;
    std::string entity = "want.entity.test";
    OHOS::AppExecFwk::ElementName element("bundlename", "appname", "abilityname");

    std::shared_ptr<DistributedWantV2> want = std::make_shared<DistributedWantV2>();
    if (want != nullptr) {
        want->SetAction(action);
        want->AddEntity(entity);
        want->AddFlags(flag);
        want->SetElement(element);

        int loop = 1;
        std::map<std::string, std::string> keys;

        AddByteParams(*(want.get()), keys, loop, FLAG_TEST_BOTH);
        AddCharParams(*(want.get()), keys, loop, FLAG_TEST_BOTH);
        AddShortParams(*(want.get()), keys, loop, FLAG_TEST_BOTH);
        AddIntParams(*(want.get()), keys, loop, FLAG_TEST_BOTH);
        AddLongParams(*(want.get()), keys, loop, FLAG_TEST_BOTH);
        AddFloatParams(*(want.get()), keys, loop, FLAG_TEST_BOTH);
        AddDoubleParams(*(want.get()), keys, loop, FLAG_TEST_BOTH);
        AddStringParams(*(want.get()), keys, loop, FLAG_TEST_BOTH);

        SendParcelTest(want, keys);
    }
}

/**
 * @tc.number: DistributedScheduleWant_Parcelable_0800
 * @tc.name: parcelable
 * @tc.desc: Verify parcelable.
 */
HWTEST_F(DistributedWantV2BaseTest, DistributedScheduleWant_Parcelable_0800, Function | MediumTest | Level3)
{
    std::string action = "want.action.test";
    unsigned int flag = 0x789;
    std::string entity = "want.entity.test";
    OHOS::AppExecFwk::ElementName element("bundlename", "appname", "abilityname");

    std::shared_ptr<DistributedWantV2> want = std::make_shared<DistributedWantV2>();
    if (want != nullptr) {
        want->SetAction(action);
        want->AddEntity(entity);
        want->AddFlags(flag);
        want->SetElement(element);
        std::map<std::string, std::string> keys;

        SendParcelTest(want, keys);
    }
}

/**
 * @tc.number:  DistributedScheduleWant_Flags_0100
 * @tc.name: SetFlags/AddFlags/GetFlags/RemoveFlags
 * @tc.desc: Verify SetFlags/AddFlags/GetFlags/RemoveFlags.
 */
HWTEST_F(DistributedWantV2BaseTest, DistributedScheduleWant_Flags_0100, Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    int flags = 3;
    int returnsflags;
    int description = 8;

    want_->SetFlags(description);
    want_->AddFlags(flags);
    returnsflags = want_->GetFlags();
    EXPECT_EQ(11, returnsflags);

    want_->RemoveFlags(flags);
    returnsflags = want_->GetFlags();
    EXPECT_EQ(description, returnsflags);
}

/**
 * @tc.number:  DistributedScheduleWant_ClearWant_0100
 * @tc.name: ClearWant
 * @tc.desc: Verify ClearWant.
 */
HWTEST_F(DistributedWantV2BaseTest, DistributedScheduleWant_ClearWant_0100, Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    DistributedWantV2 want;
    ElementName elementName;
    std::string empty = "";
    want_->ClearWant(&want);

    EXPECT_EQ((uint)0, want_->GetFlags());
    EXPECT_EQ(empty, want_->GetType());
    EXPECT_EQ(empty, want_->GetAction());
    EXPECT_EQ(elementName, want_->GetElement());
    EXPECT_EQ((size_t)0, want_->GetEntities().size());
    EXPECT_EQ(0, want_->CountEntities());
}

/**
 * @tc.number:  DistributedScheduleWant_replaceParams_0100
 * @tc.name: replaceParams(wantParams)
 * @tc.desc: Verify the function when the input string is empty.
 */
HWTEST_F(DistributedWantV2BaseTest, DistributedScheduleWant_replaceParams_0100, Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    AAFwk::WantParams wantParams;
    std::string keyStr = "123";
    std::string valueStr = "123";
    wantParams.SetParam(keyStr, String::Box(valueStr));
    want_->ReplaceParams(wantParams);

    EXPECT_EQ(valueStr, String::Unbox(IString::Query(want_->GetParams().GetParam(keyStr))));
}

/**
 * @tc.number:  DistributedScheduleWant_setElement_0100
 * @tc.name:setElement / setElementName
 * @tc.desc: Verify the function when the input string is empty.
 */
HWTEST_F(DistributedWantV2BaseTest, DistributedScheduleWant_setElement_0100, Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    std::string valueStr1 = "xxxxx";
    std::string valueStr2 = "uaid";
    std::string valueStr3 = "uaygfi";

    OHOS::AppExecFwk::ElementName elementname1;
    OHOS::AppExecFwk::ElementName elementname2;
    OHOS::AppExecFwk::ElementName elementname3;
    ElementName elementname4;
    elementname1.SetAbilityName(valueStr1);
    elementname2.SetDeviceID(valueStr2);
    elementname3.SetBundleName(valueStr3);
    want_->SetElement(elementname1);
    EXPECT_EQ(valueStr1, want_->GetElement().GetAbilityName());

    want_->SetElement(elementname2);
    EXPECT_EQ(valueStr2, want_->GetElement().GetDeviceID());

    want_->SetElement(elementname3);
    EXPECT_EQ(valueStr3, want_->GetElement().GetBundleName());

    want_->SetElementName(valueStr3, valueStr1);
    EXPECT_EQ(valueStr1, want_->GetElement().GetAbilityName());
    EXPECT_EQ(valueStr3, want_->GetElement().GetBundleName());

    want_->SetElementName(valueStr2, valueStr3, valueStr1);
    EXPECT_EQ(valueStr1, want_->GetElement().GetAbilityName());
    EXPECT_EQ(valueStr2, want_->GetElement().GetDeviceID());
    EXPECT_EQ(valueStr3, want_->GetElement().GetBundleName());
}

/**
 * @tc.number:  DistributedScheduleWant_Action_0200
 * @tc.name:SetAction / GetAction
 * @tc.desc: Verify the function when the input string is empty.
 */
HWTEST_F(DistributedWantV2BaseTest, DistributedScheduleWant_Action_0200, Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    std::string setValue;
    want_->SetAction(setValue);
    EXPECT_EQ(setValue, want_->GetAction());
}

/**
 * @tc.number:  DistributedScheduleWant_Action_0300
 * @tc.name:SetAction / GetAction
 * @tc.desc: Verify the function when the input string contains special characters.
 */
HWTEST_F(DistributedWantV2BaseTest, DistributedScheduleWant_Action_0300, Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    std::string setValue("action.system.com");
    want_->SetAction(setValue);
    EXPECT_STREQ(setValue.c_str(), want_->GetAction().c_str());
}

using testByteType = std::tuple<std::string, std::string, byte, byte, byte>;
class DistributedWantV2ParametersBoolArrayTest : public testing::TestWithParam<testByteType> {
public:
    DistributedWantV2ParametersBoolArrayTest()
    {
        want_ = nullptr;
    }
    ~DistributedWantV2ParametersBoolArrayTest()
    {
        want_ = nullptr;
    }
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<DistributedWantV2> want_;
};

void DistributedWantV2ParametersBoolArrayTest::SetUpTestCase(void)
{}

void DistributedWantV2ParametersBoolArrayTest::TearDownTestCase(void)
{}

void DistributedWantV2ParametersBoolArrayTest::SetUp(void)
{
    want_ = std::make_shared<DistributedWantV2>();
}

void DistributedWantV2ParametersBoolArrayTest::TearDown(void)
{}

/**
 * @tc.number:  DistributedScheduleWant_BoolArr_0100
 * @tc.name:SetBoolArrayParam/GetBoolArrayParam
 * @tc.desc: Verify when parameter change.
 */
HWTEST_P(DistributedWantV2ParametersBoolArrayTest,
    DistributedScheduleWant_BoolArr_0100, Function | MediumTest | Level3)
{
    std::string setKey = std::get<0>(GetParam());
    std::string getKey = std::get<1>(GetParam());
    byte setValue = std::get<2>(GetParam());
    byte defaultValue = std::get<3>(GetParam());
    byte result = std::get<4>(GetParam());
    want_->SetParam(setKey, setValue);
    EXPECT_EQ(result, want_->GetByteParam(getKey, defaultValue));
}

INSTANTIATE_TEST_SUITE_P(WantParametersBoolArrayTestCaseP, DistributedWantV2ParametersBoolArrayTest,
    testing::Values(testByteType("", "aa", '#', 'U', 'U'), testByteType("", "", 'N', 'K', 'N'),
        testByteType("1*中_aR", "aa", 'a', '%', '%'), testByteType("1*中_aR", "1*中_aR", 'a', 'z', 'a')));

using testBoolArrayType =
    std::tuple<std::string, std::string, std::vector<bool>, std::vector<bool>, std::vector<bool>>;
class DistributedWantV2BoolArrayParamTest : public testing::TestWithParam<testBoolArrayType> {
public:
    DistributedWantV2BoolArrayParamTest()
    {
        want_ = nullptr;
    }
    ~DistributedWantV2BoolArrayParamTest()
    {
        want_ = nullptr;
    }
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<DistributedWantV2> want_;
};

void DistributedWantV2BoolArrayParamTest::SetUpTestCase(void)
{}

void DistributedWantV2BoolArrayParamTest::TearDownTestCase(void)
{}

void DistributedWantV2BoolArrayParamTest::SetUp(void)
{
    want_ = std::make_shared<DistributedWantV2>();
}

void DistributedWantV2BoolArrayParamTest::TearDown(void)
{}

/**
 * @tc.number:  DistributedScheduleWant_BoolArray_0200
 * @tc.name:SetBoolArrayParam/GetBoolArrayParam
 * @tc.desc: Verify when parameter change.
 */
HWTEST_P(DistributedWantV2BoolArrayParamTest, DistributedScheduleWant_BoolArray_0200, Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    std::string setKey = std::get<0>(GetParam());
    std::string getKey = std::get<1>(GetParam());
    std::vector<bool> setValue = std::get<2>(GetParam());
    std::vector<bool> defaultValue = std::get<3>(GetParam());
    std::vector<bool> result = std::get<4>(GetParam());
    want_->SetParam(setKey, setValue);
    EXPECT_EQ(result, want_->GetBoolArrayParam(getKey));
}

INSTANTIATE_TEST_SUITE_P(WantBoolArrayParamTestCaseP, DistributedWantV2BoolArrayParamTest,
    testing::Values(testBoolArrayType("", "aa", {true, false}, {}, {}),
        testBoolArrayType("", "", {true, false}, {}, {true, false}),
        testBoolArrayType("1*中_aR", "aa", {true, false}, {}, {}),
        testBoolArrayType("1*中_aR", "1*中_aR", {false, true}, {}, {false, true})));

using testCharArrayType =
    std::tuple<std::string, std::string, std::vector<zchar>, std::vector<zchar>, std::vector<zchar>>;
class DistributedWantV2CharArrayParamTest : public testing::TestWithParam<testCharArrayType> {
public:
    DistributedWantV2CharArrayParamTest()
    {
        want_ = nullptr;
    }
    ~DistributedWantV2CharArrayParamTest()
    {
        want_ = nullptr;
    }
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<DistributedWantV2> want_;
};

void DistributedWantV2CharArrayParamTest::SetUpTestCase(void)
{}

void DistributedWantV2CharArrayParamTest::TearDownTestCase(void)
{}

void DistributedWantV2CharArrayParamTest::SetUp(void)
{
    want_ = std::make_shared<DistributedWantV2>();
}

void DistributedWantV2CharArrayParamTest::TearDown(void)
{}

/**
 * @tc.number:  DistributedScheduleWant_Parameters_CharArray_0100
 * @tc.name: SetParam/GetCharArrayParam
 * @tc.desc: Verify when parameter change.
 */
HWTEST_P(DistributedWantV2CharArrayParamTest, DistributedScheduleWant_Parameters_CharArray_0100,
    Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    std::string setKey = std::get<0>(GetParam());
    std::string getKey = std::get<1>(GetParam());
    std::vector<zchar> setValue = std::get<2>(GetParam());
    std::vector<zchar> defaultValue = std::get<3>(GetParam());
    std::vector<zchar> result = std::get<4>(GetParam());
    want_->SetParam(setKey, setValue);
    EXPECT_EQ(result, want_->GetCharArrayParam(getKey));
}

INSTANTIATE_TEST_SUITE_P(WantCharArrayParamTestCaseP, DistributedWantV2CharArrayParamTest,
    testing::Values(testCharArrayType("", "aa", {U'中', U'文'}, {}, {}),
        testCharArrayType("", "", {U'中', U'文'}, {}, {U'中', U'文'}),
        testCharArrayType("1*中_aR", "aa", {U'中', U'文'}, {}, {}),
        testCharArrayType("1*中_aR", "1*中_aR", {U'中', U'文'}, {}, {U'中', U'文'})));

/**
 * @tc.number:  DistributedScheduleWant_Parameters_CharArray_0200
 * @tc.name:  GetCharArrayParam
 * @tc.desc: Verify when the value is char array.
 */
HWTEST_F(DistributedWantV2CharArrayParamTest, DistributedScheduleWant_Parameters_CharArray_0200,
    Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    std::vector<zchar> defaultValue;
    std::string getKey("aa");
    EXPECT_EQ(defaultValue, want_->GetCharArrayParam(getKey));
}

/**
 * @tc.number:  DistributedScheduleWant_Parameters_CharArray_0300
 * @tc.name:  SetParam/GetCharArrayParam
 * @tc.desc: Verify when the value is char array.
 */
HWTEST_F(DistributedWantV2CharArrayParamTest, DistributedScheduleWant_Parameters_CharArray_0300,
    Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    std::string emptyStr("ff");
    std::vector<zchar> firstValue({U'中', U'文'});
    std::vector<zchar> secondValue({U'字', U'符'});
    std::vector<zchar> thirdValue({U'集', U'英'});
    std::string keyStr("aa");
    want_->SetParam(emptyStr, firstValue);
    want_->SetParam(emptyStr, firstValue);
    want_->SetParam(emptyStr, secondValue);
    std::vector<zchar> defaultValue;
    EXPECT_EQ(defaultValue, want_->GetCharArrayParam(keyStr));
    want_->SetParam(emptyStr, thirdValue);
    EXPECT_EQ(thirdValue, want_->GetCharArrayParam(emptyStr));
}

/**
 * @tc.number: DistributedScheduleWant_Parameters_CharArray_0400
 * @tc.name:  SetParam/GetCharArrayParam
 * @tc.desc: Verify when the value is char array.
 */
HWTEST_F(DistributedWantV2CharArrayParamTest, DistributedScheduleWant_Parameters_CharArray_0400,
    Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    std::string firstKey("%1uH3");
    std::vector<zchar> firstValue({U'中', U'文'});
    std::vector<zchar> secondValue({U'字', U'符'});
    std::vector<zchar> defaultValue;
    std::string secondKey("aa");
    want_->SetParam(firstKey, firstValue);
    want_->SetParam(firstKey, firstValue);
    want_->SetParam(firstKey, secondValue);
    EXPECT_EQ(secondValue, want_->GetCharArrayParam(firstKey));
    want_->SetParam(firstKey, firstValue);
    EXPECT_EQ(defaultValue, want_->GetCharArrayParam(secondKey));
}

using testCharType = std::tuple<std::string, std::string, zchar, zchar, zchar>;
class DistributedWantV2CharParamTest : public testing::TestWithParam<testCharType> {
public:
    DistributedWantV2CharParamTest()
    {
        want_ = nullptr;
    }
    ~DistributedWantV2CharParamTest()
    {
        want_ = nullptr;
    }
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<DistributedWantV2> want_;
};

void DistributedWantV2CharParamTest::SetUpTestCase(void)
{}

void DistributedWantV2CharParamTest::TearDownTestCase(void)
{}

void DistributedWantV2CharParamTest::SetUp(void)
{
    want_ = std::make_shared<DistributedWantV2>();
}

void DistributedWantV2CharParamTest::TearDown(void)
{}

/**
 * @tc.number: DistributedScheduleWant_Parameters_Char_0100
 * @tc.name:  SetParam/GetCharParam
 * @tc.desc: Verify when the value is char array.
 */
HWTEST_P(DistributedWantV2CharParamTest, DistributedScheduleWant_Parameters_Char_0100, Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    std::string setKey = std::get<0>(GetParam());
    std::string getKey = std::get<1>(GetParam());
    zchar setValue = std::get<2>(GetParam());
    zchar defaultValue = std::get<3>(GetParam());
    zchar result = std::get<4>(GetParam());
    want_->SetParam(setKey, setValue);
    EXPECT_EQ(result, want_->GetCharParam(getKey, defaultValue));
}

INSTANTIATE_TEST_SUITE_P(WantParametersCharTestCaseP, DistributedWantV2CharParamTest,
    testing::Values(testCharType("", "aa", U'#', U'中', U'中'), testCharType("", "", U'中', U'K', U'中'),
        testCharType("1*中_aR", "aa", U'a', U'中', U'中'), testCharType("1*中_aR", "1*中_aR", U'中', U'z', U'中')));

/**
 * @tc.number: DistributedScheduleWant_Parameters_Char_0200
 * @tc.name:  SetParam/GetCharParam
 * @tc.desc: Verify when the value is char
 */
HWTEST_F(DistributedWantV2CharParamTest, DistributedScheduleWant_Parameters_Char_0200, Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    zchar defaultValue = U'文';
    std::string getKey("aa");
    EXPECT_EQ(defaultValue, want_->GetCharParam(getKey, defaultValue));
}

/**
 * @tc.number: DistributedScheduleWant_Parameters_Char_0300
 * @tc.name:  SetParam/GetCharParam
 * @tc.desc: Verify when the value is char.
 */
HWTEST_F(DistributedWantV2CharParamTest, DistributedScheduleWant_Parameters_Char_0300, Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    std::string emptyStr("jj");
    zchar firstValue = U'中';
    zchar secondValue = U'文';
    zchar thirdValue = U'字';
    zchar firstDefaultValue = U'符';
    zchar secondDefaultValue = U'集';
    std::string keyStr("aa");
    want_->SetParam(emptyStr, firstValue);
    want_->SetParam(emptyStr, firstValue);
    want_->SetParam(emptyStr, secondValue);
    EXPECT_EQ(firstDefaultValue, want_->GetCharParam(keyStr, firstDefaultValue));
    want_->SetParam(emptyStr, thirdValue);
    EXPECT_EQ(thirdValue, want_->GetCharParam(emptyStr, secondDefaultValue));
}

/**
 * @tc.number: DistributedScheduleWant_Parameters_Char_0400
 * @tc.name:  SetParam/GetCharParam
 * @tc.desc: Verify when the value is char.
 */
HWTEST_F(DistributedWantV2CharParamTest, DistributedScheduleWant_Parameters_Char_0400, Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    std::string firstKey("%1uH3");
    zchar firstValue = U'中';
    zchar secondValue = U'文';
    zchar firstDefaultValue = U'字';
    zchar secondDefaultValue = U'符';
    std::string secondKey("aa");
    want_->SetParam(firstKey, firstValue);
    want_->SetParam(firstKey, firstValue);
    want_->SetParam(firstKey, secondValue);
    EXPECT_EQ(secondValue, want_->GetCharParam(firstKey, firstDefaultValue));
    want_->SetParam(firstKey, firstValue);
    EXPECT_EQ(secondDefaultValue, want_->GetCharParam(secondKey, secondDefaultValue));
}

using testDoubleArrayType =
    std::tuple<std::string, std::string, std::vector<double>, std::vector<double>, std::vector<double>>;
class DistributedWantV2DoubleArrayParamTest : public testing::TestWithParam<testDoubleArrayType> {
public:
    DistributedWantV2DoubleArrayParamTest()
    {
        want_ = nullptr;
    }
    ~DistributedWantV2DoubleArrayParamTest()
    {
        want_ = nullptr;
    }
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<DistributedWantV2> want_;
};

void DistributedWantV2DoubleArrayParamTest::SetUpTestCase(void)
{}

void DistributedWantV2DoubleArrayParamTest::TearDownTestCase(void)
{}

void DistributedWantV2DoubleArrayParamTest::SetUp(void)
{
    want_ = std::make_shared<DistributedWantV2>();
}

void DistributedWantV2DoubleArrayParamTest::TearDown(void)
{}

/**
 * @tc.number: DistributedScheduleWant_DoubleArray_0100
 * @tc.name:  SetParam/GetDoubleArrayParam
 * @tc.desc: Verify when parameter change.
 */
HWTEST_P(DistributedWantV2DoubleArrayParamTest,
    DistributedScheduleWant_DoubleArray_0100, Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    std::string setKey = std::get<0>(GetParam());
    std::string getKey = std::get<1>(GetParam());
    std::vector<double> setValue = std::get<2>(GetParam());
    std::vector<double> defaultValue = std::get<3>(GetParam());
    std::vector<double> result = std::get<4>(GetParam());
    want_->SetParam(setKey, setValue);
    EXPECT_EQ(result, want_->GetDoubleArrayParam(getKey));
}

INSTANTIATE_TEST_SUITE_P(WantDoubleArrayParamTestCaseP, DistributedWantV2DoubleArrayParamTest,
    testing::Values(testDoubleArrayType("", "aa", {-1.1, -2.1}, {}, {}),
        testDoubleArrayType("", "", {-41.1, -42.1}, {}, {-41.1, -42.1}),
        testDoubleArrayType("1*中_aR", "aa", {50.1, 51.1}, {}, {}),
        testDoubleArrayType("1*中_aR", "1*中_aR", {5000.1, 5001.1}, {}, {5000.1, 5001.1})));

/**
 * @tc.number: DistributedScheduleWant_DoubleArray_0200
 * @tc.name:  SetParam/GetDoubleArrayParam
 * @tc.desc: Verify when parameter change.
 */
HWTEST_F(DistributedWantV2DoubleArrayParamTest,
    DistributedScheduleWant_DoubleArray_0200, Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    std::vector<double> defaultValue;
    std::string key = "aa";
    EXPECT_EQ(defaultValue, want_->GetDoubleArrayParam(key));
}

/**
 * @tc.number: DistributedScheduleWant_DoubleArray_0300
 * @tc.name:  SetParam/GetDoubleArrayParam
 * @tc.desc: set empty-string key repeatedly, but get param of another nonexistent key
 */
HWTEST_F(DistributedWantV2DoubleArrayParamTest,
    DistributedScheduleWant_DoubleArray_0300, Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    std::vector<double> defaultValue;
    std::string setKey1 = "cc";
    std::string setKey2 = "aa";
    std::vector<double> setValue1 = {1.1, 2.1};
    std::vector<double> setValue2 = {5.1, 6.1};
    want_->SetParam(setKey1, setValue1);
    want_->SetParam(setKey1, setValue1);
    setValue1 = {2.1, 3.1};
    want_->SetParam(setKey1, setValue1);
    EXPECT_EQ(defaultValue, want_->GetDoubleArrayParam(setKey2));
    setValue1 = {4.1, 5.1};
    want_->SetParam(setKey1, setValue1);
    EXPECT_EQ(setValue1, want_->GetDoubleArrayParam(setKey1));
}

/**
 * @tc.number: DistributedScheduleWant_DoubleArray_0400
 * @tc.name:  SetParam/GetDoubleArrayParam
 * @tc.desc: set empty-string key repeatedly, then get param of the key
 */
HWTEST_F(DistributedWantV2DoubleArrayParamTest,
    DistributedScheduleWant_DoubleArray_0400, Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    std::vector<double> defaultValue;
    std::string setKey1 = "%1uH3";
    std::string setKey2 = "aa";
    std::vector<double> setValue1 = {-1.1, -2.1};
    std::vector<double> setValue2 = {9.1, 10.1};
    want_->SetParam(setKey1, setValue1);
    want_->SetParam(setKey1, setValue1);
    setValue1 = {0.1, 1.1};
    want_->SetParam(setKey1, setValue1);
    EXPECT_EQ(setValue1, want_->GetDoubleArrayParam(setKey1));
    setValue1 = {4.1, 5.1};
    want_->SetParam(setKey1, setValue1);
    setValue1 = {-10.1, -11.1};
    EXPECT_EQ(defaultValue, want_->GetDoubleArrayParam(setKey2));
}

using testFloatArrayType =
    std::tuple<std::string, std::string, std::vector<float>, std::vector<float>, std::vector<float>>;
class DistributedWantV2FloatArrayParamTest : public testing::TestWithParam<testFloatArrayType> {
public:
    DistributedWantV2FloatArrayParamTest()
    {
        want_ = nullptr;
    }
    ~DistributedWantV2FloatArrayParamTest()
    {
        want_ = nullptr;
    }
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<DistributedWantV2> want_;
};

void DistributedWantV2FloatArrayParamTest::SetUpTestCase(void)
{}

void DistributedWantV2FloatArrayParamTest::TearDownTestCase(void)
{}

void DistributedWantV2FloatArrayParamTest::SetUp(void)
{
    want_ = std::make_shared<DistributedWantV2>();
}

void DistributedWantV2FloatArrayParamTest::TearDown(void)
{}

/**
 * @tc.number: DistributedScheduleWant_FloatArray_0100
 * @tc.name:  SetParam/GetFloatArrayParam
 * @tc.desc: Verify when parameter change.
 */
HWTEST_P(DistributedWantV2FloatArrayParamTest, DistributedScheduleWant_FloatArray_0100, Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    std::string setKey = std::get<0>(GetParam());
    std::string getKey = std::get<1>(GetParam());
    std::vector<float> setValue = std::get<2>(GetParam());
    std::vector<float> defaultValue = std::get<3>(GetParam());
    std::vector<float> result = std::get<4>(GetParam());
    want_->SetParam(setKey, setValue);
    EXPECT_EQ(result, want_->GetFloatArrayParam(getKey));
}

INSTANTIATE_TEST_SUITE_P(WantFloatArrayParamTestCaseP, DistributedWantV2FloatArrayParamTest,
    testing::Values(testFloatArrayType("", "aa", {-1.1, -2.1}, {}, {}),
        testFloatArrayType("", "", {-41.1, -42.1}, {}, {-41.1, -42.1}),
        testFloatArrayType("1*中_aR", "aa", {50.1, 51.1}, {}, {}),
        testFloatArrayType("1*中_aR", "1*中_aR", {5000.1, 5001.1}, {}, {5000.1, 5001.1})));

/**
 * @tc.number: DistributedScheduleWant_FloatArray_0200
 * @tc.name:  SetParam/GetFloatArrayParam
 * @tc.desc: get param when WantParam is empty
 */
HWTEST_F(DistributedWantV2FloatArrayParamTest, DistributedScheduleWant_FloatArray_0200, Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    std::vector<float> defaultValue;
    std::string key = "aa";
    EXPECT_EQ(defaultValue, want_->GetFloatArrayParam(key));
}

/**
 * @tc.number: DistributedScheduleWant_FloatArray_0300
 * @tc.name:  SetParam & GetFloatArrayParam
 * @tc.desc: set empty-string key repeatedly, but get param of another nonexistent key
 */
HWTEST_F(DistributedWantV2FloatArrayParamTest, DistributedScheduleWant_FloatArray_0300, Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    std::vector<float> defaultValue;
    std::string setKey1 = "hh";
    std::string setKey2 = "aa";
    std::vector<float> setValue1 = {1.1, 2.1};
    want_->SetParam(setKey1, setValue1);
    want_->SetParam(setKey1, setValue1);
    setValue1 = {2.1, 3.1};
    want_->SetParam(setKey1, setValue1);
    EXPECT_EQ(defaultValue, want_->GetFloatArrayParam(setKey2));
    setValue1 = {4.1, 5.1};
    want_->SetParam(setKey1, setValue1);
    EXPECT_EQ(setValue1, want_->GetFloatArrayParam(setKey1));
}

/**
 * @tc.number: DistributedScheduleWant_FloatArray_0400
 * @tc.name:  SetParam & GetFloatArrayParam
 * @tc.desc: set empty-string key repeatedly, then get param of the key
 */
HWTEST_F(DistributedWantV2FloatArrayParamTest, DistributedScheduleWant_FloatArray_0400, Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    std::vector<float> defaultValue;
    std::string setKey1 = "%1uH3";
    std::string setKey2 = "aa";
    std::vector<float> setValue1 = {-1.1, -2.1};
    std::vector<float> setValue2 = {9.1, 10.1};
    want_->SetParam(setKey1, setValue1);
    want_->SetParam(setKey1, setValue1);
    setValue1 = {0.1, 1.1};
    want_->SetParam(setKey1, setValue1);
    EXPECT_EQ(setValue1, want_->GetFloatArrayParam(setKey1));
    setValue1 = {4.1, 5.1};
    want_->SetParam(setKey1, setValue1);
    EXPECT_EQ(defaultValue, want_->GetFloatArrayParam(setKey2));
}

using testLongArrayType =
    std::tuple<std::string, std::string, std::vector<long>, std::vector<long>, std::vector<long>>;
class DistributedWantV2LongArrayParamTest : public testing::TestWithParam<testLongArrayType> {
public:
    DistributedWantV2LongArrayParamTest()
    {
        want_ = nullptr;
    }
    ~DistributedWantV2LongArrayParamTest()
    {
        want_ = nullptr;
    }
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<DistributedWantV2> want_;
};

void DistributedWantV2LongArrayParamTest::SetUpTestCase(void)
{}

void DistributedWantV2LongArrayParamTest::TearDownTestCase(void)
{}

void DistributedWantV2LongArrayParamTest::SetUp(void)
{
    want_ = std::make_shared<DistributedWantV2>();
}

void DistributedWantV2LongArrayParamTest::TearDown(void)
{}

/**
 * @tc.number: DistributedScheduleWant_LongArray_0100
 * @tc.name:  SetParam & GetLongArrayParam
 * @tc.desc: Verify when parameter change.
 */
HWTEST_P(DistributedWantV2LongArrayParamTest, DistributedScheduleWant_LongArray_0100, Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    std::string setKey = std::get<0>(GetParam());
    std::string getKey = std::get<1>(GetParam());
    std::vector<long> setValue = std::get<2>(GetParam());
    std::vector<long> defaultValue = std::get<3>(GetParam());
    std::vector<long> result = std::get<4>(GetParam());
    want_->SetParam(setKey, setValue);
    EXPECT_EQ(result, want_->GetLongArrayParam(getKey));
}

INSTANTIATE_TEST_SUITE_P(WantLongArrayParamTestCaseP, DistributedWantV2LongArrayParamTest,
    testing::Values(testLongArrayType("", "aa", {-1, 3, 25, -9}, {}, {}),
        testLongArrayType("", "", {-41, 0, 0, 9}, {}, {-41, 0, 0, 9}),
        testLongArrayType("1*中_aR", "aa", {50, 2, -9}, {}, {}),
        testLongArrayType("1*中_aR", "1*中_aR", {-5000}, {}, {-5000})));

/**
 * @tc.number: DistributedScheduleWant_LongArray_0200
 * @tc.name:  SetParam & GetLongArrayParam
 * @tc.desc: get param when WantParam is empty
 */
HWTEST_F(DistributedWantV2LongArrayParamTest, DistributedScheduleWant_LongArray_0200, Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    std::vector<long> defaultValue;
    std::string key = "aa";
    EXPECT_EQ(defaultValue, want_->GetLongArrayParam(key));
}

/**
 * @tc.number: DistributedScheduleWant_LongArray_0300
 * @tc.name:  SetParam & GetLongArrayParam
 * @tc.desc: set empty-string key repeatedly, but get param of another nonexistent key
 */
HWTEST_F(DistributedWantV2LongArrayParamTest, DistributedScheduleWant_LongArray_0300, Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    std::vector<long> defaultValue;
    std::string setKey1 = "bb";
    std::string setKey2 = "aa";
    std::vector<long> setValue1 = {1, 2};
    want_->SetParam(setKey1, setValue1);
    want_->SetParam(setKey1, setValue1);
    setValue1 = {2, 3};
    want_->SetParam(setKey1, setValue1);
    EXPECT_EQ(defaultValue, want_->GetLongArrayParam(setKey2));
    setValue1 = {4, 5};
    want_->SetParam(setKey1, setValue1);
    EXPECT_EQ(setValue1, want_->GetLongArrayParam(setKey1));
}

/**
 * @tc.number: DistributedScheduleWant_LongArray_0400
 * @tc.name:  SetParam & GetLongArrayParam
 * @tc.desc: set empty-string key repeatedly, then get param of the key
 */
HWTEST_F(DistributedWantV2LongArrayParamTest, DistributedScheduleWant_LongArray_0400, Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    std::vector<long> defaultValue;
    std::string setKey1 = "%1uH3";
    std::string setKey2 = "aa";
    std::vector<long> setValue1 = {-1, -2};
    want_->SetParam(setKey1, setValue1);
    want_->SetParam(setKey1, setValue1);
    setValue1 = {0, 1};
    want_->SetParam(setKey1, setValue1);
    EXPECT_EQ(setValue1, want_->GetLongArrayParam(setKey1));
    setValue1 = {4, 5};
    want_->SetParam(setKey1, setValue1);
    EXPECT_EQ(defaultValue, want_->GetLongArrayParam(setKey2));
}

using testShortArrayType =
    std::tuple<std::string, std::string, std::vector<short>, std::vector<short>, std::vector<short>>;
class DistributedWantV2ShortArrayParamTest : public testing::TestWithParam<testShortArrayType> {
public:
    DistributedWantV2ShortArrayParamTest()
    {
        want_ = nullptr;
    }
    ~DistributedWantV2ShortArrayParamTest()
    {
        want_ = nullptr;
    }
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<DistributedWantV2> want_;
};

void DistributedWantV2ShortArrayParamTest::SetUpTestCase(void)
{}

void DistributedWantV2ShortArrayParamTest::TearDownTestCase(void)
{}

void DistributedWantV2ShortArrayParamTest::SetUp(void)
{
    want_ = std::make_shared<DistributedWantV2>();
}

void DistributedWantV2ShortArrayParamTest::TearDown(void)
{}

/**
 * @tc.number: DistributedScheduleWant_ShortArray_0100
 * @tc.name:  SetParam/GetShortArrayParam
 * @tc.desc: Verify when parameter change.
 */
HWTEST_P(DistributedWantV2ShortArrayParamTest, DistributedScheduleWant_ShortArray_0100, Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    std::string setKey = std::get<0>(GetParam());
    std::string getKey = std::get<1>(GetParam());
    std::vector<short> setValue = std::get<2>(GetParam());
    std::vector<short> defaultValue = std::get<3>(GetParam());
    std::vector<short> result = std::get<4>(GetParam());
    want_->SetParam(setKey, setValue);
    EXPECT_EQ(result, want_->GetShortArrayParam(getKey));
}

INSTANTIATE_TEST_SUITE_P(WantShortArrayParamTestCaseP, DistributedWantV2ShortArrayParamTest,
    testing::Values(testShortArrayType("", "aa", {-1, 3, 25, -9}, {}, {}),
        testShortArrayType("", "", {-41, 0, 0, 9}, {}, {-41, 0, 0, 9}),
        testShortArrayType("1*中_aR", "aa", {50, 2, -9}, {}, {}),
        testShortArrayType("1*中_aR", "1*中_aR", {-5000}, {}, {-5000})));

/**
 * @tc.number: DistributedScheduleWant_ShortArray_0200
 * @tc.name:  SetParam/GetShortArrayParam
 * @tc.desc: Verify when the value is short array
 */
HWTEST_F(DistributedWantV2ShortArrayParamTest, DistributedScheduleWant_ShortArray_0200, Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    std::vector<short> defaultValue;
    std::string getKey("aa");
    EXPECT_EQ(defaultValue, want_->GetShortArrayParam(getKey));
}

/**
 * @tc.number: DistributedScheduleWant_ShortArray_0300
 * @tc.name:  SetParam/GetShortArrayParam
 * @tc.desc: Verify when the value is short array
 */
HWTEST_F(DistributedWantV2ShortArrayParamTest, DistributedScheduleWant_ShortArray_0300, Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    std::string emptyStr("hh");
    std::vector<short> firstValue({1, 4, -9});
    std::vector<short> secondValue({1, 8, -9});
    std::vector<short> thirdValue({1, 4, 9});
    std::string keyStr("aa");
    want_->SetParam(emptyStr, firstValue);
    want_->SetParam(emptyStr, firstValue);
    want_->SetParam(emptyStr, secondValue);
    std::vector<short> defaultValue;
    EXPECT_EQ(defaultValue, want_->GetShortArrayParam(keyStr));
    want_->SetParam(emptyStr, thirdValue);
    EXPECT_EQ(thirdValue, want_->GetShortArrayParam(emptyStr));
}

/**
 * @tc.number: DistributedScheduleWant_ShortArray_0400
 * @tc.name:  SetParam/GetShortArrayParam
 * @tc.desc: Verify when the value is short array
 */
HWTEST_F(DistributedWantV2ShortArrayParamTest, DistributedScheduleWant_ShortArray_0400, Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    std::string firstKey("%1uH3");
    std::vector<short> firstValue({-1, -2});
    std::vector<short> secondValue({-1, -2, -1, -2, 0});
    std::vector<short> thirdValue({-1, -2, 100});
    std::string secondKey("aa");
    want_->SetParam(firstKey, firstValue);
    want_->SetParam(firstKey, firstValue);
    want_->SetParam(firstKey, secondValue);
    EXPECT_EQ(secondValue, want_->GetShortArrayParam(firstKey));
    want_->SetParam(firstKey, thirdValue);
    std::vector<short> defaultValue;
    EXPECT_EQ(defaultValue, want_->GetShortArrayParam(secondKey));
}

using testShortType = std::tuple<std::string, std::string, short, short, short>;
class DistributedWantV2ShortParamTest : public testing::TestWithParam<testShortType> {
public:
    DistributedWantV2ShortParamTest()
    {
        want_ = nullptr;
    }
    ~DistributedWantV2ShortParamTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<DistributedWantV2> want_;
};

void DistributedWantV2ShortParamTest::SetUpTestCase(void)
{}

void DistributedWantV2ShortParamTest::TearDownTestCase(void)
{}

void DistributedWantV2ShortParamTest::SetUp(void)
{
    want_ = std::make_shared<DistributedWantV2>();
}

void DistributedWantV2ShortParamTest::TearDown(void)
{}

/**
 * @tc.number: DistributedScheduleWant_Short_0100
 * @tc.name:  SetParam/GetShortParam
 * @tc.desc: Verify when parameter change.
 */
HWTEST_P(DistributedWantV2ShortParamTest, DistributedScheduleWant_Short_0100, Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    std::string setKey = std::get<0>(GetParam());
    std::string getKey = std::get<1>(GetParam());
    short setValue = std::get<2>(GetParam());
    short defaultValue = std::get<3>(GetParam());
    short result = std::get<4>(GetParam());
    want_->SetParam(setKey, setValue);
    EXPECT_EQ(result, want_->GetShortParam(getKey, defaultValue));
}

INSTANTIATE_TEST_SUITE_P(WantShortParamTestCaseP, DistributedWantV2ShortParamTest,
    testing::Values(testShortType("", "aa", -1, 100, 100), testShortType("", "", -9, -41, -9),
        testShortType("1*中_aR", "aa", 50, 5, 5), testShortType("1*中_aR", "1*中_aR", -5000, 5000, -5000)));

/**
 * @tc.number: DistributedScheduleWant_Short_0200
 * @tc.name:  SetParam/GetShortParam
 * @tc.desc: Verify when the value is short
 */
HWTEST_F(DistributedWantV2ShortParamTest, DistributedScheduleWant_Short_0200, Function | MediumTest | Level3)
{
    short defaultValue = 200;
    std::string getKey("aa");
    EXPECT_EQ(defaultValue, want_->GetShortParam(getKey, defaultValue));
}
/**
 * @tc.number: DistributedScheduleWant_Short_0300
 * @tc.name:  SetParam/GetShortParam
 * @tc.desc: Verify when the value is short
 */
HWTEST_F(DistributedWantV2ShortParamTest, DistributedScheduleWant_Short_0300, Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    std::string emptyStr("bb");
    short firstValue = 1;
    short secondValue = 2;
    short thirdValue = 4;
    short firstDefaultValue = 3;
    short secondDefaultValue = 5;
    std::string keyStr("aa");
    want_->SetParam(emptyStr, firstValue);
    want_->SetParam(emptyStr, firstValue);
    want_->SetParam(emptyStr, secondValue);
    EXPECT_EQ(firstDefaultValue, want_->GetShortParam(keyStr, firstDefaultValue));
    want_->SetParam(emptyStr, thirdValue);
    EXPECT_EQ(thirdValue, want_->GetShortParam(emptyStr, secondDefaultValue));
}

/**
 * @tc.number: DistributedScheduleWant_Short_0400
 * @tc.name:  SetParam/GetShortParam
 * @tc.desc: Verify when the value is short
 */
HWTEST_F(DistributedWantV2ShortParamTest, DistributedScheduleWant_Short_0400, Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    std::string firstKey("%1uH3");
    short firstValue = -1;
    short secondValue = 0;
    short thirdValue = 4;
    short firstDefaultValue = 9;
    short secondDefaultValue = -10;
    std::string secondKey("aa");
    want_->SetParam(firstKey, firstValue);
    want_->SetParam(firstKey, firstValue);
    want_->SetParam(firstKey, secondValue);
    EXPECT_EQ(secondValue, want_->GetShortParam(firstKey, firstDefaultValue));
    want_->SetParam(firstKey, thirdValue);
    EXPECT_EQ(secondDefaultValue, want_->GetShortParam(secondKey, secondDefaultValue));
}

using testStrArrayType =
    std::tuple<std::string, std::string, std::vector<std::string>, std::vector<std::string>, std::vector<std::string>>;
class DistributedWantV2StringArrayParamTest : public testing::TestWithParam<testStrArrayType> {
public:
    DistributedWantV2StringArrayParamTest()
    {
        want_ = nullptr;
    }
    ~DistributedWantV2StringArrayParamTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<DistributedWantV2> want_ = nullptr;
};

void DistributedWantV2StringArrayParamTest::SetUpTestCase(void)
{}

void DistributedWantV2StringArrayParamTest::TearDownTestCase(void)
{}

void DistributedWantV2StringArrayParamTest::SetUp(void)
{
    want_ = std::make_shared<DistributedWantV2>();
}

void DistributedWantV2StringArrayParamTest::TearDown(void)
{}

/**
 * @tc.number: DistributedScheduleWant_StringArray_0100
 * @tc.name:  SetParam/GetStringArrayParam
 * @tc.desc: Verify when parameter change.
 */
HWTEST_P(DistributedWantV2StringArrayParamTest,
    DistributedScheduleWant_StringArray_0100, Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    std::string setKey = std::get<0>(GetParam());
    std::string getKey = std::get<1>(GetParam());
    std::vector<std::string> setValue = std::get<2>(GetParam());
    std::vector<std::string> defaultValue = std::get<3>(GetParam());
    std::vector<std::string> result = std::get<4>(GetParam());
    want_->SetParam(setKey, setValue);
    EXPECT_EQ(result, want_->GetStringArrayParam(getKey));
}

INSTANTIATE_TEST_SUITE_P(WantStringArrayParamTestCaseP, DistributedWantV2StringArrayParamTest,
    testing::Values(testStrArrayType("", "aa", {"1*中_aR", "dbdb"}, {}, {}),
        testStrArrayType("", "", {"1*中_aR", "dbdb"}, {}, {"1*中_aR", "dbdb"}),
        testStrArrayType("1*中_aR", "aa", {"1*中_aR", "dbdb"}, {}, {}),
        testStrArrayType("1*中_aR", "1*中_aR", {"1*中_aR", "dbdb"}, {}, {"1*中_aR", "dbdb"})));

/**
 * @tc.number: DistributedScheduleWant_StringArray_0200
 * @tc.name:  SetParam/GetStringArrayParam
 * @tc.desc: get param when WantParam is empty
 */
HWTEST_F(DistributedWantV2StringArrayParamTest,
    DistributedScheduleWant_StringArray_0200, Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    std::vector<std::string> defaultValue;
    std::string key = "aa";
    std::vector<std::string> resultValue = want_->GetStringArrayParam(key);
    EXPECT_EQ(defaultValue, resultValue);
}

/**
 * @tc.number: DistributedScheduleWant_StringArray_0300
 * @tc.name:  SetParam/GetStringArrayParam
 * @tc.desc: set empty-string key repeatedly, but get param of another nonexistent key
 */
HWTEST_F(DistributedWantV2StringArrayParamTest,
    DistributedScheduleWant_StringArray_0300, Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    std::vector<std::string> defaultValue;
    std::vector<std::string> setValue1 = {"aaa", "2132"};
    std::vector<std::string> setValue2 = {"1*中_aR", "dbdb"};
    std::string key1 = "cc";
    std::string key2 = "aa";
    want_->SetParam(key1, setValue1);
    want_->SetParam(key1, setValue1);
    want_->SetParam(key1, setValue2);
    std::vector<std::string> resultValue = want_->GetStringArrayParam(key2);
    EXPECT_EQ(defaultValue, resultValue);

    want_->SetParam(key1, setValue1);
    resultValue = want_->GetStringArrayParam(key1);
    EXPECT_EQ(setValue1, resultValue);
}

/**
 * @tc.number: DistributedScheduleWant_StringArray_0400
 * @tc.name:  SetParam/GetStringArrayParam
 * @tc.desc: set empty-string key repeatedly, then get param of the key
 */
HWTEST_F(DistributedWantV2StringArrayParamTest,
    DistributedScheduleWant_StringArray_0400, Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    std::vector<std::string> defaultValue;
    std::vector<std::string> setValue = {"aaa", "2132"};
    std::string key1 = "%1uH3";
    std::string key2 = "aa";
    want_->SetParam(key1, setValue);
    want_->SetParam(key1, setValue);
    setValue = {"1*中_aR", "3#$%"};
    want_->SetParam(key1, setValue);
    std::vector<std::string> resultValue = want_->GetStringArrayParam(key1);
    EXPECT_EQ(setValue, resultValue);

    setValue = {"aaa", "2132"};
    want_->SetParam(key1, setValue);
    resultValue = want_->GetStringArrayParam(key2);
    EXPECT_EQ(defaultValue, resultValue);
}

using testStrType = std::tuple<std::string, std::string, std::string, std::string, std::string>;
class DistributedWantV2StringParamTest : public testing::TestWithParam<testStrType> {
public:
    DistributedWantV2StringParamTest()
    {
        want_ = nullptr;
    }
    ~DistributedWantV2StringParamTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<DistributedWantV2> want_;
};

void DistributedWantV2StringParamTest::SetUpTestCase(void)
{}

void DistributedWantV2StringParamTest::TearDownTestCase(void)
{}

void DistributedWantV2StringParamTest::SetUp(void)
{
    want_ = std::make_shared<DistributedWantV2>();
}

void DistributedWantV2StringParamTest::TearDown(void)
{}

/**
 * @tc.number: DistributedScheduleWant_String_0100
 * @tc.name:  SetParam/GetStringParam
 * @tc.desc: Verify when parameter change.
 */
HWTEST_P(DistributedWantV2StringParamTest, DistributedScheduleWant_String_0100, Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    std::string setKey = std::get<0>(GetParam());
    std::string getKey = std::get<1>(GetParam());
    std::string setValue = std::get<2>(GetParam());
    std::string defaultValue = std::get<3>(GetParam());
    std::string result = std::get<4>(GetParam());
    want_->SetParam(setKey, setValue);
    EXPECT_EQ(result, want_->GetStringParam(getKey));
}

INSTANTIATE_TEST_SUITE_P(WantStringParamTestCaseP, DistributedWantV2StringParamTest,
    testing::Values(testStrType("", "aa", "1*中_aR", "", ""), testStrType("", "", "1*中_aR", "", "1*中_aR"),
        testStrType("1*中_aR", "aa", "aaa", "", ""), testStrType("1*中_aR", "1*中_aR", "aaa", "", "aaa")));

/**
 * @tc.number: DistributedScheduleWant_String_0200
 * @tc.name:  SetParam/GetStringParam
 * @tc.desc: get param when WantParam is empty.
 */
HWTEST_F(DistributedWantV2StringParamTest, DistributedScheduleWant_String_0200, Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    std::string defaultStrValue;
    std::string key = "aa";
    EXPECT_EQ(defaultStrValue, want_->GetStringParam(key));
}

/**
 * @tc.number: DistributedScheduleWant_String_0300
 * @tc.name:  SetParam/GetStringParam
 * @tc.desc: set empty-string key repeatedly, but get param of another nonexistent key.
 */
HWTEST_F(DistributedWantV2StringParamTest, DistributedScheduleWant_String_0300, Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    std::string defaultStrValue;
    std::string setValue1 = "aaa";
    std::string setValue2 = "1*中_aR";
    std::string key1 = "dd";
    std::string key2 = "aa";
    want_->SetParam(key1, setValue1);
    want_->SetParam(key1, setValue1);
    want_->SetParam(key1, setValue2);
    EXPECT_EQ(defaultStrValue, want_->GetStringParam(key2));
    want_->SetParam(key1, setValue1);
    EXPECT_EQ(setValue1, want_->GetStringParam(key1));
}

/**
 * @tc.number: DistributedScheduleWant_String_0400
 * @tc.name:  SetParam/GetStringParam
 * @tc.desc: set empty-string key repeatedly, then get param of the key.
 */
HWTEST_F(DistributedWantV2StringParamTest, DistributedScheduleWant_String_0400, Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    std::string key1 = "%1uH3";
    std::string defaultStrValue;
    std::string setValue1 = "aaa";
    std::string setValue2 = "1*中_aR";
    std::string key2 = "aa";
    want_->SetParam(key1, setValue1);
    want_->SetParam(key1, setValue1);
    want_->SetParam(key1, setValue2);
    EXPECT_EQ("1*中_aR", want_->GetStringParam(key1));
    want_->SetParam(key1, setValue1);
    EXPECT_EQ(defaultStrValue, want_->GetStringParam(key2));
}

using testLongType = std::tuple<std::string, std::string, long, long, long>;
class DistributedWantV2LongParamTest : public testing::TestWithParam<testLongType> {
public:
    DistributedWantV2LongParamTest()
    {
        want_ = nullptr;
    }
    ~DistributedWantV2LongParamTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<DistributedWantV2> want_ = nullptr;
};

void DistributedWantV2LongParamTest::SetUpTestCase(void)
{}

void DistributedWantV2LongParamTest::TearDownTestCase(void)
{}

void DistributedWantV2LongParamTest::SetUp(void)
{
    want_ = std::make_shared<DistributedWantV2>();
}

void DistributedWantV2LongParamTest::TearDown(void)
{}

/**
 * @tc.number: DistributedScheduleWant_LongParam_0100
 * @tc.name:  SetParam/GetLongParam
 * @tc.desc: Verify when parameter change.
 */
HWTEST_P(DistributedWantV2LongParamTest, DistributedScheduleWant_LongParam_0100, Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    std::string setKey = std::get<0>(GetParam());
    std::string getKey = std::get<1>(GetParam());
    long setValue = std::get<2>(GetParam());
    long defaultValue = std::get<3>(GetParam());
    long result = std::get<4>(GetParam());
    want_->SetParam(setKey, setValue);
    EXPECT_EQ(result, want_->GetLongParam(getKey, defaultValue));
}

INSTANTIATE_TEST_SUITE_P(WantLongParamTestCaseP, DistributedWantV2LongParamTest,
    testing::Values(testLongType("b1", "b1", -1, 100, -1), testLongType("b3", "b4", 600, 200, 200),
        testLongType("b5", "b5", 50, 6, 50), testLongType("b6", "b7", 1000, 2200, 2200)));

/**
 * @tc.number: DistributedScheduleWant_LongParam_0200
 * @tc.name:  SetParam/GetLongParam
 * @tc.desc:  get param when WantParam is empty.
 */
HWTEST_F(DistributedWantV2LongParamTest, DistributedScheduleWant_LongParam_0200, Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    long defaultValue = 100;
    std::string key = "aa";
    EXPECT_EQ(defaultValue, want_->GetLongParam(key, defaultValue));
}

/**
 * @tc.number: DistributedScheduleWant_LongParam_0300
 * @tc.name:  SetParam/GetLongParam
 * @tc.desc:  set empty-string key repeatedly, but get param of another nonexistent key.
 */
HWTEST_F(DistributedWantV2LongParamTest, DistributedScheduleWant_LongParam_0300, Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    std::string setKey1 = "dd";
    std::string setKey2 = "aa";
    long setValue1 = 1;
    long setValue2 = 5;
    want_->SetParam(setKey1, setValue1);
    want_->SetParam(setKey1, setValue1);
    setValue1 = 2;
    want_->SetParam(setKey1, setValue1);
    setValue1 = 3;
    EXPECT_EQ(setValue1, want_->GetLongParam(setKey2, setValue1));
    setValue1 = 4;
    want_->SetParam(setKey1, setValue1);
    EXPECT_EQ(setValue1, want_->GetLongParam(setKey1, setValue2));
}

/**
 * @tc.number: DistributedScheduleWant_LongParam_0400
 * @tc.name:  SetParam/GetLongParam
 * @tc.desc:  set empty-string key repeatedly, then get param of the key.
 */
HWTEST_F(DistributedWantV2LongParamTest, DistributedScheduleWant_LongParam_0400, Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    std::string setKey1 = "%1uH3";
    std::string setKey2 = "aa";
    long setValue1 = -1;
    long setValue2 = 9;
    want_->SetParam(setKey1, setValue1);
    want_->SetParam(setKey1, setValue1);
    setValue1 = 0;
    want_->SetParam(setKey1, setValue1);
    EXPECT_EQ(setValue1, want_->GetLongParam(setKey1, setValue2));
    setValue1 = 4;
    want_->SetParam(setKey1, setValue1);
    setValue1 = -10;
    EXPECT_EQ(setValue1, want_->GetLongParam(setKey2, setValue1));
}

using testIntType = std::tuple<std::string, std::string, int, int, int>;
class DistributedWantV2IntParamTest : public testing::TestWithParam<testIntType> {
public:
    DistributedWantV2IntParamTest()
    {
        want_ = nullptr;
    }
    ~DistributedWantV2IntParamTest()
    {
        want_ = nullptr;
    }
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<DistributedWantV2> want_;
};

void DistributedWantV2IntParamTest::SetUpTestCase(void)
{}

void DistributedWantV2IntParamTest::TearDownTestCase(void)
{}

void DistributedWantV2IntParamTest::SetUp(void)
{
    want_ = std::make_shared<DistributedWantV2>();
}

void DistributedWantV2IntParamTest::TearDown(void)
{}

/**
 * @tc.number: DistributedScheduleWant_IntParam_0100
 * @tc.name:  SetParam/GetIntParam
 * @tc.desc:  Verify when parameter change.
 */
HWTEST_P(DistributedWantV2IntParamTest, DistributedScheduleWant_IntParam_0100, Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    std::string setKey = std::get<0>(GetParam());
    std::string getKey = std::get<1>(GetParam());
    int setValue = std::get<2>(GetParam());
    int defaultValue = std::get<3>(GetParam());
    int result = std::get<4>(GetParam());
    want_->SetParam(setKey, setValue);
    EXPECT_EQ(result, want_->GetIntParam(getKey, defaultValue));
}

INSTANTIATE_TEST_SUITE_P(WantParametersIntTestCaseP, DistributedWantV2IntParamTest,
    testing::Values(testIntType("", "aa", -1, 100, 100), testIntType("", "", -9, -41, -9),
        testIntType("1*中_aR", "aa", 50, 5, 5), testIntType("1*中_aR", "1*中_aR", -5000, 5000, -5000)));

/**
 * @tc.number: DistributedScheduleWant_IntParam_0200
 * @tc.name:  SetParam/GetIntParam
 * @tc.desc:  Verify when the value is integer.
 */
HWTEST_F(DistributedWantV2IntParamTest, DistributedScheduleWant_IntParam_0200, Function | MediumTest | Level3)
{
    int defaultValue = 200;
    std::string getKey("aa");
    EXPECT_EQ(defaultValue, want_->GetIntParam(getKey, defaultValue));
}

/**
 * @tc.number: DistributedScheduleWant_IntParam_0300
 * @tc.name:  SetParam/GetIntParam
 * @tc.desc:  Verify when the value is integer.
 */
HWTEST_F(DistributedWantV2IntParamTest, DistributedScheduleWant_IntParam_0300, Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    std::string emptyStr("bb");
    int firstValue = 1;
    int secondValue = 2;
    int thirdValue = 4;
    int firstDefaultValue = 3;
    int secondDefaultValue = 5;
    std::string keyStr("aa");
    want_->SetParam(emptyStr, firstValue);
    want_->SetParam(emptyStr, firstValue);
    want_->SetParam(emptyStr, secondValue);
    EXPECT_EQ(firstDefaultValue, want_->GetIntParam(keyStr, firstDefaultValue));
    want_->SetParam(emptyStr, thirdValue);
    EXPECT_EQ(thirdValue, want_->GetIntParam(emptyStr, secondDefaultValue));
}

/**
 * @tc.number: DistributedScheduleWant_IntParam_0400
 * @tc.name:  SetParam/GetIntParam
 * @tc.desc:  Verify when the value is integer.
 */
HWTEST_F(DistributedWantV2IntParamTest, DistributedScheduleWant_IntParam_0400, Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    std::string firstKey("%1uH3");
    int firstValue = -1;
    int secondValue = 0;
    int thirdValue = 4;
    int firstDefaultValue = 9;
    int secondDefaultValue = -10;
    std::string secondKey("aa");
    want_->SetParam(firstKey, firstValue);
    want_->SetParam(firstKey, firstValue);
    want_->SetParam(firstKey, secondValue);
    EXPECT_EQ(secondValue, want_->GetIntParam(firstKey, firstDefaultValue));
    want_->SetParam(firstKey, thirdValue);
    EXPECT_EQ(secondDefaultValue, want_->GetIntParam(secondKey, secondDefaultValue));
}

using testIntArrayType = std::tuple<std::string, std::string, std::vector<int>, std::vector<int>, std::vector<int>>;
class DistributedWantV2IntArrayParamTest : public testing::TestWithParam<testIntArrayType> {
public:
    DistributedWantV2IntArrayParamTest()
    {
        want_ = nullptr;
    }
    ~DistributedWantV2IntArrayParamTest()
    {
        want_ = nullptr;
    }
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<DistributedWantV2> want_;
};

void DistributedWantV2IntArrayParamTest::SetUpTestCase(void)
{}

void DistributedWantV2IntArrayParamTest::TearDownTestCase(void)
{}

void DistributedWantV2IntArrayParamTest::SetUp(void)
{
    want_ = std::make_shared<DistributedWantV2>();
}

void DistributedWantV2IntArrayParamTest::TearDown(void)
{}

/**
 * @tc.number: DistributedScheduleWant_IntArrayParam_0100
 * @tc.name:  SetParam/GetIntArrayParam
 * @tc.desc:  Verify when parameter change.
 */
HWTEST_P(DistributedWantV2IntArrayParamTest,
    DistributedScheduleWant_IntArrayParam_0100, Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    std::string setKey = std::get<0>(GetParam());
    std::string getKey = std::get<1>(GetParam());
    std::vector<int> setValue = std::get<2>(GetParam());
    std::vector<int> defaultValue = std::get<3>(GetParam());
    std::vector<int> result = std::get<4>(GetParam());
    want_->SetParam(setKey, setValue);
    EXPECT_EQ(result, want_->GetIntArrayParam(getKey));
}

INSTANTIATE_TEST_SUITE_P(WantIntArrayParamTestCaseP, DistributedWantV2IntArrayParamTest,
    testing::Values(testIntArrayType("", "aa", {-1, 3, 25, -9}, {}, {}),
        testIntArrayType("", "", {-41, 0, 0, 9}, {}, {-41, 0, 0, 9}),
        testIntArrayType("1*中_aR", "aa", {50, 2, -9}, {}, {}),
        testIntArrayType("1*中_aR", "1*中_aR", {-5000}, {}, {-5000})));

/**
 * @tc.number: DistributedScheduleWant_IntArrayParam_0200
 * @tc.name:  SetParam/GetIntArrayParam
 * @tc.desc:  Verify when the value is integer array.
 */
HWTEST_F(DistributedWantV2IntArrayParamTest,
    DistributedScheduleWant_IntArrayParam_0200, Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    std::vector<int> defaultValue;
    std::string getKey("aa");
    EXPECT_EQ(defaultValue, want_->GetIntArrayParam(getKey));
}

/**
 * @tc.number: DistributedScheduleWant_IntArrayParam_0300
 * @tc.name:  SetParam/GetIntArrayParam
 * @tc.desc:  Verify when the value is integer array.
 */
HWTEST_F(DistributedWantV2IntArrayParamTest,
    DistributedScheduleWant_IntArrayParam_0300, Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    std::string emptyStr("cc");
    std::vector<int> firstValue({1, 4, -9});
    std::vector<int> secondValue({1, 8, -9});
    std::vector<int> thirdValue({1, 4, 9});
    std::string keyStr("aa");
    want_->SetParam(emptyStr, firstValue);
    want_->SetParam(emptyStr, firstValue);
    want_->SetParam(emptyStr, secondValue);
    std::vector<int> defaultValue;
    EXPECT_EQ(defaultValue, want_->GetIntArrayParam(keyStr));
    want_->SetParam(emptyStr, thirdValue);
    EXPECT_EQ(thirdValue, want_->GetIntArrayParam(emptyStr));
}

/**
 * @tc.number: DistributedScheduleWant_IntArrayParam_0400
 * @tc.name:  SetParam/GetIntArrayParam
 * @tc.desc:  Verify when the value is integer array.
 */
HWTEST_F(DistributedWantV2IntArrayParamTest,
    DistributedScheduleWant_IntArrayParam_0400, Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    std::string firstKey("%1uH3");
    std::vector<int> firstValue({-1, -2});
    std::vector<int> secondValue({-1, -2, -1, -2, 0});
    std::vector<int> thirdValue({-1, -2, 100});
    std::string secondKey("aa");
    want_->SetParam(firstKey, firstValue);
    want_->SetParam(firstKey, firstValue);
    want_->SetParam(firstKey, secondValue);
    EXPECT_EQ(secondValue, want_->GetIntArrayParam(firstKey));
    want_->SetParam(firstKey, thirdValue);
    std::vector<int> defaultValue;
    EXPECT_EQ(defaultValue, want_->GetIntArrayParam(secondKey));
}

using testFloatType = std::tuple<std::string, std::string, float, float, float>;
class DistributedWantV2FloatParamTest : public testing::TestWithParam<testFloatType> {
public:
    DistributedWantV2FloatParamTest()
    {
        want_ = nullptr;
    }
    ~DistributedWantV2FloatParamTest()
    {
        want_ = nullptr;
    }

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<DistributedWantV2> want_;
};

void DistributedWantV2FloatParamTest::SetUpTestCase(void)
{}

void DistributedWantV2FloatParamTest::TearDownTestCase(void)
{}

void DistributedWantV2FloatParamTest::SetUp(void)
{
    want_ = std::make_shared<DistributedWantV2>();
}

void DistributedWantV2FloatParamTest::TearDown(void)
{}

/**
 * @tc.number: DistributedScheduleWant_FloatParam_0100
 * @tc.name:  SetParam/GetFloatParam
 * @tc.desc:  Verify when parameter change.
 */
HWTEST_P(DistributedWantV2FloatParamTest, DistributedScheduleWant_FloatParam_0100, Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    std::string setKey = std::get<0>(GetParam());
    std::string getKey = std::get<1>(GetParam());
    float setValue = std::get<2>(GetParam());
    float defaultValue = std::get<3>(GetParam());
    float result = std::get<4>(GetParam());
    want_->SetParam(setKey, setValue);
    EXPECT_EQ(result, want_->GetFloatParam(getKey, defaultValue));
}

INSTANTIATE_TEST_SUITE_P(WantFloatParamTestCaseP, DistributedWantV2FloatParamTest,
    testing::Values(testFloatType("", "aa", -1.1, 100.1, 100.1), testFloatType("", "", -9.1, -41.1, -9.1),
        testFloatType("1*中_aR", "aa", 50.1, 5.1, 5.1), testFloatType("1*中_aR", "1*中_aR", -5000.1, 5000.1, -5000.1)));

/**
 * @tc.number: DistributedScheduleWant_FloatParam_0200
 * @tc.name:  SetParam/GetFloatParam
 * @tc.desc:  get param when WantParam is empty.
 */
HWTEST_F(DistributedWantV2FloatParamTest, DistributedScheduleWant_FloatParam_0200, Function | MediumTest | Level3)
{
    float defaultValue = 100.1;
    std::string key = "aa";
    EXPECT_EQ(defaultValue, want_->GetFloatParam(key, defaultValue));
}

/**
 * @tc.number: DistributedScheduleWant_FloatParam_0300
 * @tc.name:  SetParam/GetFloatParam
 * @tc.desc:  set empty-string key repeatedly, but get param of another nonexistent key.
 */
HWTEST_F(DistributedWantV2FloatParamTest, DistributedScheduleWant_FloatParam_0300, Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    std::string setKey1 = "ee";
    std::string setKey2 = "aa";
    float setValue1 = 1.1;
    float setValue2 = 5.1;
    want_->SetParam(setKey1, setValue1);
    want_->SetParam(setKey1, setValue1);
    setValue1 = 2.1;
    want_->SetParam(setKey1, setValue1);
    setValue1 = 3.1;
    EXPECT_EQ(setValue1, want_->GetFloatParam(setKey2, setValue1));
    setValue1 = 4.1;
    want_->SetParam(setKey1, setValue1);
    EXPECT_EQ(setValue1, want_->GetFloatParam(setKey1, setValue2));
}

/**
 * @tc.number: DistributedScheduleWant_FloatParam_0400
 * @tc.name:  SetParam/GetFloatParam
 * @tc.desc:  set empty-string key repeatedly, but get param of another nonexistent key.
 */
HWTEST_F(DistributedWantV2FloatParamTest, DistributedScheduleWant_FloatParam_0400, Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    std::string setKey1 = "%1uH3";
    std::string setKey2 = "aa";
    float setValue1 = -1.1;
    float setValue2 = 9.1;
    want_->SetParam(setKey1, setValue1);
    want_->SetParam(setKey1, setValue1);
    setValue1 = 0.1;
    want_->SetParam(setKey1, setValue1);
    EXPECT_EQ(setValue1, want_->GetFloatParam(setKey1, setValue2));
    setValue1 = 4.1;
    want_->SetParam(setKey1, setValue1);
    setValue1 = -10.1;
    EXPECT_EQ(setValue1, want_->GetFloatParam(setKey2, setValue1));
}

using testDoubleType = std::tuple<std::string, std::string, double, double, double>;
class DistributedWantV2DoubleParamTest : public testing::TestWithParam<testDoubleType> {
public:
    DistributedWantV2DoubleParamTest()
    {
        want_ = nullptr;
    }
    ~DistributedWantV2DoubleParamTest()
    {
        want_ = nullptr;
    }
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<DistributedWantV2> want_;
};

void DistributedWantV2DoubleParamTest::SetUpTestCase(void)
{}

void DistributedWantV2DoubleParamTest::TearDownTestCase(void)
{}

void DistributedWantV2DoubleParamTest::SetUp(void)
{
    want_ = std::make_shared<DistributedWantV2>();
}

void DistributedWantV2DoubleParamTest::TearDown(void)
{}

/**
 * @tc.number: DistributedScheduleWant_DoubleParam_0100
 * @tc.name:  SetParam/GetDoubleParam
 * @tc.desc:  Verify when parameter change.
 */
HWTEST_P(DistributedWantV2DoubleParamTest, DistributedScheduleWant_DoubleParam_0100, Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    std::string setKey = std::get<0>(GetParam());
    std::string getKey = std::get<1>(GetParam());
    double setValue = std::get<2>(GetParam());
    double defaultValue = std::get<3>(GetParam());
    double result = std::get<4>(GetParam());
    want_->SetParam(setKey, setValue);
    EXPECT_EQ(result, want_->GetDoubleParam(getKey, defaultValue));
}

INSTANTIATE_TEST_SUITE_P(WantDoubleParamTestCaseP, DistributedWantV2DoubleParamTest,
    testing::Values(testDoubleType("", "aa", -1.1, 100.1, 100.1), testDoubleType("", "", -9.1, -41.1, -9.1),
        testDoubleType("1*中_aR", "aa", 50.1, 5.1, 5.1),
        testDoubleType("1*中_aR", "1*中_aR", -5000.1, 5000.1, -5000.1)));

/**
 * @tc.number: DistributedScheduleWant_DoubleParam_0300
 * @tc.name:  SetParam & GetDoubleParam
 * @tc.desc:  set empty-string key repeatedly, but get param of another nonexistent key.
 */
HWTEST_F(DistributedWantV2DoubleParamTest, DistributedScheduleWant_DoubleParam_0300, Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    double defaultValue = 100.1;
    std::string key = "aa";
    EXPECT_EQ(defaultValue, want_->GetDoubleParam(key, defaultValue));
}

/**
 * @tc.number: DistributedScheduleWant_DoubleParam_0400
 * @tc.name:  SetParam & GetDoubleParam
 * @tc.desc:  set empty-string key repeatedly, then get param of the key.
 */
HWTEST_F(DistributedWantV2DoubleParamTest, DistributedScheduleWant_DoubleParam_0400, Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    std::string setKey1 = "ff";
    std::string setKey2 = "aa";
    double setValue1 = 1.1;
    double setValue2 = 5.1;
    want_->SetParam(setKey1, setValue1);
    want_->SetParam(setKey1, setValue1);
    setValue1 = 2.1;
    want_->SetParam(setKey1, setValue1);
    setValue1 = 3.1;
    EXPECT_EQ(setValue1, want_->GetDoubleParam(setKey2, setValue1));
    setValue1 = 4.1;
    want_->SetParam(setKey1, setValue1);
    EXPECT_EQ(setValue1, want_->GetDoubleParam(setKey1, setValue2));
}

/**
 * @tc.number: DistributedScheduleWant_ByteArray_0100
 * @tc.name:  SetParam/GetByteArrayParam
 * @tc.desc:  Verify when parameter change.
 */
HWTEST_F(DistributedWantV2DoubleParamTest, DistributedScheduleWant_ByteArray_0100, Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    std::string setKey1 = "%1uH3";
    std::string setKey2 = "aa";
    double setValue1 = -1.1;
    double setValue2 = 9.1;
    want_->SetParam(setKey1, setValue1);
    want_->SetParam(setKey1, setValue1);
    setValue1 = 0.1;
    want_->SetParam(setKey1, setValue1);
    EXPECT_EQ(setValue1, want_->GetDoubleParam(setKey1, setValue2));
    setValue1 = 4.1;
    want_->SetParam(setKey1, setValue1);
    setValue1 = -10.1;
    EXPECT_EQ(setValue1, want_->GetDoubleParam(setKey2, setValue1));
}

using testByteArrayType =
    std::tuple<std::string, std::string, std::vector<byte>, std::vector<byte>, std::vector<byte>>;
class DistributedWantV2ByteArrayParamTest : public testing::TestWithParam<testByteArrayType> {
public:
    DistributedWantV2ByteArrayParamTest()
    {
        want_ = nullptr;
    }
    ~DistributedWantV2ByteArrayParamTest()
    {
        want_ = nullptr;
    }
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<DistributedWantV2> want_;
};

void DistributedWantV2ByteArrayParamTest::SetUpTestCase(void)
{}

void DistributedWantV2ByteArrayParamTest::TearDownTestCase(void)
{}

void DistributedWantV2ByteArrayParamTest::SetUp(void)
{
    want_ = std::make_shared<DistributedWantV2>();
}

void DistributedWantV2ByteArrayParamTest::TearDown(void)
{}

/**
 * @tc.number: DistributedScheduleWant_ByteArray_0100
 * @tc.name:  SetParam/GetByteArrayParam
 * @tc.desc:  Verify when parameter change.
 */
HWTEST_P(DistributedWantV2ByteArrayParamTest, DistributedScheduleWant_ByteArray_0100, Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    std::string setKey = std::get<0>(GetParam());
    std::string getKey = std::get<1>(GetParam());
    std::vector<byte> setValue = std::get<2>(GetParam());
    std::vector<byte> defaultValue = std::get<3>(GetParam());
    std::vector<byte> result = std::get<4>(GetParam());
    want_->SetParam(setKey, setValue);
    EXPECT_EQ(result, want_->GetByteArrayParam(getKey));
}

INSTANTIATE_TEST_SUITE_P(WantByteArrayParamTestCaseP, DistributedWantV2ByteArrayParamTest,
    testing::Values(testByteArrayType("", "aa", {'*', 'D'}, {}, {}),
        testByteArrayType("", "", {'%', ')'}, {}, {'%', ')'}), testByteArrayType("1*中_aR", "aa", {'R', '.'}, {}, {}),
        testByteArrayType("1*中_aR", "1*中_aR", {'R', 'b'}, {}, {'R', 'b'})));

/**
 * @tc.number: DistributedScheduleWant_ByteArray_0200
 * @tc.name:  SetParam/GetByteArrayParam
 * @tc.desc:  Verify when the value is byte array.
 */
HWTEST_F(DistributedWantV2ByteArrayParamTest, DistributedScheduleWant_ByteArray_0200, Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    std::vector<byte> defaultValue;
    std::string getKey("aa");
    EXPECT_EQ(defaultValue, want_->GetByteArrayParam(getKey));
}

/**
 * @tc.number: DistributedScheduleWant_ByteArray_0300
 * @tc.name:  SetParam/GetByteArrayParam
 * @tc.desc:  Verify when the value is byte array.
 */
HWTEST_F(DistributedWantV2ByteArrayParamTest, DistributedScheduleWant_ByteArray_0300, Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    std::string emptyStr("gg");
    std::vector<byte> firstValue({'a', '2'});
    std::vector<byte> secondValue({'1', 'd'});
    std::vector<byte> thirdValue({'t', '3'});
    std::string keyStr("aa");
    want_->SetParam(emptyStr, firstValue);
    want_->SetParam(emptyStr, firstValue);
    want_->SetParam(emptyStr, secondValue);
    std::vector<byte> defaultValue;
    EXPECT_EQ(defaultValue, want_->GetByteArrayParam(keyStr));
    want_->SetParam(emptyStr, thirdValue);
    EXPECT_EQ(thirdValue, want_->GetByteArrayParam(emptyStr));
}

/**
 * @tc.number: DistributedScheduleWant_ByteArray_0400
 * @tc.name:  SetParam/GetByteArrayParam
 * @tc.desc:  Verify when the value is byte array.
 */
HWTEST_F(DistributedWantV2ByteArrayParamTest, DistributedScheduleWant_ByteArray_0400, Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    std::string firstKey("%1uH3");
    std::vector<byte> firstValue({'a', '2'});
    std::vector<byte> secondValue({'w', '$'});
    std::vector<byte> defaultValue;
    std::string secondKey("aa");
    want_->SetParam(firstKey, firstValue);
    want_->SetParam(firstKey, firstValue);
    want_->SetParam(firstKey, secondValue);
    EXPECT_EQ(secondValue, want_->GetByteArrayParam(firstKey));
    want_->SetParam(firstKey, firstValue);
    EXPECT_EQ(defaultValue, want_->GetByteArrayParam(secondKey));
}

using testBoolType = std::tuple<std::string, std::string, bool, bool, bool>;
class DistributedWantV2BoolParamTest : public testing::TestWithParam<testBoolType> {
public:
    DistributedWantV2BoolParamTest()
    {
        want_ = nullptr;
    }
    ~DistributedWantV2BoolParamTest()
    {
        want_ = nullptr;
    }
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<DistributedWantV2> want_;
};

void DistributedWantV2BoolParamTest::SetUpTestCase(void)
{}

void DistributedWantV2BoolParamTest::TearDownTestCase(void)
{}

void DistributedWantV2BoolParamTest::SetUp(void)
{
    want_ = std::make_shared<DistributedWantV2>();
}

void DistributedWantV2BoolParamTest::TearDown(void)
{}

/**
 * @tc.number: DistributedScheduleWant_BoolParam_0100
 * @tc.name: SetParam/GetBoolParam
 * @tc.desc: Verify when parameter change.
 */
HWTEST_P(DistributedWantV2BoolParamTest, DistributedScheduleWant_BoolParam_0100, Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    std::string setKey = std::get<0>(GetParam());
    std::string getKey = std::get<1>(GetParam());
    bool setValue = std::get<2>(GetParam());
    bool defaultValue = std::get<3>(GetParam());
    bool result = std::get<4>(GetParam());
    want_->SetParam(setKey, setValue);
    EXPECT_EQ(result, want_->GetBoolParam(getKey, defaultValue));
}

INSTANTIATE_TEST_SUITE_P(WantBoolParamTestCaseP, DistributedWantV2BoolParamTest,
    testing::Values(testBoolType("b1", "aa", true, true, true), testBoolType("b1", "aa", true, false, false),
        testBoolType("b2", "b2", true, true, true), testBoolType("b3", "b3", true, false, true),
        testBoolType("123", "123", true, false, true), testBoolType("123", "aa", true, false, false),
        testBoolType("-~*&%￥", "-~*&%￥", true, false, true), testBoolType("-~*&%￥", "aa", true, false, false),
        testBoolType("中文", "中文", true, false, true), testBoolType("中文", "aa", true, false, false),
        testBoolType("_中文ddPEJKJ#(&*~#^%", "_中文ddPEJKJ#(&*~#^%", true, false, true),
        testBoolType("_中文ddPEJKJ#(&*~#^%", "123", true, false, false)));

/**
 * @tc.number: DistributedScheduleWant_BoolParam_0200
 * @tc.name:   SetParam/GetBoolParam
 * @tc.desc: Verify when set twice and get twice.
 */
HWTEST_F(DistributedWantV2BoolParamTest, DistributedScheduleWant_BoolParam_0200, Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    std::string firstKey("_中文ddPEJKJ#(&*~#^%");
    std::string secondKey("key33");
    want_->SetParam(firstKey, true);
    want_->SetParam(secondKey, true);
    EXPECT_EQ(true, want_->GetBoolParam(firstKey, false));
    EXPECT_EQ(true, want_->GetBoolParam(secondKey, false));
}

/**
 * @tc.number: DistributedScheduleWant_BoolParam_0300
 * @tc.name:   SetParam/GetBoolParam
 * @tc.desc: Verify when set 20 times, and get once.
 */
HWTEST_F(DistributedWantV2BoolParamTest, DistributedScheduleWant_BoolParam_0300, Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    std::string keyStr("_中文ddPEJKJ#(&*~#^%");
    for (int i = 0; i < 20; i++) {
        want_->SetParam(keyStr, true);
    }
    EXPECT_EQ(true, want_->GetBoolParam(keyStr, false));
}

/**
 * @tc.number: DistributedScheduleWant_Want_0100
 * @tc.name:   Want() and Want(want)
 * @tc.desc: Verify Want()
 */
HWTEST_F(DistributedWantV2BaseTest, DistributedScheduleWant_Want_0100, Function | MediumTest | Level3)
{
    DistributedWantV2 want;

    EXPECT_EQ((uint)0, want.GetFlags());
    EXPECT_EQ(std::string(""), want.GetAction());

    std::vector<std::string> vect = want.GetEntities();
    EXPECT_EQ((size_t)0, vect.size());
    EXPECT_EQ(std::string(""), want.GetType());

    want.SetFlags(10);
    want.SetAction("system.Action.test");
    want.AddEntity("system.Entity.test");
    want.SetType("system.Type.test");

    DistributedWantV2 want2(want);
    EXPECT_EQ("system.Action.test", want2.GetAction());
    EXPECT_EQ(true, want2.HasEntity("system.Entity.test"));
    EXPECT_EQ("system.Type.test", want2.GetType());
}

/**
 * @tc.number: DistributedScheduleWant_Entity_0100
 * @tc.name:    [AddEntity or RemoveEntity] & HasEntity &CountEntities
 * @tc.desc: Verify [AddEntity or RemoveEntity] & HasEntity &CountEntities
 */
HWTEST_F(DistributedWantV2BaseTest, DistributedScheduleWant_Entity_0100, Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    std::string entity1 = "entity.system.entity1";

    want_->AddEntity(entity1);

    EXPECT_EQ(true, want_->HasEntity(entity1));
    EXPECT_EQ(1, want_->CountEntities());
    want_->RemoveEntity(entity1);
    EXPECT_EQ(false, want_->HasEntity(entity1));
    EXPECT_EQ(0, want_->CountEntities());
    int length = want_->GetEntities().size();
    EXPECT_EQ(0, length);

    std::string entity2 = "entity.system.entity2";

    want_->AddEntity(entity1);
    want_->AddEntity(entity2);

    EXPECT_EQ(true, want_->HasEntity(entity1));
    EXPECT_EQ(2, want_->CountEntities());
    EXPECT_EQ(true, want_->HasEntity(entity2));
    EXPECT_EQ(2, want_->CountEntities());

    want_->RemoveEntity(entity1);
    want_->RemoveEntity(entity2);
    EXPECT_EQ(0, want_->CountEntities());
    int length2 = want_->GetEntities().size();

    EXPECT_EQ(0, length2);
}

/**
 * @tc.number: DistributedScheduleWant_HasParameter_0100
 * @tc.name:    SetParam and HasParameter
 * @tc.desc: Verify HasParameter()
 */
HWTEST_F(DistributedWantV2BaseTest, DistributedScheduleWant_HasParameter_0100, Function | MediumTest | Level3)
{
    ASSERT_NE(want_, nullptr);
    std::vector<std::string> vector;
    std::string key = "system.want.test.key";
    std::string key2 = "system.want.test.key2";

    vector.push_back("system.want.test.content");
    want_->SetParam(key, vector);
    EXPECT_EQ(true, want_->HasParameter(key));

    want_->SetParam(key2, vector);
    EXPECT_EQ(true, want_->HasParameter(key2));
}
/**
 * @tc.number: DistributedScheduleWant_ToString_0100
 * @tc.name:    ToString and FromString
 * @tc.desc: Verify FromString()
 */
HWTEST_F(DistributedWantV2BaseTest, DistributedScheduleWant_ToString_0100, Function | MediumTest | Level3)
{
    std::string deviceId = "deviceId";
    std::string bundleName = "bundleName";
    std::string abilityName ="abilityName";
    std::string uri = "url";
    std::string type = "type";
    unsigned int flags = 1;
    std::string action = "action";
    AAFwk::WantParams parameters;
    std::vector<std::string> entities = {"entity.system.entity1", "entity.system.entity2"};

    std::string keyBool = "key_bool";
    bool valueBool = true;
    parameters.SetParam(keyBool, Boolean::Box(valueBool));

    std::string keyStr = "key_string";
    std::string valueString = "123";
    parameters.SetParam(keyStr, String::Box(valueString));

    std::string keyInt = "key_int";
    int valueInt = 111;
    parameters.SetParam(keyStr, Integer::Box(valueInt));

    std::shared_ptr<DistributedWantV2> want1 = std::make_shared<DistributedWantV2>();
    ASSERT_NE(nullptr, want1);
    want1->SetElementName(deviceId, bundleName, abilityName);
    want1->SetUri(uri);
    want1->SetType(type);
    want1->SetFlags(flags);
    want1->SetAction(action);
    for (auto entity : entities) {
        want1->AddEntity(entity);
    }

    std::string jsonString = want1->ToString();
    DistributedWantV2* newWant = DistributedWantV2::FromString(jsonString);
    ASSERT_NE(nullptr, newWant);
    std::shared_ptr<DistributedWantV2> want2(newWant);

    CompareWant(want1, want2);
}

/*
 * Feature: Array
 * Function: GetLength and GetType
 * SubFunction: NA
 * FunctionPoints: GetLength and GetType
 * EnvConditions: NA
 * CaseDescription: Verify GetLength and GetType method of Array.
 */
HWTEST_F(DistributedWantV2BaseTest, array_test_001, TestSize.Level3)
{
    sptr<Array> arrayObj(new Array(9, g_IID_IInteger));
    ASSERT_NE(arrayObj, nullptr);
    long size = 0;
    arrayObj->GetLength(size);
    EXPECT_EQ(size, 9);
    InterfaceID type;
    arrayObj->GetType(type);
    EXPECT_EQ(type, g_IID_IInteger);
}

/*
 * Feature: Array
 * Function: Get and Set
 * SubFunction: NA
 * FunctionPoints: Get and Set
 * EnvConditions: NA
 * CaseDescription: Verify Get and Set method of Array.
 */
HWTEST_F(DistributedWantV2BaseTest, array_test_002, TestSize.Level3)
{
    sptr<Array> arrayObj(new Array(19, g_IID_IInteger));
    ASSERT_NE(arrayObj, nullptr);
    arrayObj->Set(0, Integer::Box(23));
    sptr<IInterface> valueObj = nullptr;
    arrayObj->Get(0, valueObj);
    EXPECT_TRUE(valueObj != nullptr);
    EXPECT_EQ(Integer::Unbox(IInteger::Query(valueObj)), 23);
    arrayObj->Get(1, valueObj);
    EXPECT_TRUE(valueObj == nullptr);
}

/*
 * Feature: Array
 * Function: ToString
 * SubFunction: NA
 * FunctionPoints: ToString
 * EnvConditions: NA
 * CaseDescription: Verify ToString method of Array.
 */
HWTEST_F(DistributedWantV2BaseTest, array_test_003, TestSize.Level3)
{
    sptr<Array> arrayObj(new Array(5, g_IID_IInteger));
    ASSERT_NE(arrayObj, nullptr);
    arrayObj->Set(0, Integer::Box(2));
    arrayObj->Set(1, Integer::Box(3));
    arrayObj->Set(2, Integer::Box(5));
    arrayObj->Set(3, Integer::Box(7));
    arrayObj->Set(4, Integer::Box(11));
    EXPECT_EQ(arrayObj->ToString(), std::string("I5{2,3,5,7,11}"));
}

/*
 * Feature: Array
 * Function: Parse
 * SubFunction: NA
 * FunctionPoints: Parse
 * EnvConditions: NA
 * CaseDescription: Verify Parse method of Array.
 */
HWTEST_F(DistributedWantV2BaseTest, array_test_004, TestSize.Level3)
{
    sptr<IArray> arrayObj = Array::Parse("I5{2,3,5,7,11}");
    sptr<IInterface> valueObj = nullptr;
    ASSERT_NE(arrayObj, nullptr);
    arrayObj->Get(0, valueObj);
    EXPECT_EQ(Integer::Unbox(IInteger::Query(valueObj)), 2);
    arrayObj->Get(1, valueObj);
    EXPECT_EQ(Integer::Unbox(IInteger::Query(valueObj)), 3);
    arrayObj->Get(2, valueObj);
    EXPECT_EQ(Integer::Unbox(IInteger::Query(valueObj)), 5);
    arrayObj->Get(3, valueObj);
    EXPECT_EQ(Integer::Unbox(IInteger::Query(valueObj)), 7);
    arrayObj->Get(4, valueObj);
    EXPECT_EQ(Integer::Unbox(IInteger::Query(valueObj)), 11);
}

/*
 * Feature: Array
 * Function: Equals
 * SubFunction: NA
 * FunctionPoints: Equals
 * EnvConditions: NA
 * CaseDescription: Verify Equals method of Array.
 */
HWTEST_F(DistributedWantV2BaseTest, array_test_005, TestSize.Level3)
{
    sptr<IArray> arrayObj1 = Array::Parse("I5{2,3,5,7,11}");
    sptr<IArray> arrayObj2 = Array::Parse("I5{2,3,7,7,11}");
    sptr<IArray> arrayObj3 = Array::Parse("I5{2,3,5,7,11}");
    ASSERT_NE(arrayObj1, nullptr);
    ASSERT_NE(arrayObj2, nullptr);
    ASSERT_NE(arrayObj3, nullptr);
    EXPECT_FALSE(Object::Equals(*(arrayObj1.GetRefPtr()), *(arrayObj2.GetRefPtr())));
    EXPECT_TRUE(Object::Equals(*(arrayObj1.GetRefPtr()), *(arrayObj3.GetRefPtr())));
}

/**
 * @tc.number: SetElementModuleName_test_001
 * @tc.name: SetElementModuleName
 * @tc.desc: Test the want function SetElementModuleName.
 * @tc.require: issueI648W6
 */
HWTEST_F(DistributedWantV2BaseTest, SetElementModuleName_test_001, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "SetElementModuleName_test_001 start";

    std::shared_ptr<OHOS::AppExecFwk::ElementName> element = std::make_shared<OHOS::AppExecFwk::ElementName>();
    ASSERT_NE(nullptr, element);
    const char* moduleName = "12345";
    element->SetModuleName(moduleName);

    GTEST_LOG_(INFO) << "SetElementModuleName_test_001 end";
}

/**
 * @tc.number: ParseURI_test_001
 * @tc.name: ParseURI
 * @tc.desc: Test the want function ParseURI.
 * @tc.require: issueI648W6
 */
HWTEST_F(DistributedWantV2BaseTest, ParseURI_test_001, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "ParseURI_test_001 start";

    OHOS::AppExecFwk::ElementName element;
    std::string uri = "#Intent;action;end";
    bool result = element.ParseURI(uri);
    EXPECT_EQ(result, false);

    GTEST_LOG_(INFO) << "ParseURI_test_001 end";
}

/**
 * @tc.number: GetLowerCaseScheme_test_001
 * @tc.name: GetLowerCaseScheme
 * @tc.desc: Test GetLowerCaseScheme.
 * @tc.require: I77HFZ
 */
HWTEST_F(DistributedWantV2BaseTest, GetLowerCaseScheme_test_001, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "GetLowerCaseScheme_test_001 start";
    std::string strUri = "";
    Uri uri(strUri);
    Want want;
    want.GetLowerCaseScheme(uri);
    EXPECT_TRUE(strUri.empty());
    GTEST_LOG_(INFO) << "GetLowerCaseScheme_test_001 end";
}

/**
 * @tc.number: GetLowerCaseScheme_test_002
 * @tc.name: GetLowerCaseScheme
 * @tc.desc: Test GetLowerCaseScheme.
 * @tc.require: I77HFZ
 */
HWTEST_F(DistributedWantV2BaseTest, GetLowerCaseScheme_test_002, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "GetLowerCaseScheme_test_002 start";
    std::string strUri = "#Test;action;end";
    Uri uri(strUri);
    Want want;
    want.GetLowerCaseScheme(uri);
    EXPECT_FALSE(strUri.empty());
    GTEST_LOG_(INFO) << "GetLowerCaseScheme_test_002 end";
}

/**
 * @tc.number: GetLowerCaseScheme_test_003
 * @tc.name: GetLowerCaseScheme
 * @tc.desc: Test GetLowerCaseScheme.
 * @tc.require: I77HFZ
 */
HWTEST_F(DistributedWantV2BaseTest, GetLowerCaseScheme_test_003, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "GetLowerCaseScheme_test_003 start";
    std::string strUri = "#Test;action;end";
    Uri uri(strUri);
    uri.scheme_ = "NOT VALID";
    Want want;
    want.GetLowerCaseScheme(uri);
    EXPECT_FALSE(strUri.empty());
    GTEST_LOG_(INFO) << "GetLowerCaseScheme_test_003 end";
}

/**
 * @tc.number: GetLowerCaseScheme_test_004
 * @tc.name: GetLowerCaseScheme
 * @tc.desc: Test GetLowerCaseScheme.
 */
HWTEST_F(DistributedWantV2BaseTest, GetLowerCaseScheme_test_004, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "GetLowerCaseScheme_test_004 start";
    DistributedWantV2 want;
    Uri lowerCaseUri("http://TEST.COM");
    Uri result = want.GetLowerCaseScheme(lowerCaseUri);
    EXPECT_EQ(result, lowerCaseUri);
    GTEST_LOG_(INFO) << "GetLowerCaseScheme_test_004 end";
}

/**
 * @tc.number: ReadUriFromParcel_test_001
 * @tc.name: ReadUriFromParcel
 * @tc.desc: Test ReadUriFromParcel.
 */
HWTEST_F(DistributedWantV2BaseTest, ReadUriFromParcel_test_001, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "ReadUriFromParcel_test_001 start";
    DistributedWantV2 want;
    Parcel parcel;
    parcel.WriteInt32(DistributedOperation::VALUE_OBJECT);
    bool result = want.ReadUriFromParcel(parcel);
    EXPECT_EQ(result, true);
    GTEST_LOG_(INFO) << "ReadUriFromParcel_test_001 end";
}

/**
 * @tc.number: CanReadFromJson_test_001
 * @tc.name: CanReadFromJson
 * @tc.desc: Test CanReadFromJson.
 */
HWTEST_F(DistributedWantV2BaseTest, CanReadFromJson_test_001, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "CanReadFromJson_test_001 start";
    DistributedWantV2 want;
    nlohmann::json wantJson;
    wantJson["deviceId"] = "device1";
    wantJson["bundleName"] = "bundle1";
    wantJson["abilityName"] = "ability1";
    wantJson["uri"] = "uri1";
    wantJson["type"] = "type1";
    wantJson["flags"] = 1;
    wantJson["action"] = "action1";
    wantJson["parameters"] = "parameters1";
    bool result = want.CanReadFromJson(wantJson);
    EXPECT_EQ(result, false);

    wantJson["deviceId"] = 1;
    wantJson["entities"] = "entities1";
    result = want.CanReadFromJson(wantJson);
    EXPECT_EQ(result, false);

    wantJson["deviceId"] = "device1";
    wantJson["bundleName"] = 1;
    result = want.CanReadFromJson(wantJson);
    EXPECT_EQ(result, false);
    GTEST_LOG_(INFO) << "CanReadFromJson_test_001 end";
}

/**
 * @tc.number: CanReadFromJson_test_002
 * @tc.name: CanReadFromJson
 * @tc.desc: Test CanReadFromJson.
 */
HWTEST_F(DistributedWantV2BaseTest, CanReadFromJson_test_002, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "CanReadFromJson_test_002 start";
    DistributedWantV2 want;
    nlohmann::json wantJson;
    wantJson["deviceId"] = "device1";
    wantJson["bundleName"] = "bundle1";
    wantJson["abilityName"] = 1;
    wantJson["uri"] = "uri1";
    wantJson["type"] = "type1";
    wantJson["flags"] = 1;
    wantJson["action"] = "action1";
    wantJson["parameters"] = "parameters1";
    wantJson["entities"] = "entities1";
    bool result = want.CanReadFromJson(wantJson);
    EXPECT_EQ(result, false);

    wantJson["abilityName"] = "ability1";
    wantJson["uri"] = 1;
    result = want.CanReadFromJson(wantJson);
    EXPECT_EQ(result, false);

    wantJson["uri"] = "uri1";
    wantJson["type"] = 1;
    result = want.CanReadFromJson(wantJson);
    EXPECT_EQ(result, false);
    GTEST_LOG_(INFO) << "CanReadFromJson_test_002 end";
}

/**
 * @tc.number: CanReadFromJson_test_003
 * @tc.name: CanReadFromJson
 * @tc.desc: Test CanReadFromJson.
 */
HWTEST_F(DistributedWantV2BaseTest, CanReadFromJson_test_003, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "CanReadFromJson_test_003 start";
    DistributedWantV2 want;
    nlohmann::json wantJson;
    wantJson["deviceId"] = "device1";
    wantJson["bundleName"] = "bundle1";
    wantJson["abilityName"] = "ability1";
    wantJson["uri"] = "uri1";
    wantJson["type"] = "type1";
    wantJson["flags"] = "flags1";
    wantJson["action"] = "action1";
    wantJson["parameters"] = "parameters1";
    wantJson["entities"] = "entities1";
    bool result = want.CanReadFromJson(wantJson);
    EXPECT_EQ(result, false);

    wantJson["flags"] = 1;
    wantJson["action"] = 1;
    result = want.CanReadFromJson(wantJson);
    EXPECT_EQ(result, false);


    wantJson["action"] = "action1";
    wantJson["parameters"] = 1;
    result = want.CanReadFromJson(wantJson);
    EXPECT_EQ(result, false);
    GTEST_LOG_(INFO) << "CanReadFromJson_test_003 end";
}

/**
 * @tc.number: ReadFromJson_test_001
 * @tc.name: ReadFromJson
 * @tc.desc: Test ReadFromJson.
 */
HWTEST_F(DistributedWantV2BaseTest, ReadFromJson_test_001, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "ReadFromJson_test_001 start";
    DistributedWantV2 want;
    nlohmann::json wantJson;
    wantJson["deviceId"] = "device1";
    wantJson["bundleName"] = "bundle1";
    wantJson["abilityName"] = "ability1";
    wantJson["uri"] = "uri1";
    wantJson["type"] = "type1";
    wantJson["flags"] = 1;
    wantJson["action"] = "action1";
    wantJson["parameters"] = "parameters1";
    wantJson["entities"] = "entities1";
    bool result = want.ReadFromJson(wantJson);
    EXPECT_EQ(result, false);
    GTEST_LOG_(INFO) << "ReadFromJson_test_001 end";
}

/**
 * @tc.number: ReadFromJson_test_002
 * @tc.name: ReadFromJson
 * @tc.desc: Test ReadFromJson.
 */
HWTEST_F(DistributedWantV2BaseTest, ReadFromJson_test_002, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "ReadFromJson_test_002 start";
    DistributedWantV2 want;
    std::string str = "";
    EXPECT_EQ(want.FromString(str), nullptr);
    GTEST_LOG_(INFO) << "ReadFromJson_test_002 end";
}
}  // namespace DistributedSchedule
}  // namespace OHOS
