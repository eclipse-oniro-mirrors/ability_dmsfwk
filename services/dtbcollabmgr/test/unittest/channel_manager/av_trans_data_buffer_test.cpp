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

#include "av_trans_data_buffer_test.h"
#include "dtbcollabmgr_log.h"
#include "test_log.h"
#include "securec.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace DistributedCollab {
namespace {
    static const std::string TAG = "DSchedCollabAVTransDataBufferTest";
    static constexpr size_t CAPACITY_MID = 1024;
    static constexpr size_t OFFSET_ZERO = 0;
    static constexpr size_t OFFSET_MID = 4;
    static constexpr size_t SIZE_ZERO = 0;
    static constexpr uint8_t FILL_BYTE = 0xAB;
}

void AVTransDataBufferTest::SetUpTestCase()
{
    DTEST_LOG << "AVTransDataBufferTest::SetUpTestCase" << std::endl;
}

void AVTransDataBufferTest::TearDownTestCase()
{
    DTEST_LOG << "AVTransDataBufferTest::TearDownTestCase" << std::endl;
}

void AVTransDataBufferTest::SetUp()
{
    DTEST_LOG << "AVTransDataBufferTest::SetUp" << std::endl;
    capacity_ = CAPACITY_MID;
    buffer_ = std::make_unique<AVTransDataBuffer>(capacity_);
}

void AVTransDataBufferTest::TearDown()
{
    DTEST_LOG << "AVTransDataBufferTest::TearDown" << std::endl;
    buffer_.reset();
}

/**
 * @tc.name: Constructor_ValidCapacity_001
 * @tc.desc: construct AVTransDataBuffer with a valid capacity and verify accessors
 * @tc.type: FUNC
 * @tc.level: TestSize.Level1
 */
HWTEST_F(AVTransDataBufferTest, Constructor_ValidCapacity_001, TestSize.Level1)
{
    ASSERT_NE(buffer_, nullptr);
    EXPECT_EQ(buffer_->Capacity(), capacity_);
    EXPECT_EQ(buffer_->Size(), capacity_);
    EXPECT_EQ(buffer_->Offset(), OFFSET_ZERO);
    EXPECT_NE(buffer_->Data(), nullptr);
}

/**
 * @tc.name: Constructor_ZeroCapacity_001
 * @tc.desc: construct AVTransDataBuffer with zero capacity, no memory is allocated
 * @tc.type: FUNC
 * @tc.level: TestSize.Level1
 */
HWTEST_F(AVTransDataBufferTest, Constructor_ZeroCapacity_001, TestSize.Level1)
{
    auto zeroBuffer = std::make_unique<AVTransDataBuffer>(SIZE_ZERO);
    ASSERT_NE(zeroBuffer, nullptr);
    EXPECT_EQ(zeroBuffer->Data(), nullptr);
    EXPECT_EQ(zeroBuffer->Capacity(), SIZE_ZERO);
    EXPECT_EQ(zeroBuffer->Size(), SIZE_ZERO);
    EXPECT_EQ(zeroBuffer->Offset(), OFFSET_ZERO);
}

/**
 * @tc.name: Constructor_ExceedMaxSize_001
 * @tc.desc: construct AVTransDataBuffer with capacity equal to max buffer size, no memory is allocated
 * @tc.type: FUNC
 * @tc.level: TestSize.Level1
 */
HWTEST_F(AVTransDataBufferTest, Constructor_ExceedMaxSize_001, TestSize.Level1)
{
    size_t maxCapacity = AVTransDataBuffer::DSCHED_MAX_BUFFER_SIZE;
    auto overBuffer = std::make_unique<AVTransDataBuffer>(maxCapacity);
    ASSERT_NE(overBuffer, nullptr);
    EXPECT_EQ(overBuffer->Data(), nullptr);
    EXPECT_EQ(overBuffer->Capacity(), SIZE_ZERO);
    EXPECT_EQ(overBuffer->Size(), SIZE_ZERO);
}

/**
 * @tc.name: Constructor_BufferZeroInitialized_001
 * @tc.desc: verify the underlying buffer memory is zero-initialized after construction
 * @tc.type: FUNC
 * @tc.level: TestSize.Level2
 */
HWTEST_F(AVTransDataBufferTest, Constructor_BufferZeroInitialized_001, TestSize.Level2)
{
    uint8_t *data = buffer_->Data();
    ASSERT_NE(data, nullptr);
    for (size_t i = 0; i < buffer_->Size(); ++i) {
        EXPECT_EQ(data[i], 0);
    }
}

/**
 * @tc.name: Data_CanBeWrittenAndRead_001
 * @tc.desc: write bytes through Data() and read them back to verify the pointer is usable
 * @tc.type: FUNC
 * @tc.level: TestSize.Level1
 */
