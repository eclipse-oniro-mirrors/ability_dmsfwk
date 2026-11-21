/*
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

#include "distributed_operation_builder_test.h"

#include "test_log.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace DistributedSchedule {
namespace {
const std::string ABILITY_NAME = "MainAbility";
const std::string BUNDLE_NAME = "com.example.demo";
const std::string DEVICE_ID = "192.168.1.100";
const std::string ACTION = "ohos.want.action.viewData";
const std::string URI_STR = "https://www.example.com/page";
const unsigned int FLAGS_VALUE = 0x10;
}

void DistributedOperationBuilderTest::SetUpTestCase()
{
    DTEST_LOG << "DistributedOperationBuilderTest::SetUpTestCase" << std::endl;
}

void DistributedOperationBuilderTest::TearDownTestCase()
{
    DTEST_LOG << "DistributedOperationBuilderTest::TearDownTestCase" << std::endl;
}

void DistributedOperationBuilderTest::SetUp()
{
    DTEST_LOG << "DistributedOperationBuilderTest::SetUp" << std::endl;
    builder_ = std::make_shared<DistributedOperationBuilder>();
}

void DistributedOperationBuilderTest::TearDown()
{
    DTEST_LOG << "DistributedOperationBuilderTest::TearDown" << std::endl;
    builder_.reset();
}

/**
 * @tc.name: testBuild_Default_001
 * @tc.desc: build() with no setters returns a non-null op with empty/default fields
 * @tc.type: FUNC
 * @tc.require: I7XVTZ
 */
HWTEST_F(DistributedOperationBuilderTest, testBuild_Default_001, TestSize.Level3)
{
    DTEST_LOG << "DistributedOperationBuilderTest testBuild_Default_001 begin" << std::endl;
    ASSERT_NE(builder_, nullptr);
    auto op = builder_->build();
    ASSERT_NE(op, nullptr);
    EXPECT_EQ(op->GetAbilityName(), std::string(""));
    EXPECT_EQ(op->GetBundleName(), std::string(""));
    EXPECT_EQ(op->GetDeviceId(), std::string(""));
    EXPECT_EQ(op->GetAction(), std::string(""));
    EXPECT_EQ(op->GetEntities().size(), static_cast<size_t>(0));
    EXPECT_EQ(op->GetFlags(), static_cast<unsigned int>(0));
    DTEST_LOG << "DistributedOperationBuilderTest testBuild_Default_001 end" << std::endl;
}

/**
 * @tc.name: testWithAbilityName_001
 * @tc.desc: WithAbilityName sets the ability name on the built operation
 * @tc.type: FUNC
 * @tc.require: I7XVTZ
 */
HWTEST_F(DistributedOperationBuilderTest, testWithAbilityName_001, TestSize.Level3)
{
    DTEST_LOG << "DistributedOperationBuilderTest testWithAbilityName_001 begin" << std::endl;
    ASSERT_NE(builder_, nullptr);
    builder_->WithAbilityName(ABILITY_NAME);
    auto op = builder_->build();
    ASSERT_NE(op, nullptr);
    EXPECT_EQ(op->GetAbilityName(), ABILITY_NAME);
    DTEST_LOG << "DistributedOperationBuilderTest testWithAbilityName_001 end" << std::endl;
}

/**
 * @tc.name: testWithBundleName_001
 * @tc.desc: WithBundleName sets the bundle name on the built operation
 * @tc.type: FUNC
 * @tc.require: I7XVTZ
 */
HWTEST_F(DistributedOperationBuilderTest, testWithBundleName_001, TestSize.Level3)
{
    DTEST_LOG << "DistributedOperationBuilderTest testWithBundleName_001 begin" << std::endl;
    ASSERT_NE(builder_, nullptr);
    builder_->WithBundleName(BUNDLE_NAME);
    auto op = builder_->build();
    ASSERT_NE(op, nullptr);
    EXPECT_EQ(op->GetBundleName(), BUNDLE_NAME);
    DTEST_LOG << "DistributedOperationBuilderTest testWithBundleName_001 end" << std::endl;
}

