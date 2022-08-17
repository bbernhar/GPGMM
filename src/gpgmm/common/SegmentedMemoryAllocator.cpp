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

#include "gpgmm/common/SegmentedMemoryAllocator.h"

#include "gpgmm/common/EventMessage.h"
#include "gpgmm/utils/Assert.h"
#include "gpgmm/utils/Math.h"
#include "gpgmm/utils/Utils.h"

namespace gpgmm {

    // Helper for FindSegment to find the middle of a linked-list.
    iterator SegmentedMemoryAllocator::GetMiddleSegment(iterator start, iterator end) {
        iterator slow = start;
        iterator fast = ++start;
        while (fast != end) {
            fast++;
            if (fast != end) {
                slow++;
                fast++;
            }
        }

        return slow;
    }

    // Perform a slower, O(n) binary search, over a non-contigious array (linked-list).
    iterator SegmentedMemoryAllocator::FindSegment(iterator start, iterator end, uint64_t size) {
        iterator left = start;
        iterator right = end;

        while (left != right) {
            auto middle = GetMiddleSegment(left, right);
            if (middle == mFreeSegments.end()) {
                return middle;
            }
            if ((*middle).GetMemorySize() == size) {
                return middle;

            } else if ((*middle).GetMemorySize() > size) {
                // Smaller then middle, go left.
                right = middle;

            } else {
                // Larger then middle, go right.
                middle++;
                left = middle;
            }
        }
        return left;
    }

    // MemorySegment

    MemorySegment::MemorySegment(uint64_t memorySize) : LIFOMemoryPool(memorySize) {
    }

    MemorySegment::~MemorySegment() {
        ReleasePool();
    }

    // SegmentedMemoryAllocator

    SegmentedMemoryAllocator::SegmentedMemoryAllocator(
        std::unique_ptr<MemoryAllocator> memoryAllocator,
        uint64_t memoryAlignment)
        : MemoryAllocator(std::move(memoryAllocator)), mMemoryAlignment(memoryAlignment) {
    }

    SegmentedMemoryAllocator::~SegmentedMemoryAllocator() {
        mFreeSegments.clear();
    }

    MemorySegment* SegmentedMemoryAllocator::GetOrCreateFreeSegment(uint64_t memorySize) {
        auto existingFreeSegment =
            FindSegment(mFreeSegments.begin(), mFreeSegments.end(), memorySize);

        // List is empty, append it at end.
        if (mFreeSegments.empty()) {
            mFreeSegments.emplace_back(memorySize);
            return &mFreeSegments.back();
        }

        // Segment already exists, reuse it.
        if ((*existingFreeSegment).GetMemorySize() == memorySize) {
            return &(*existingFreeSegment);
        }

        // Or insert a new segment in sorted order.
        MemorySegment newMemorySegment{memorySize};
        if (memorySize > (*existingFreeSegment).GetMemorySize()) {
            existingFreeSegment++;
            mFreeSegments.insert(existingFreeSegment, std::move(newMemorySegment));
        } else {
            existingFreeSegment--;
            mFreeSegments.insert(existingFreeSegment, std::move(newMemorySegment));
        }

        return &(*existingFreeSegment);
    }

    std::unique_ptr<MemoryAllocation> SegmentedMemoryAllocator::TryAllocateMemory(
        const MemoryAllocationRequest& request) {
        TRACE_EVENT0(TraceEventCategory::Default, "SegmentedMemoryAllocator.TryAllocateMemory");

        std::lock_guard<std::mutex> lock(mMutex);

        GPGMM_INVALID_IF(!ValidateRequest(request));

        const uint64_t memorySize = AlignTo(request.SizeInBytes, mMemoryAlignment);
        MemorySegment* segment = GetOrCreateFreeSegment(memorySize);
        ASSERT(segment != nullptr);

        MemoryAllocation allocation = segment->AcquireFromPool();
        if (allocation == GPGMM_ERROR_INVALID_ALLOCATION) {
            std::unique_ptr<MemoryAllocation> allocationPtr;
            GPGMM_TRY_ASSIGN(GetNextInChain()->TryAllocateMemory(request), allocationPtr);
            allocation = *allocationPtr;
        } else {
            mInfo.FreeMemoryUsage -= allocation.GetSize();
        }

        mInfo.UsedMemoryCount++;
        mInfo.UsedMemoryUsage += allocation.GetSize();

        MemoryBase* memory = allocation.GetMemory();
        ASSERT(memory != nullptr);

        memory->SetPool(segment);

        return std::make_unique<MemoryAllocation>(this, memory, request.SizeInBytes);
    }

    void SegmentedMemoryAllocator::DeallocateMemory(std::unique_ptr<MemoryAllocation> allocation) {
        TRACE_EVENT0(TraceEventCategory::Default, "SegmentedMemoryAllocator.DeallocateMemory");

        std::lock_guard<std::mutex> lock(mMutex);

        ASSERT(allocation != nullptr);

        mInfo.FreeMemoryUsage += allocation->GetSize();
        mInfo.UsedMemoryCount--;
        mInfo.UsedMemoryUsage -= allocation->GetSize();

        MemoryBase* memory = allocation->GetMemory();
        ASSERT(memory != nullptr);

        MemoryPool* pool = memory->GetPool();
        ASSERT(pool != nullptr);

        pool->ReturnToPool(
            MemoryAllocation(GetNextInChain(), memory, allocation->GetRequestSize()));
    }

    uint64_t SegmentedMemoryAllocator::ReleaseMemory(uint64_t bytesToRelease) {
        std::lock_guard<std::mutex> lock(mMutex);

        uint64_t totalBytesReleased = 0;
        for (auto& segment : mFreeSegments) {
            const uint64_t bytesReleasedPerSegment = segment.ReleasePool(bytesToRelease);
            bytesToRelease -= bytesReleasedPerSegment;
            mInfo.FreeMemoryUsage -= bytesReleasedPerSegment;
            totalBytesReleased += bytesReleasedPerSegment;

            if (totalBytesReleased >= bytesToRelease) {
                break;
            }
        }

        return totalBytesReleased;
    }

    uint64_t SegmentedMemoryAllocator::GetMemoryAlignment() const {
        return mMemoryAlignment;
    }

    uint64_t SegmentedMemoryAllocator::GetSegmentSizeForTesting() const {
        std::lock_guard<std::mutex> lock(mMutex);
        return std::distance(mFreeSegments.begin(), mFreeSegments.end());
    }

    const char* SegmentedMemoryAllocator::GetTypename() const {
        return "SegmentedMemoryAllocator";
    }

}  // namespace gpgmm
