// Copyright 2019 The Dawn Authors
// Copyright 2022 The GPGMM Authors
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

#include "gpgmm/common/MemoryAllocation.h"

#include "gpgmm/common/Memory.h"
#include "gpgmm/common/MemoryBlock.h"
#include "gpgmm/utils/Assert.h"

namespace gpgmm {

    MemoryAllocation::MemoryAllocation()
        : mAllocator(nullptr),
          mMemory(nullptr),
          mOffset(kInvalidOffset),
          mMethod(AllocationMethod::kUndefined),
          mBlock(nullptr),
          mMappedPointer(nullptr) {
    }

    MemoryAllocation::MemoryAllocation(MemoryAllocator* allocator,
                                       IMemoryObject* memory,
                                       uint64_t offset,
                                       AllocationMethod method,
                                       MemoryBlock* block,
                                       uint64_t requestSize,
                                       uint8_t* mappedPointer)
        : mAllocator(allocator),
          mMemory(memory),
          mOffset(offset),
          mMethod(method),
          mBlock(block),
#ifdef GPGMM_ENABLE_MEMORY_ALIGN_CHECKS
          mRequestSize(requestSize),
#endif
          mMappedPointer(mappedPointer) {
    }

    MemoryAllocation::MemoryAllocation(MemoryAllocator* allocator,
                                       IMemoryObject* memory,
                                       uint64_t requestSize,
                                       uint8_t* mappedPointer)
        : mAllocator(allocator),
          mMemory(memory),
          mOffset(0),
          mMethod(AllocationMethod::kStandalone),
          mBlock(nullptr),
#ifdef GPGMM_ENABLE_MEMORY_ALIGN_CHECKS
          mRequestSize(requestSize),
#endif
          mMappedPointer(mappedPointer) {
    }

    bool MemoryAllocation::operator==(const MemoryAllocation& other) const {
        return (other.mAllocator == mAllocator && other.mMemory == mMemory &&
                other.mOffset == mOffset && other.mMethod == mMethod && other.mBlock == mBlock);
    }

    bool MemoryAllocation::operator!=(const MemoryAllocation& other) const {
        return !operator==(other);
    }

    MemoryAllocationInfo MemoryAllocation::GetInfo() const {
        return {GetSize(), GetAlignment()};
    }

    IMemoryObject* MemoryAllocation::GetMemory() const {
        return mMemory;
    }

    uint8_t* MemoryAllocation::GetMappedPointer() const {
        return mMappedPointer;
    }

    MemoryAllocator* MemoryAllocation::GetAllocator() const {
        return mAllocator;
    }

    uint64_t MemoryAllocation::GetSize() const {
        switch (mMethod) {
            case gpgmm::AllocationMethod::kStandalone:
                ASSERT(mMemory != nullptr);
                return mMemory->GetSize();
            case gpgmm::AllocationMethod::kSubAllocated:
            case gpgmm::AllocationMethod::kSubAllocatedWithin: {
                ASSERT(mBlock != nullptr);
                return mBlock->Size;
            }
            default: {
                UNREACHABLE();
                return kInvalidSize;
            }
        }
    }

    uint64_t MemoryAllocation::GetRequestSize() const {
#ifdef GPGMM_ENABLE_MEMORY_ALIGN_CHECKS
        return mRequestSize;
#else
        return kInvalidSize;
#endif
    }

    uint64_t MemoryAllocation::GetAlignment() const {
        switch (mMethod) {
            case gpgmm::AllocationMethod::kStandalone:
                ASSERT(mMemory != nullptr);
                return mMemory->GetAlignment();
            // Sub-allocation cannot be further divided and must have a alignment equal to the
            // the size.
            case gpgmm::AllocationMethod::kSubAllocated:
            case gpgmm::AllocationMethod::kSubAllocatedWithin: {
                ASSERT(mBlock != nullptr);
                return mBlock->Size;
            }
            default: {
                UNREACHABLE();
                return kInvalidSize;
            }
        }
    }

    uint64_t MemoryAllocation::GetOffset() const {
        return mOffset;
    }

    AllocationMethod MemoryAllocation::GetMethod() const {
        return mMethod;
    }

    MemoryBlock* MemoryAllocation::GetBlock() const {
        return mBlock;
    }

}  // namespace gpgmm