/**
 * @tc.name: testWithDeviceId_001
 * @tc.desc: WithDeviceId sets the device id on the built operation
 * @tc.type: FUNC
 * @tc.require: I7XVTZ
 */
HWTEST_F(DistributedOperationBuilderTest, testWithDeviceId_001, TestSize.Level3)
{
    DTEST_LOG << "DistributedOperationBuilderTest testWithDeviceId_001 begin" << std::endl;
    ASSERT_NE(builder_, nullptr);
    builder_->WithDeviceId(DEVICE_ID);
    auto op = builder_->build();
    ASSERT_NE(op, nullptr);
    EXPECT_EQ(op->GetDeviceId(), DEVICE_ID);
    DTEST_LOG << "DistributedOperationBuilderTest testWithDeviceId_001 end" << std::endl;
}

/**
 * @tc.name: testWithAction_001
 * @tc.desc: WithAction sets the action on the built operation
 * @tc.type: FUNC
 * @tc.require: I7XVTZ
 */
HWTEST_F(DistributedOperationBuilderTest, testWithAction_001, TestSize.Level3)
{
    DTEST_LOG << "DistributedOperationBuilderTest testWithAction_001 begin" << std::endl;
    ASSERT_NE(builder_, nullptr);
    builder_->WithAction(ACTION);
    auto op = builder_->build();
    ASSERT_NE(op, nullptr);
    EXPECT_EQ(op->GetAction(), ACTION);
    DTEST_LOG << "DistributedOperationBuilderTest testWithAction_001 end" << std::endl;
}

/**
 * @tc.name: testWithEntities_001
 * @tc.desc: WithEntities sets the entity vector on the built operation
 * @tc.type: FUNC
 * @tc.require: I7XVTZ
 */
HWTEST_F(DistributedOperationBuilderTest, testWithEntities_001, TestSize.Level3)
{
    DTEST_LOG << "DistributedOperationBuilderTest testWithEntities_001 begin" << std::endl;
    ASSERT_NE(builder_, nullptr);
    std::vector<std::string> entities = { "entity.home", "entity.browsable" };
    builder_->WithEntities(entities);
    auto op = builder_->build();
    ASSERT_NE(op, nullptr);
    EXPECT_EQ(op->GetEntities(), entities);
    EXPECT_EQ(op->GetEntities().size(), static_cast<size_t>(2));
    DTEST_LOG << "DistributedOperationBuilderTest testWithEntities_001 end" << std::endl;
}

/**
 * @tc.name: testWithEntities_Empty_001
 * @tc.desc: WithEntities with an empty vector leaves the built op with zero entities
 * @tc.type: FUNC
 * @tc.require: I7XVTZ
 */
HWTEST_F(DistributedOperationBuilderTest, testWithEntities_Empty_001, TestSize.Level3)
{
    DTEST_LOG << "DistributedOperationBuilderTest testWithEntities_Empty_001 begin" << std::endl;
    ASSERT_NE(builder_, nullptr);
    std::vector<std::string> empty;
    builder_->WithEntities(empty);
    auto op = builder_->build();
    ASSERT_NE(op, nullptr);
    EXPECT_EQ(op->GetEntities().size(), static_cast<size_t>(0));
    DTEST_LOG << "DistributedOperationBuilderTest testWithEntities_Empty_001 end" << std::endl;
}

/**
 * @tc.name: testWithFlags_001
 * @tc.desc: WithFlags sets the flags on the built operation
 * @tc.type: FUNC
 * @tc.require: I7XVTZ
 */
