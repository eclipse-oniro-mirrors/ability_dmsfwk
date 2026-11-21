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

#include "dms_hitrace_chain_test.h"

#include "test_log.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace DistributedSchedule {
namespace {
const std::string EMPTY_NAME = "";
const std::string NORMAL_NAME = "DmsStartRemoteAbility";
const std::string LONG_NAME =
    "A_very_long_distributed_schedule_trace_chain_name_used_for_boundary_validation_1234567890";
}

void DmsHiTraceChainTest::SetUpTestCase()
{
    DTEST_LOG << "DmsHiTraceChainTest::SetUpTestCase" << std::endl;
}

void DmsHiTraceChainTest::TearDownTestCase()
{
    DTEST_LOG << "DmsHiTraceChainTest::TearDownTestCase" << std::endl;
}

void DmsHiTraceChainTest::SetUp()
{
    DTEST_LOG << "DmsHiTraceChainTest::SetUp" << std::endl;
}

void DmsHiTraceChainTest::TearDown()
{
    DTEST_LOG << "DmsHiTraceChainTest::TearDown" << std::endl;
}

/**
 * @tc.name: testConstructor_EmptyName_001
 * @tc.desc: constructing DmsHiTraceChain with an empty name does not crash and starts a chain
 * @tc.type: FUNC
 * @tc.require: I621C1
 */
HWTEST_F(DmsHiTraceChainTest, testConstructor_EmptyName_001, TestSize.Level3)
{
    DTEST_LOG << "DmsHiTraceChainTest testConstructor_EmptyName_001 begin" << std::endl;
    EXPECT_NO_FATAL_FAILURE({
        DmsHiTraceChain chain(EMPTY_NAME);
        EXPECT_TRUE(chain.hiTraceId.IsValid());
    });
    DTEST_LOG << "DmsHiTraceChainTest testConstructor_EmptyName_001 end" << std::endl;
}

/**
 * @tc.name: testConstructor_NormalName_001
 * @tc.desc: constructing DmsHiTraceChain with a normal name starts a valid chain
 * @tc.type: FUNC
 * @tc.require: I621C1
 */
HWTEST_F(DmsHiTraceChainTest, testConstructor_NormalName_001, TestSize.Level3)
{
    DTEST_LOG << "DmsHiTraceChainTest testConstructor_NormalName_001 begin" << std::endl;
    EXPECT_NO_FATAL_FAILURE({
        DmsHiTraceChain chain(NORMAL_NAME);
        EXPECT_TRUE(chain.hiTraceId.IsValid());
    });
    DTEST_LOG << "DmsHiTraceChainTest testConstructor_NormalName_001 end" << std::endl;
}

/**
 * @tc.name: testConstructor_LongName_001
 * @tc.desc: constructing DmsHiTraceChain with a long name does not crash
 * @tc.type: FUNC
 * @tc.require: I621C1
 */
HWTEST_F(DmsHiTraceChainTest, testConstructor_LongName_001, TestSize.Level3)
{
    DTEST_LOG << "DmsHiTraceChainTest testConstructor_LongName_001 begin" << std::endl;
    EXPECT_NO_FATAL_FAILURE({
        DmsHiTraceChain chain(LONG_NAME);
        EXPECT_TRUE(chain.hiTraceId.IsValid());
    });
    DTEST_LOG << "DmsHiTraceChainTest testConstructor_LongName_001 end" << std::endl;
}

/**
 * @tc.name: testDestructor_EndsChain_001
 * @tc.desc: destructor ends the chain without crashing after scope exit
 * @tc.type: FUNC
 * @tc.require: I621C1
 */
HWTEST_F(DmsHiTraceChainTest, testDestructor_EndsChain_001, TestSize.Level3)
{
    DTEST_LOG << "DmsHiTraceChainTest testDestructor_EndsChain_001 begin" << std::endl;
    EXPECT_NO_FATAL_FAILURE({
        {
            DmsHiTraceChain chain(NORMAL_NAME);
            EXPECT_TRUE(chain.hiTraceId.IsValid());
        }
    });
    DTEST_LOG << "DmsHiTraceChainTest testDestructor_EndsChain_001 end" << std::endl;
}

/**
 * @tc.name: testNestedConstruction_001
 * @tc.desc: nested DmsHiTraceChain does not crash; Begin only works the first time so inner id is invalid
 * @tc.type: FUNC
 * @tc.require: I621C1
 */
