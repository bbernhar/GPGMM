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

#include "gpgmm/common/MemoryAllocator.h"

#include "gpgmm/utils/Math.h"

namespace gpgmm {

    class AllocateMemoryTask : public VoidCallback {
      public:
        AllocateMemoryTask(MemoryAllocator* allocator, const MEMORY_ALLOCATION_REQUEST& request)
            : mAllocator(allocator), mRequest(request) {
        }

        void operator()() override {
            mAllocation = mAllocator->TryAllocateMemory(mRequest);
        }

        std::unique_ptr<MemoryAllocation> AcquireAllocation() {
            return std::move(mAllocation);
        }

      private:
        MemoryAllocator* const mAllocator;
        const MEMORY_ALLOCATION_REQUEST mRequest;

        std::unique_ptr<MemoryAllocation> mAllocation;
    };

    // MemoryAllocationEvent

    MemoryAllocationEvent::MemoryAllocationEvent(std::shared_ptr<Event> event,
                                                 std::shared_ptr<AllocateMemoryTask> task)
        : mTask(task), mEvent(event) {
    }

    void MemoryAllocationEvent::Wait() {
        mEvent->Wait();
    }

    bool MemoryAllocationEvent::IsSignaled() {
        return mEvent->IsSignaled();
    }

    void MemoryAllocationEvent::Signal() {
        return mEvent->Signal();
    }

    std::unique_ptr<MemoryAllocation> MemoryAllocationEvent::AcquireAllocation() const {
        return mTask->AcquireAllocation();
    }

    // MemoryAllocator

    MemoryAllocator::MemoryAllocator() : mThreadPool(ThreadPool::Create()) {
    }

    MemoryAllocator::MemoryAllocator(std::unique_ptr<MemoryAllocator> next)
        : AllocatorNode(std::move(next)), mThreadPool(ThreadPool::Create()) {
    }

    MemoryAllocator::~MemoryAllocator() {
        // If memory cannot be reused by a (parent) allocator, ensure no used memory leaked.
        if (GetParent() == nullptr) {
            ASSERT(mInfo.UsedBlockUsage == 0u);
            ASSERT(mInfo.UsedBlockCount == 0u);
            ASSERT(mInfo.UsedMemoryCount == 0u);
            ASSERT(mInfo.UsedMemoryUsage == 0u);
        }
    }

    std::unique_ptr<MemoryAllocation> MemoryAllocator::TryAllocateMemory(
        const MEMORY_ALLOCATION_REQUEST& request) {
        ASSERT(false);
        return {};
    }

    std::shared_ptr<MemoryAllocationEvent> MemoryAllocator::TryAllocateMemoryAsync(
        const MEMORY_ALLOCATION_REQUEST& request) {
        std::shared_ptr<AllocateMemoryTask> task =
            std::make_shared<AllocateMemoryTask>(this, request);
        return std::make_shared<MemoryAllocationEvent>(ThreadPool::PostTask(mThreadPool, task),
                                                       task);
    }

    void MemoryAllocator::ReleaseMemory() {
        std::lock_guard<std::mutex> lock(mMutex);
        if (GetNextInChain() != nullptr) {
            GetNextInChain()->ReleaseMemory();
        }
    }

    uint64_t MemoryAllocator::GetMemorySize() const {
        return kInvalidSize;
    }

    uint64_t MemoryAllocator::GetMemoryAlignment() const {
        return kInvalidOffset;
    }

    MEMORY_ALLOCATOR_INFO MemoryAllocator::GetInfo() const {
        std::lock_guard<std::mutex> lock(mMutex);
        return mInfo;
    }

    const char* MemoryAllocator::GetTypename() const {
        return "MemoryAllocator";
    }

    bool MemoryAllocator::IsRequestInvalid(const MEMORY_ALLOCATION_REQUEST& request) const {
        GPGMM_INVALID_IF(request.Alignment == 0, "Requested alignment must be non-zero.");
        GPGMM_INVALID_IF(request.SizeInBytes == 0, "Requested size must be non-zero");
        GPGMM_INVALID_IF(AlignTo(request.SizeInBytes, request.Alignment),
                         "Requested size is not aligned to the alignment.");

        GPGMM_INVALID_IF(GetMemorySize() != kInvalidSize && GetMemorySize() < request.SizeInBytes,
                         "Request size exceeds memory size allowed by allocator.");
        GPGMM_INVALID_IF(
            GetMemoryAlignment() != kInvalidOffset && GetMemoryAlignment() < request.Alignment,
            "Request alignment exceeds memory alignment allowed by allocator.");
        return false;
    }

}  // namespace gpgmm
