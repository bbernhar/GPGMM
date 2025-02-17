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

#include "gpgmm/vk/DeviceMemoryVk.h"

namespace gpgmm::vk {

    DeviceMemory::DeviceMemory(VkDeviceMemory memory,
                               uint32_t memoryTypeIndex,
                               uint64_t size,
                               uint64_t alignment)
        : MemoryBase(size, alignment), mMemory(memory), mMemoryTypeIndex(memoryTypeIndex) {
    }

    VkDeviceMemory DeviceMemory::GetDeviceMemory() const {
        return mMemory;
    }

    uint32_t DeviceMemory::GetMemoryTypeIndex() const {
        return mMemoryTypeIndex;
    }

    uint64_t DeviceMemory::GetSize() const {
        return MemoryBase::GetSize();
    }

    uint64_t DeviceMemory::GetAlignment() const {
        return MemoryBase::GetAlignment();
    }

    void DeviceMemory::AddSubAllocationRef() {
        return MemoryBase::AddSubAllocationRef();
    }

    bool DeviceMemory::RemoveSubAllocationRef() {
        return MemoryBase::RemoveSubAllocationRef();
    }

    IMemoryPool* DeviceMemory::GetPool() const {
        return MemoryBase::GetPool();
    }

    void DeviceMemory::SetPool(IMemoryPool* pool) {
        return MemoryBase::SetPool(pool);
    }

}  // namespace gpgmm::vk
