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

#ifndef GPGMM_VK_DEVICEMEMORYVK_H_
#define GPGMM_VK_DEVICEMEMORYVK_H_

#include "gpgmm/common/Memory.h"

#include "gpgmm/vk/vk_platform.h"

namespace gpgmm::vk {

    class DeviceMemory final : public MemoryBase, public IMemoryObject {
      public:
        DeviceMemory(VkDeviceMemory memory,
                     uint32_t memoryTypeIndex,
                     uint64_t size,
                     uint64_t alignment);
        ~DeviceMemory() override = default;

        VkDeviceMemory GetDeviceMemory() const;
        uint32_t GetMemoryTypeIndex() const;

        // IMemoryObject
        uint64_t GetSize() const override;
        uint64_t GetAlignment() const override;
        void AddSubAllocationRef() override;
        bool RemoveSubAllocationRef() override;
        IMemoryPool* GetPool() const override;
        void SetPool(IMemoryPool* pool) override;

      private:
        VkDeviceMemory mMemory = VK_NULL_HANDLE;
        uint32_t mMemoryTypeIndex = 0;
    };

}  // namespace gpgmm::vk

#endif  // GPGMM_VK_DEVICEMEMORYVK_H_
