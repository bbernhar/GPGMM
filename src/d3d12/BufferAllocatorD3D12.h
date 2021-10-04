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

#ifndef GPGMM_D3D12_BUFFERALLOCATORD3D12_H_
#define GPGMM_D3D12_BUFFERALLOCATORD3D12_H_

#include "src/MemoryAllocator.h"

#include "src/d3d12/d3d12_platform.h"

namespace gpgmm { namespace d3d12 {

    class ResourceAllocator;

    class BufferAllocator : public MemoryAllocator {
      public:
        BufferAllocator(ResourceAllocator* resourceAlloctor,
                        D3D12_HEAP_TYPE heapType,
                        D3D12_RESOURCE_FLAGS resourceFlags,
                        D3D12_RESOURCE_STATES initialUsage,
                        uint64_t resourceSize,
                        uint64_t resourceAlignment);
        ~BufferAllocator() override = default;

        // MemoryAllocator interface
        std::unique_ptr<MemoryAllocation> AllocateMemory(uint64_t size,
                                                         uint64_t alignment) override;
        void DeallocateMemory(MemoryAllocation* allocation) override;

        uint64_t GetMemorySize() const override;
        uint64_t GetMemoryAlignment() const override;

      private:
        ResourceAllocator* mResourceAllocator;

        D3D12_HEAP_TYPE mHeapType;
        D3D12_RESOURCE_FLAGS mResourceFlags;
        D3D12_RESOURCE_STATES mInitialUsage;

        uint64_t mResourceSize;
        uint64_t mResourceAlignment;
    };

}}  // namespace gpgmm::d3d12

#endif  // GPGMM_D3D12_BUFFERALLOCATORD3D12_H_
