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

#ifndef GPGMM_D3D12_BUFFERALLOCATORD3D12_H_
#define GPGMM_D3D12_BUFFERALLOCATORD3D12_H_

#include "gpgmm/common/MemoryAllocator.h"

#include "gpgmm/d3d12/d3d12_platform.h"

namespace gpgmm::d3d12 {

    class ResourceAllocator;

    class BufferAllocator : public MemoryAllocator {
      public:
        BufferAllocator(ResourceAllocator* resourceAllocator,
                        D3D12_HEAP_PROPERTIES heapProperties,
                        D3D12_HEAP_FLAGS heapFlags,
                        D3D12_RESOURCE_FLAGS resourceFlags,
                        D3D12_RESOURCE_STATES initialResourceState);
        ~BufferAllocator() override = default;

        // MemoryAllocator interface
        std::unique_ptr<MemoryAllocation> TryAllocateMemory(
            const MemoryAllocationRequest& request) override;
        void DeallocateMemory(std::unique_ptr<MemoryAllocation> allocation) override;

      private:
        ResourceAllocator* const mResourceAllocator;

        const D3D12_HEAP_PROPERTIES mHeapProperties;
        const D3D12_HEAP_FLAGS mHeapFlags;
        const D3D12_RESOURCE_FLAGS mResourceFlags;
        const D3D12_RESOURCE_STATES mInitialResourceState;
    };

}  // namespace gpgmm::d3d12

#endif  // GPGMM_D3D12_BUFFERALLOCATORD3D12_H_