HWTEST_F(DistributedOperationBuilderTest, testWithFlags_001, TestSize.Level3)
{
    DTEST_LOG << "DistributedOperationBuilderTest testWithFlags_001 begin" << std::endl;
    ASSERT_NE(builder_, nullptr);
    builder_->WithFlags(FLAGS_VALUE);
    auto op = builder_->build();
    ASSERT_NE(op, nullptr);
    EXPECT_EQ(op->GetFlags(), FLAGS_VALUE);
    DTEST_LOG << "DistributedOperationBuilderTest testWithFlags_001 end" << std::endl;
}

/**
 * @tc.name: testWithUri_001
 * @tc.desc: WithUri sets the uri on the built operation
 * @tc.type: FUNC
 * @tc.require: I7XVTZ
 */
HWTEST_F(DistributedOperationBuilderTest, testWithUri_001, TestSize.Level3)
{
    DTEST_LOG << "DistributedOperationBuilderTest testWithUri_001 begin" << std::endl;
    ASSERT_NE(builder_, nullptr);
    Uri uri(URI_STR.c_str());
    builder_->WithUri(uri);
    auto op = builder_->build();
    ASSERT_NE(op, nullptr);
    EXPECT_EQ(op->GetUri().ToString(), URI_STR);
    DTEST_LOG << "DistributedOperationBuilderTest testWithUri_001 end" << std::endl;
}

/**
 * @tc.name: testChainedSetters_001
 * @tc.desc: all With* setters chained together populate every field of the built op
 * @tc.type: FUNC
 * @tc.require: I7XVTZ
 */
HWTEST_F(DistributedOperationBuilderTest, testChainedSetters_001, TestSize.Level3)
{
    DTEST_LOG << "DistributedOperationBuilderTest testChainedSetters_001 begin" << std::endl;
    ASSERT_NE(builder_, nullptr);
    std::vector<std::string> entities = { "entity.one", "entity.two", "entity.three" };
    Uri uri(URI_STR.c_str());
    auto op = builder_->WithAbilityName(ABILITY_NAME)
                          .WithBundleName(BUNDLE_NAME)
                          .WithDeviceId(DEVICE_ID)
                          .WithAction(ACTION)
                          .WithEntities(entities)
                          .WithFlags(FLAGS_VALUE)
                          .WithUri(uri)
                          .build();
    ASSERT_NE(op, nullptr);
    EXPECT_EQ(op->GetAbilityName(), ABILITY_NAME);
    EXPECT_EQ(op->GetBundleName(), BUNDLE_NAME);
    EXPECT_EQ(op->GetDeviceId(), DEVICE_ID);
    EXPECT_EQ(op->GetAction(), ACTION);
    EXPECT_EQ(op->GetEntities(), entities);
    EXPECT_EQ(op->GetFlags(), FLAGS_VALUE);
    EXPECT_EQ(op->GetUri().ToString(), URI_STR);
    DTEST_LOG << "DistributedOperationBuilderTest testChainedSetters_001 end" << std::endl;
}

/**
 * @tc.name: testBuild_ReturnsNonNull_001
 * @tc.desc: build() always returns a non-null shared_ptr across repeated calls
 * @tc.type: FUNC
 * @tc.require: I7XVTZ
 */
HWTEST_F(DistributedOperationBuilderTest, testBuild_ReturnsNonNull_001, TestSize.Level3)
{
    DTEST_LOG << "DistributedOperationBuilderTest testBuild_ReturnsNonNull_001 begin" << std::endl;
    ASSERT_NE(builder_, nullptr);
    for (int i = 0; i < 5; i++) {
        auto op = builder_->build();
        EXPECT_NE(op, nullptr);
    }
    DTEST_LOG << "DistributedOperationBuilderTest testBuild_ReturnsNonNull_001 end" << std::endl;
}

/**
 * @tc.name: testSetterOverwrite_001
 * @tc.desc: calling a With* setter twice keeps only the last value
 * @tc.type: FUNC
 * @tc.require: I7XVTZ
 */
