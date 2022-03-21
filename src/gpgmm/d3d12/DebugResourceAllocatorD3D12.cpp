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

#include "gpgmm/d3d12/DebugResourceAllocatorD3D12.h"

#include "gpgmm/common/Log.h"
#include "gpgmm/common/Utils.h"
#include "gpgmm/d3d12/BackendD3D12.h"
#include "gpgmm/d3d12/ErrorD3D12.h"
#include "gpgmm/d3d12/ResourceAllocationD3D12.h"
#include "gpgmm/d3d12/SerializerD3D12.h"

namespace gpgmm { namespace d3d12 {

    DebugResourceAllocator::~DebugResourceAllocator() {
        ReportLiveAllocations();
    }

    void DebugResourceAllocator::ReportLiveAllocations() const {
        for (auto allocationEntry : mLiveAllocations) {
            const ResourceAllocation* allocation = allocationEntry->GetValue().GetAllocation();
            gpgmm::WarningLog() << "Live ResourceAllocation: "
                                << "Addr=" << ToString(allocation) << ", "
                                << "ExtRef=" << allocation->GetRefCount() << ", "
                                << "Info="
                                << Serializer::Serialize(allocation->GetInfo()).ToString();
        }
    }

    void DebugResourceAllocator::AddAllocationToTrack(ResourceAllocation* allocation) {
        mLiveAllocations.GetOrCreate(
            ResourceAllocationEntry(allocation, allocation->GetAllocator()), true);
        allocation->SetAllocator(this);
    }

    void DebugResourceAllocator::DeallocateMemory(MemoryAllocation* allocation) {
        auto entry =
            mLiveAllocations.GetOrCreate(ResourceAllocationEntry(ToBackend(allocation)), false);
        bool hasNoRef = entry->Unref();
        ASSERT(hasNoRef);

        entry->GetValue().GetAllocator()->DeallocateMemory(allocation);
    }

}}  // namespace gpgmm::d3d12