HWTEST_F(AVTransDataBufferTest, Data_CanBeWrittenAndRead_001, TestSize.Level1)
{
    uint8_t *data = buffer_->Data();
    ASSERT_NE(data, nullptr);
    EXPECT_EQ(memset_s(data, buffer_->Size(), FILL_BYTE, buffer_->Size()), EOK);
    for (size_t i = 0; i < buffer_->Size(); ++i) {
        EXPECT_EQ(data[i], FILL_BYTE);
    }
}

/**
 * @tc.name: SetRange_Success_Boundary_001
 * @tc.desc: SetRange where offset plus size exactly equals capacity should succeed
 * @tc.type: FUNC
 * @tc.level: TestSize.Level1
 */
HWTEST_F(AVTransDataBufferTest, SetRange_Success_Boundary_001, TestSize.Level1)
{
    EXPECT_EQ(buffer_->SetRange(OFFSET_ZERO, capacity_), ERR_OK);
    EXPECT_EQ(buffer_->Offset(), OFFSET_ZERO);
    EXPECT_EQ(buffer_->Size(), capacity_);
}

/**
 * @tc.name: SetRange_Success_PartialRange_001
 * @tc.desc: SetRange with a partial range inside capacity updates offset and size
 * @tc.type: FUNC
 * @tc.level: TestSize.Level1
 */
HWTEST_F(AVTransDataBufferTest, SetRange_Success_PartialRange_001, TestSize.Level1)
{
    size_t offset = OFFSET_MID;
    size_t size = capacity_ - offset;
    EXPECT_EQ(buffer_->SetRange(offset, size), ERR_OK);
    EXPECT_EQ(buffer_->Offset(), offset);
    EXPECT_EQ(buffer_->Size(), size);
}

/**
 * @tc.name: SetRange_Fails_OffsetExceedsCapacity_001
 * @tc.desc: SetRange where offset exceeds capacity should fail
 * @tc.type: FUNC
 * @tc.level: TestSize.Level1
 */
HWTEST_F(AVTransDataBufferTest, SetRange_Fails_OffsetExceedsCapacity_001, TestSize.Level1)
{
    size_t offset = capacity_ + 1;
    EXPECT_EQ(buffer_->SetRange(offset, SIZE_ZERO), -1);
    EXPECT_EQ(buffer_->Offset(), OFFSET_ZERO);
    EXPECT_EQ(buffer_->Size(), capacity_);
}

/**
 * @tc.name: SetRange_Fails_SumExceedsCapacity_001
 * @tc.desc: SetRange where offset plus size exceeds capacity should fail
 * @tc.type: FUNC
 * @tc.level: TestSize.Level1
 */
HWTEST_F(AVTransDataBufferTest, SetRange_Fails_SumExceedsCapacity_001, TestSize.Level1)
{
    size_t offset = OFFSET_MID;
    size_t size = capacity_;
    EXPECT_EQ(buffer_->SetRange(offset, size), -1);
    EXPECT_EQ(buffer_->Offset(), OFFSET_ZERO);
    EXPECT_EQ(buffer_->Size(), capacity_);
}

/**
 * @tc.name: Data_ReflectsRangeOffset_001
 * @tc.desc: after SetRange the Data() pointer advances by the range offset
 * @tc.type: FUNC
 * @tc.level: TestSize.Level2
 */
HWTEST_F(AVTransDataBufferTest, Data_ReflectsRangeOffset_001, TestSize.Level2)
{
    uint8_t *base = buffer_->Data();
    ASSERT_NE(base, nullptr);
    size_t offset = OFFSET_MID;
    ASSERT_EQ(buffer_->SetRange(offset, capacity_ - offset), ERR_OK);
    uint8_t *shifted = buffer_->Data();
    ASSERT_NE(shifted, nullptr);
    EXPECT_EQ(shifted, base + offset);
}

/**
 * @tc.name: Data_WriteThroughRange_001
 * @tc.desc: writing through the shifted Data() pointer affects the underlying buffer at the correct offset
 * @tc.type: FUNC
 * @tc.level: TestSize.Level3
 */
HWTEST_F(AVTransDataBufferTest, Data_WriteThroughRange_001, TestSize.Level3)
{
    uint8_t *base = buffer_->Data();
    ASSERT_NE(base, nullptr);
    size_t offset = OFFSET_MID;
    size_t rangeSize = OFFSET_MID;
    ASSERT_EQ(buffer_->SetRange(offset, rangeSize), ERR_OK);
    uint8_t *shifted = buffer_->Data();
    ASSERT_NE(shifted, nullptr);
    EXPECT_EQ(memset_s(shifted, buffer_->Capacity() - offset, FILL_BYTE, rangeSize), EOK);
    for (size_t i = 0; i < rangeSize; ++i) {
        EXPECT_EQ(base[offset + i], FILL_BYTE);
    }
    EXPECT_EQ(base[0], 0);
}
} // namespace DistributedCollab
} // namespace OHOS