HWTEST_F(DmsHiTraceChainTest, testNestedConstruction_001, TestSize.Level3)
{
    DTEST_LOG << "DmsHiTraceChainTest testNestedConstruction_001 begin" << std::endl;
    EXPECT_NO_FATAL_FAILURE({
        DmsHiTraceChain outer("outer_chain");
        EXPECT_TRUE(outer.hiTraceId.IsValid());
        {
            DmsHiTraceChain inner("inner_chain");
            EXPECT_FALSE(inner.hiTraceId.IsValid());
        }
        EXPECT_TRUE(outer.hiTraceId.IsValid());
    });
    DTEST_LOG << "DmsHiTraceChainTest testNestedConstruction_001 end" << std::endl;
}

/**
 * @tc.name: testRepeatedConstructDestruct_001
 * @tc.desc: repeatedly constructing and destroying chains does not crash
 * @tc.type: FUNC
 * @tc.require: I621C1
 */
HWTEST_F(DmsHiTraceChainTest, testRepeatedConstructDestruct_001, TestSize.Level3)
{
    DTEST_LOG << "DmsHiTraceChainTest testRepeatedConstructDestruct_001 begin" << std::endl;
    EXPECT_NO_FATAL_FAILURE({
        for (int i = 0; i < 20; i++) {
            DmsHiTraceChain chain("loop_chain_" + std::to_string(i));
            EXPECT_TRUE(chain.hiTraceId.IsValid());
        }
    });
    DTEST_LOG << "DmsHiTraceChainTest testRepeatedConstructDestruct_001 end" << std::endl;
}

/**
 * @tc.name: testHeapAllocated_001
 * @tc.desc: heap-allocated DmsHiTraceChain starts a chain and destruction ends it
 * @tc.type: FUNC
 * @tc.require: I621C1
 */
HWTEST_F(DmsHiTraceChainTest, testHeapAllocated_001, TestSize.Level3)
{
    DTEST_LOG << "DmsHiTraceChainTest testHeapAllocated_001 begin" << std::endl;
    EXPECT_NO_FATAL_FAILURE({
        DmsHiTraceChain* chain = new DmsHiTraceChain(NORMAL_NAME);
        ASSERT_NE(chain, nullptr);
        EXPECT_TRUE(chain->hiTraceId.IsValid());
        delete chain;
    });
    DTEST_LOG << "DmsHiTraceChainTest testHeapAllocated_001 end" << std::endl;
}

/**
 * @tc.name: testSharedPtrManaged_001
 * @tc.desc: DmsHiTraceChain managed by shared_ptr works correctly
 * @tc.type: FUNC
 * @tc.require: I621C1
 */
HWTEST_F(DmsHiTraceChainTest, testSharedPtrManaged_001, TestSize.Level3)
{
    DTEST_LOG << "DmsHiTraceChainTest testSharedPtrManaged_001 begin" << std::endl;
    EXPECT_NO_FATAL_FAILURE({
        auto chain = std::make_shared<DmsHiTraceChain>(NORMAL_NAME);
        ASSERT_NE(chain, nullptr);
        EXPECT_TRUE(chain->hiTraceId.IsValid());
    });
    DTEST_LOG << "DmsHiTraceChainTest testSharedPtrManaged_001 end" << std::endl;
}

/**
 * @tc.name: testMultipleChains_001
 * @tc.desc: concurrent DmsHiTraceChain objects do not crash; only the first gets a valid id
 * @tc.type: FUNC
 * @tc.require: I621C1
 */
HWTEST_F(DmsHiTraceChainTest, testMultipleChains_001, TestSize.Level3)
{
    DTEST_LOG << "DmsHiTraceChainTest testMultipleChains_001 begin" << std::endl;
    EXPECT_NO_FATAL_FAILURE({
        DmsHiTraceChain a("chain_a");
        DmsHiTraceChain b("chain_b");
        DmsHiTraceChain c("chain_c");
        EXPECT_TRUE(a.hiTraceId.IsValid());
        EXPECT_FALSE(b.hiTraceId.IsValid());
        EXPECT_FALSE(c.hiTraceId.IsValid());
    });
    DTEST_LOG << "DmsHiTraceChainTest testMultipleChains_001 end" << std::endl;
}

/**
 * @tc.name: testUniquePtrManaged_001
 * @tc.desc: DmsHiTraceChain managed by unique_ptr works correctly
 * @tc.type: FUNC
 * @tc.require: I621C1
 */
HWTEST_F(DmsHiTraceChainTest, testUniquePtrManaged_001, TestSize.Level3)
{
    DTEST_LOG << "DmsHiTraceChainTest testUniquePtrManaged_001 begin" << std::endl;
    EXPECT_NO_FATAL_FAILURE({
        auto chain = std::make_unique<DmsHiTraceChain>(NORMAL_NAME);
        ASSERT_NE(chain, nullptr);
        EXPECT_TRUE(chain->hiTraceId.IsValid());
    });
    DTEST_LOG << "DmsHiTraceChainTest testUniquePtrManaged_001 end" << std::endl;
}
} // namespace DistributedSchedule
} // namespace OHOS