HWTEST_F(DistributedOperationBuilderTest, testSetterOverwrite_001, TestSize.Level3)
{
    DTEST_LOG << "DistributedOperationBuilderTest testSetterOverwrite_001 begin" << std::endl;
    ASSERT_NE(builder_, nullptr);
    builder_->WithAbilityName("first");
    builder_->WithAbilityName("second");
    auto op = builder_->build();
    ASSERT_NE(op, nullptr);
    EXPECT_EQ(op->GetAbilityName(), std::string("second"));
    DTEST_LOG << "DistributedOperationBuilderTest testSetterOverwrite_001 end" << std::endl;
}

/**
 * @tc.name: testMultipleBuilders_001
 * @tc.desc: two independent builders produce operations with their own field values
 * @tc.type: FUNC
 * @tc.require: I7XVTZ
 */
HWTEST_F(DistributedOperationBuilderTest, testMultipleBuilders_001, TestSize.Level3)
{
    DTEST_LOG << "DistributedOperationBuilderTest testMultipleBuilders_001 begin" << std::endl;
    auto builderA = std::make_shared<DistributedOperationBuilder>();
    auto builderB = std::make_shared<DistributedOperationBuilder>();
    ASSERT_NE(builderA, nullptr);
    ASSERT_NE(builderB, nullptr);
    builderA->WithAbilityName("abilityA");
    builderB->WithAbilityName("abilityB");
    auto opA = builderA->build();
    auto opB = builderB->build();
    ASSERT_NE(opA, nullptr);
    ASSERT_NE(opB, nullptr);
    EXPECT_EQ(opA->GetAbilityName(), std::string("abilityA"));
    EXPECT_EQ(opB->GetAbilityName(), std::string("abilityB"));
    DTEST_LOG << "DistributedOperationBuilderTest testMultipleBuilders_001 end" << std::endl;
}

/**
 * @tc.name: testWithEntities_Single_001
 * @tc.desc: WithEntities with a single-element vector is preserved
 * @tc.type: FUNC
 * @tc.require: I7XVTZ
 */
HWTEST_F(DistributedOperationBuilderTest, testWithEntities_Single_001, TestSize.Level3)
{
    DTEST_LOG << "DistributedOperationBuilderTest testWithEntities_Single_001 begin" << std::endl;
    ASSERT_NE(builder_, nullptr);
    std::vector<std::string> entities = { "entity.only" };
    builder_->WithEntities(entities);
    auto op = builder_->build();
    ASSERT_NE(op, nullptr);
    EXPECT_EQ(op->GetEntities().size(), static_cast<size_t>(1));
    EXPECT_EQ(op->GetEntities()[0], std::string("entity.only"));
    DTEST_LOG << "DistributedOperationBuilderTest testWithEntities_Single_001 end" << std::endl;
}

/**
 * @tc.name: testRebuildSameBuilder_001
 * @tc.desc: building twice from the same builder yields equivalent operations
 * @tc.type: FUNC
 * @tc.require: I7XVTZ
 */
HWTEST_F(DistributedOperationBuilderTest, testRebuildSameBuilder_001, TestSize.Level3)
{
    DTEST_LOG << "DistributedOperationBuilderTest testRebuildSameBuilder_001 begin" << std::endl;
    ASSERT_NE(builder_, nullptr);
    builder_->WithAbilityName(ABILITY_NAME).WithBundleName(BUNDLE_NAME).WithFlags(FLAGS_VALUE);
    auto first = builder_->build();
    auto second = builder_->build();
    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);
    EXPECT_EQ(first->GetAbilityName(), second->GetAbilityName());
    EXPECT_EQ(first->GetBundleName(), second->GetBundleName());
    EXPECT_EQ(first->GetFlags(), second->GetFlags());
    DTEST_LOG << "DistributedOperationBuilderTest testRebuildSameBuilder_001 end" << std::endl;
}
} // namespace DistributedSchedule
} // namespace OHOS
