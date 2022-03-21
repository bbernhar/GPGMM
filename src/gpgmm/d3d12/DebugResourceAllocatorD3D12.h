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

#ifndef GPGMM_D3D12_DEBUGRESOURCEALLOCATORD3D12_H_
#define GPGMM_D3D12_DEBUGRESOURCEALLOCATORD3D12_H_

#include "gpgmm/MemoryAllocator.h"
#include "gpgmm/MemoryCache.h"

namespace gpgmm { namespace d3d12 {

    class ResourceAllocation;

    class DebugResourceAllocator : public MemoryAllocator {
      public:
        DebugResourceAllocator() = default;
        ~DebugResourceAllocator() override;

        void AddAllocationToTrack(ResourceAllocation* allocation);
        void ReportLiveAllocations() const;

        void DeallocateMemory(MemoryAllocation* allocation) override;

      private:
        class ResourceAllocationEntry {
          public:
            ResourceAllocationEntry(ResourceAllocation* allocation) : mAllocation(allocation) {
            }

            ResourceAllocationEntry(ResourceAllocation* allocation, MemoryAllocator* allocator)
                : mAllocation(allocation), mAllocator(allocator) {
            }

            MemoryAllocator* GetAllocator() const {
                return mAllocator;
            }

            ResourceAllocation* GetAllocation() const {
                return mAllocation;
            }

            size_t GetKey() const {
                return reinterpret_cast<uintptr_t>(mAllocation);
            }

          private:
            ResourceAllocation* mAllocation = nullptr;
            MemoryAllocator* mAllocator = nullptr;
        };

        MemoryCache<ResourceAllocationEntry> mLiveAllocations = {};
    };

}}  // namespace gpgmm::d3d12

#endif  // GPGMM_D3D12_DEBUGRESOURCEALLOCATORD3D12_H_
