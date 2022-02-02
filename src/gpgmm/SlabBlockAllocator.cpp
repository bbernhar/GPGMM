// Copyright 2021 The GPGMM Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "gpgmm/SlabBlockAllocator.h"

#include "gpgmm/common/Assert.h"
#include "gpgmm/common/Math.h"

namespace gpgmm {

    SlabBlockAllocator::SlabBlockAllocator(uint64_t blockCount, uint64_t blockSize)
        : mBlockCount(blockCount), mBlockSize(blockSize), mNextFreeBlockIndex(0) {
        SlabBlock* head = new SlabBlock{};
        head->Offset = 0;
        head->Size = blockSize;
        mFreeList.head = head;
    }

    SlabBlockAllocator::~SlabBlockAllocator() {
        SlabBlock* head = mFreeList.head;
        while (head != nullptr) {
            ASSERT(head != nullptr);
            SlabBlock* next = head->pNext;
            delete head;
            head = next;
        }
    }

    Block* SlabBlockAllocator::AllocateBlock(uint64_t size, uint64_t alignment) {
        if (size == 0 || size > mBlockSize) {
            return nullptr;
        }

        // Offset must be equal to a multiple of |mBlockSize|.
        if (!IsAligned(mBlockSize, alignment)) {
            return nullptr;
        }

        // Pop off HEAD in the free-list.
        SlabBlock* head = mFreeList.head;
        if (head == nullptr) {
            mFreeList.head = nullptr;
        } else {
            mFreeList.head = mFreeList.head->pNext;
        }

        // And push new block at HEAD if not full.
        if (mFreeList.head == nullptr && mNextFreeBlockIndex + 1 < mBlockCount) {
            mFreeList.head = new SlabBlock{};
            mFreeList.head->Offset = ++mNextFreeBlockIndex * mBlockSize;
            mFreeList.head->Size = mBlockSize;
        }

        return head;
    }

    void SlabBlockAllocator::DeallocateBlock(Block* block) {
        ASSERT(block != nullptr);

        SlabBlock* currBlock = static_cast<SlabBlock*>(block);

        // Push block to HEAD
        SlabBlock* head = mFreeList.head;
        mFreeList.head = currBlock;
        currBlock->pNext = head;
    }

    bool SlabBlockAllocator::IsFull() const {
        return mFreeList.head == nullptr;
    }

}  // namespace gpgmm
