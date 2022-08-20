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

#ifndef INCLUDE_GPGMM_D3D12_MIN_H_
#define INCLUDE_GPGMM_D3D12_MIN_H_

// gpgmm_d3d12_min.h is self-contained header of a minimum viable GPGMM implementation.
// This header specifically allows users to leverage GPGMM's portable GMM interface without
// requiring the full GPGMM implementation in the build, allowing for incremental enabling during
// development. Internally, GPU memory will not be re-used nor does it do any residency management
// (will no-op). It is functionality-equivelent to calling ID3D12Device::CreateCommittedResource.

// Macro must be defined should these platform headers be already provided.
#ifndef GPGMM_D3D12_HEADERS_ALREADY_INCLUDED
#    include <d3d12.h>
#    include <dxgi1_4.h>
#    include <wrl.h>
#endif

#include <atomic>
#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <string>
#include <vector>

#define GPGMM_RETURN_IF_FAILED(expr) \
    {                                \
        HRESULT hr = expr;           \
        if (FAILED(hr)) {            \
            return hr;               \
        }                            \
    }                                \
    for (;;)                         \
    break

namespace gpgmm {

    static constexpr uint64_t kInvalidSize = std::numeric_limits<uint64_t>::max();
    static constexpr uint64_t kInvalidOffset = std::numeric_limits<uint64_t>::max();

    class MemoryBase {
      public:
        MemoryBase(uint64_t size, uint64_t alignment) : mSize(size), mAlignment(alignment) {
        }

        uint64_t GetSize() const {
            return mSize;
        }

        uint64_t GetAlignment() const {
            return mAlignment;
        }

      private:
        const uint64_t mSize;
        const uint64_t mAlignment;
    };

    struct MemoryAllocationInfo {
        uint64_t SizeInBytes;
        uint64_t Alignment;
    };

    struct MemoryAllocatorInfo {
        uint32_t UsedBlockCount;
        uint64_t UsedBlockUsage;
        uint32_t UsedMemoryCount;
        uint64_t UsedMemoryUsage;
        uint64_t FreeMemoryUsage;
        uint64_t PrefetchedMemoryMisses;
        uint64_t PrefetchedMemoryMissesEliminated;
        uint64_t SizeCacheMisses;
        uint64_t SizeCacheHits;
    };

    class MemoryAllocation;

    class MemoryAllocator {
      public:
        virtual void DeallocateMemory(std::unique_ptr<MemoryAllocation> allocation) = 0;

        virtual uint64_t ReleaseMemory(uint64_t bytesToRelease = kInvalidSize) {
            return 0;
        }

        virtual MemoryAllocatorInfo GetInfo() const {
            return {};
        }

      protected:
        MemoryAllocatorInfo mInfo = {};
    };

    enum AllocationMethod {
        kStandalone = 0x0,
        kSubAllocated = 0x2,
        kSubAllocatedWithin = 0x4,
        kUndefined = 0x8
    };

    class MemoryBlock;

    class MemoryAllocation {
      public:
        MemoryAllocation(MemoryAllocator* allocator, MemoryBase* memory, uint64_t requestSize)
            : mAllocator(allocator), mMemory(memory), mRequestSize(requestSize) {
        }

        virtual ~MemoryAllocation() = default;

        MemoryAllocation(const MemoryAllocation&) = default;
        MemoryAllocation& operator=(const MemoryAllocation&) = default;
        bool operator==(const MemoryAllocation&) const;
        bool operator!=(const MemoryAllocation& other) const;

        MemoryAllocationInfo GetInfo() const {
            return {GetSize(), GetAlignment()};
        }

        MemoryBase* GetMemory() const {
            return mMemory;
        }

        uint8_t* GetMappedPointer() const {
            return nullptr;
        }

        MemoryAllocator* GetAllocator() const {
            return mAllocator;
        }
        uint64_t GetSize() const {
            return mMemory->GetSize();
        }

        uint64_t GetRequestSize() const {
            return mRequestSize;
        }

        uint64_t GetAlignment() const {
            return mMemory->GetAlignment();
        }

        uint64_t GetOffset() const {
            return 0;
        }

        AllocationMethod GetMethod() const {
            return AllocationMethod::kStandalone;
        }

        MemoryBlock* GetBlock() const {
            return nullptr;
        }

      protected:
        MemoryAllocator* mAllocator;

      private:
        MemoryBase* mMemory;
        uint64_t mRequestSize;
    };

}  // namespace gpgmm

namespace gpgmm::d3d12 {

    class IUnknownImpl : public IUnknown {
      public:
        IUnknownImpl() = default;
        virtual ~IUnknownImpl() = default;

        // IUnknown interface
        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override {
            // Always set out parameter to nullptr, validating it first.
            if (ppvObject == nullptr) {
                return E_INVALIDARG;
            }

            *ppvObject = nullptr;

            if (riid == IID_IUnknown) {
                // Increment reference and return pointer.
                *ppvObject = this;
                AddRef();
                return S_OK;
            }
            return E_NOINTERFACE;
        }

        ULONG STDMETHODCALLTYPE AddRef() override {
            return ++mRefCount;
        }

        ULONG STDMETHODCALLTYPE Release() override {
            const ULONG refCount = --mRefCount ? 0 : mRefCount;
            if (refCount == 0) {
                DeleteThis();
            }
            return refCount;
        }

        // Derived class may override this if they require a custom deleter.
        virtual void DeleteThis() {
            delete this;
        }

      private:
        uint64_t mRefCount = 1;
    };

    enum RESIDENCY_SEGMENT {
        RESIDENCY_SEGMENT_UNKNOWN,
        RESIDENCY_SEGMENT_LOCAL,
        RESIDENCY_SEGMENT_NON_LOCAL,
    };

    struct HEAP_INFO {
        bool IsResident;
    };

    struct HEAP_DESC {
        uint64_t SizeInBytes;
        uint64_t Alignment;
        D3D12_HEAP_TYPE HeapType;
        bool AlwaysInBudget;
        bool IsExternal;
        RESIDENCY_SEGMENT MemorySegment;
        std::string DebugName;
    };

    using CreateHeapFn = std::function<HRESULT(ID3D12Pageable** ppPageableOut)>;

    class ResidencyManager;
    class ResourceAllocator;

    class Heap final : public MemoryBase, public IUnknownImpl {
      public:
        static HRESULT CreateHeap(const HEAP_DESC& descriptor,
                                  ResidencyManager* const pResidencyManager,
                                  CreateHeapFn&& createHeapFn,
                                  Heap** ppHeapOut) {
            Microsoft::WRL::ComPtr<ID3D12Pageable> pageable;
            GPGMM_RETURN_IF_FAILED(createHeapFn(&pageable));

            if (ppHeapOut != nullptr) {
                *ppHeapOut = new Heap(pageable, descriptor.SizeInBytes, descriptor.Alignment);
            }

            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override {
            return mPageable->QueryInterface(riid, ppvObject);
        }

        bool IsResident() const {
            return true;
        }

        HEAP_INFO GetInfo() const {
            return {IsResident()};
        }

      private:
        Heap(Microsoft::WRL::ComPtr<ID3D12Pageable> pageable, uint64_t size, uint64_t alignment);

        Microsoft::WRL::ComPtr<ID3D12Pageable> mPageable;
    };

    class ResidencyList final {
      public:
        ResidencyList() = default;

        HRESULT Add(Heap* pHeap) {
            return S_OK;
        }

        HRESULT Reset() {
            return S_OK;
        }

        std::vector<Heap*>::const_iterator begin() const {
            return mList.begin();
        }

        std::vector<Heap*>::const_iterator end() const {
            return mList.end();
        }

      private:
        std::vector<Heap*> mList;
    };

    enum EVENT_RECORD_SCOPE {
        EVENT_RECORD_SCOPE_PER_PROCESS = 0x1,
        EVENT_RECORD_SCOPE_PER_INSTANCE = 0x2,
    };

    enum EVENT_RECORD_FLAGS {
        EVENT_RECORD_FLAG_NONE = 0x0,
        EVENT_RECORD_FLAG_API_OBJECTS = 0x1,
        EVENT_RECORD_FLAG_API_CALLS = 0x2,
        EVENT_RECORD_FLAG_API_TIMINGS = 0x4,
        EVENT_RECORD_FLAG_COUNTERS = 0x8,
        EVENT_RECORD_FLAG_CAPTURE = 0x3,
        EVENT_RECORD_FLAG_ALL_EVENTS = 0xFF,
    };

    struct EVENT_RECORD_OPTIONS {
        EVENT_RECORD_FLAGS Flags;
        D3D12_MESSAGE_SEVERITY MinMessageLevel;
        EVENT_RECORD_SCOPE EventScope;
        bool UseDetailedTimingEvents;
        std::string TraceFile;
    };

    struct RESIDENCY_DESC {
        Microsoft::WRL::ComPtr<ID3D12Device> Device;
        Microsoft::WRL::ComPtr<IDXGIAdapter3> Adapter;
        bool IsUMA;
        D3D12_MESSAGE_SEVERITY MinLogLevel;
        EVENT_RECORD_OPTIONS RecordOptions;
        float VideoMemoryBudget;
        uint64_t Budget;
        uint64_t EvictBatchSize;
        uint64_t InitialFenceValue;
        bool UpdateBudgetByPolling;
    };

    struct RESIDENCY_INFO {
        uint64_t ResidentMemoryUsage;
        uint64_t ResidentMemoryCount;
    };

    class ResidencyManager final : public IUnknownImpl {
      public:
        static HRESULT CreateResidencyManager(const RESIDENCY_DESC& descriptor,
                                              ResidencyManager** ppResidencyManagerOut) {
            if (ppResidencyManagerOut != nullptr) {
                *ppResidencyManagerOut = new ResidencyManager(descriptor);
            }

            return S_OK;
        }

        ~ResidencyManager() override = default;

        HRESULT LockHeap(Heap* pHeap) {
            return S_OK;
        }

        HRESULT UnlockHeap(Heap* pHeap) {
            return S_OK;
        }

        HRESULT ExecuteCommandLists(ID3D12CommandQueue* pQueue,
                                    ID3D12CommandList* const* ppCommandLists,
                                    ResidencyList* const* ppResidencyLists,
                                    uint32_t count) {
            pQueue->ExecuteCommandLists(count, ppCommandLists);
            return S_OK;
        }

        HRESULT SetVideoMemoryReservation(const DXGI_MEMORY_SEGMENT_GROUP& memorySegmentGroup,
                                          uint64_t availableForReservation,
                                          uint64_t* pCurrentReservationOut = nullptr) {
            return S_OK;
        }

        DXGI_QUERY_VIDEO_MEMORY_INFO* GetVideoMemoryInfo(
            const DXGI_MEMORY_SEGMENT_GROUP& memorySegmentGroup) {
            return nullptr;
        }

        HRESULT UpdateVideoMemorySegments() {
            return S_OK;
        }

        RESIDENCY_INFO GetInfo() const {
            return {0, 0};
        }

        DXGI_MEMORY_SEGMENT_GROUP GetMemorySegmentGroup(D3D12_HEAP_TYPE heapType) const {
            if (mIsUMA) {
                return DXGI_MEMORY_SEGMENT_GROUP_LOCAL;
            }

            D3D12_HEAP_PROPERTIES heapProperties = mDevice->GetCustomHeapProperties(0, heapType);

            if (heapProperties.MemoryPoolPreference == D3D12_MEMORY_POOL_L1) {
                return DXGI_MEMORY_SEGMENT_GROUP_LOCAL;
            }

            return DXGI_MEMORY_SEGMENT_GROUP_NON_LOCAL;
        }

      private:
        ResidencyManager(const RESIDENCY_DESC& descriptor)
            : mDevice(descriptor.Device), mAdapter(descriptor.Adapter), mIsUMA(descriptor.IsUMA) {
        }

        Microsoft::WRL::ComPtr<ID3D12Device> mDevice;
        Microsoft::WRL::ComPtr<IDXGIAdapter3> mAdapter;

        const bool mIsUMA;
    };

    struct RESOURCE_ALLOCATION_DESC {
        uint64_t RequestSizeInBytes;
        uint64_t HeapOffset;
        uint64_t OffsetFromResource;
        AllocationMethod Method;
        std::string DebugName;
    };

    struct RESOURCE_ALLOCATION_INFO {
        uint64_t SizeInBytes;
        uint64_t Alignment;
    };

    class ResourceAllocation final : public MemoryAllocation, public IUnknownImpl {
      public:
        static HRESULT CreateResourceAllocation(const RESOURCE_ALLOCATION_DESC& desc,
                                                ResidencyManager* pResidencyManager,
                                                MemoryAllocator* pAllocator,
                                                Heap* pResourceHeap,
                                                MemoryBlock* pBlock,
                                                Microsoft::WRL::ComPtr<ID3D12Resource> resource,
                                                ResourceAllocation** ppResourceAllocationOut) {
            if (ppResourceAllocationOut != nullptr) {
                *ppResourceAllocationOut =
                    new ResourceAllocation(desc, pAllocator, pResourceHeap, resource);
            }

            return S_OK;
        }

        void DeleteThis() override {
            GetAllocator()->DeallocateMemory(std::unique_ptr<ResourceAllocation>(this));
        }

        HRESULT Map(uint32_t subresource = 0,
                    const D3D12_RANGE* readRange = nullptr,
                    void** dataOut = nullptr) {
            return mResource->Map(subresource, readRange, dataOut);
        }

        void Unmap(uint32_t subresource = 0, const D3D12_RANGE* writtenRange = nullptr) {
            return mResource->Unmap(subresource, writtenRange);
        }

        ID3D12Resource* GetResource() const {
            return mResource.Get();
        }

        bool IsResident() const {
            return static_cast<Heap*>(GetMemory())->IsResident();
        }

        D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const {
            return mResource->GetGPUVirtualAddress();
        }

        uint64_t GetOffsetFromResource() const {
            return 0;
        }

        RESOURCE_ALLOCATION_INFO GetInfo() const {
            return {GetSize(), GetAlignment()};
        }

        Heap* GetMemory() const {
            return static_cast<Heap*>(MemoryAllocation::GetMemory());
        }

      private:
        ResourceAllocation(const RESOURCE_ALLOCATION_DESC& desc,
                           MemoryAllocator* allocator,
                           Heap* resourceHeap,
                           Microsoft::WRL::ComPtr<ID3D12Resource> resource)
            : MemoryAllocation(allocator, resourceHeap, desc.RequestSizeInBytes),
              mResource(std::move(resource)) {
        }

        Microsoft::WRL::ComPtr<ID3D12Resource> mResource;
    };

    enum ALLOCATOR_FLAGS {
        ALLOCATOR_FLAG_NONE = 0x0,
        ALLOCATOR_FLAG_ALWAYS_COMMITED = 0x1,
        ALLOCATOR_FLAG_ALWAYS_IN_BUDGET = 0x2,
        ALLOCATOR_FLAG_DISABLE_MEMORY_PREFETCH = 0x4,
        ALLOCATOR_FLAG_ALWAYS_ON_DEMAND = 0x8,
    };

    enum ALLOCATOR_ALGORITHM {
        ALLOCATOR_ALGORITHM_SLAB = 0x0,
        ALLOCATOR_ALGORITHM_BUDDY_SYSTEM = 0x1,
        ALLOCATOR_ALGORITHM_FIXED_POOL = 0x2,
        ALLOCATOR_ALGORITHM_SEGMENTED_POOL = 0x3,
    };

    struct ALLOCATOR_DESC {
        Microsoft::WRL::ComPtr<ID3D12Device> Device;
        Microsoft::WRL::ComPtr<IDXGIAdapter> Adapter;
        ALLOCATOR_FLAGS Flags = ALLOCATOR_FLAG_NONE;
        D3D12_MESSAGE_SEVERITY MinLogLevel;
        EVENT_RECORD_OPTIONS RecordOptions;
        D3D12_RESOURCE_HEAP_TIER ResourceHeapTier;
        ALLOCATOR_ALGORITHM SubAllocationAlgorithm;
        ALLOCATOR_ALGORITHM PoolAlgorithm;
        uint64_t PreferredResourceHeapSize;
        uint64_t MaxResourceHeapSize;
        double MemoryFragmentationLimit;
        double MemoryGrowthFactor;
    };

    enum ALLOCATION_FLAGS {
        ALLOCATION_FLAG_NONE = 0x0,
        ALLOCATION_FLAG_NEVER_ALLOCATE_MEMORY = 0x1,
        ALLOCATION_FLAG_ALLOW_SUBALLOCATE_WITHIN_RESOURCE = 0x2,
        ALLOCATION_FLAG_NEVER_SUBALLOCATE_MEMORY = 0x4,
        ALLOCATION_FLAG_ALWAYS_PREFETCH_MEMORY = 0x8,
        ALLOCATION_FLAG_ALWAYS_CACHE_SIZE = 0x10,
    };

    struct ALLOCATION_DESC {
        ALLOCATION_FLAGS Flags = ALLOCATION_FLAG_NONE;
        D3D12_HEAP_TYPE HeapType = D3D12_HEAP_TYPE_DEFAULT;
        D3D12_HEAP_FLAGS ExtraRequiredHeapFlags = D3D12_HEAP_FLAG_NONE;
        uint64_t RequireResourceHeapPadding = 0;
        std::string DebugName;
    };

    enum FEATURE {
        FEATURE_RESOURCE_SUBALLOCATION_SUPPORT,
    };

    using RESOURCE_ALLOCATOR_INFO = MemoryAllocatorInfo;

    class ResourceAllocator final : public MemoryAllocator, public IUnknownImpl {
      public:
        static HRESULT CreateAllocator(const ALLOCATOR_DESC& allocatorDescriptor,
                                       ResourceAllocator** ppResourceAllocatorOut,
                                       ResidencyManager** ppResidencyManagerOut = nullptr) {
            if (allocatorDescriptor.Device == nullptr || allocatorDescriptor.Adapter == nullptr) {
                return E_INVALIDARG;
            }

            Microsoft::WRL::ComPtr<ResidencyManager> residencyManager;
            if (ppResidencyManagerOut != nullptr) {
                RESIDENCY_DESC residencyDesc = {};
                residencyDesc.Device = allocatorDescriptor.Device;

                D3D12_FEATURE_DATA_ARCHITECTURE arch = {};
                GPGMM_RETURN_IF_FAILED(residencyDesc.Device->CheckFeatureSupport(
                    D3D12_FEATURE_ARCHITECTURE, &arch, sizeof(arch)));
                residencyDesc.IsUMA = arch.UMA;

                GPGMM_RETURN_IF_FAILED(allocatorDescriptor.Adapter.As(&residencyDesc.Adapter));

                GPGMM_RETURN_IF_FAILED(
                    ResidencyManager::CreateResidencyManager(residencyDesc, &residencyManager));
            }

            Microsoft::WRL::ComPtr<ResourceAllocator> resourceAllocator;
            GPGMM_RETURN_IF_FAILED(
                CreateAllocator(allocatorDescriptor, residencyManager.Get(), &resourceAllocator));

            if (ppResourceAllocatorOut != nullptr) {
                *ppResourceAllocatorOut = resourceAllocator.Detach();
            }

            if (ppResidencyManagerOut != nullptr) {
                *ppResidencyManagerOut = residencyManager.Detach();
            }

            return S_OK;
        }

        static HRESULT CreateAllocator(const ALLOCATOR_DESC& allocatorDescriptor,
                                       ResidencyManager* pResidencyManager,
                                       ResourceAllocator** ppResourceAllocatorOut) {
            if (ppResourceAllocatorOut != nullptr) {
                *ppResourceAllocatorOut =
                    new ResourceAllocator(allocatorDescriptor, pResidencyManager);
            }

            return S_OK;
        }

        HRESULT CreateResource(const ALLOCATION_DESC& allocationDescriptor,
                               const D3D12_RESOURCE_DESC& resourceDescriptor,
                               D3D12_RESOURCE_STATES initialResourceState,
                               const D3D12_CLEAR_VALUE* pClearValue,
                               ResourceAllocation** ppResourceAllocationOut) {
            Heap* resourceHeap = nullptr;
            Microsoft::WRL::ComPtr<ID3D12Resource> committedResource;

            const D3D12_RESOURCE_ALLOCATION_INFO resourceInfo =
                mDevice->GetResourceAllocationInfo(0, 1, &resourceDescriptor);

            HEAP_DESC resourceHeapDesc = {};
            resourceHeapDesc.SizeInBytes = resourceInfo.SizeInBytes;
            resourceHeapDesc.Alignment = resourceInfo.Alignment;
            resourceHeapDesc.HeapType = allocationDescriptor.HeapType;

            GPGMM_RETURN_IF_FAILED(Heap::CreateHeap(
                resourceHeapDesc, mResidencyManager.Get(),
                [&](ID3D12Pageable** ppPageableOut) -> HRESULT {
                    D3D12_HEAP_PROPERTIES heapProperties = {};
                    heapProperties.Type = allocationDescriptor.HeapType;

                    GPGMM_RETURN_IF_FAILED(mDevice->CreateCommittedResource(
                        &heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDescriptor,
                        initialResourceState, pClearValue, IID_PPV_ARGS(&committedResource)));

                    Microsoft::WRL::ComPtr<ID3D12Pageable> pageable;
                    GPGMM_RETURN_IF_FAILED(committedResource.As(&pageable));
                    *ppPageableOut = pageable.Detach();
                    return S_OK;
                },
                &resourceHeap));

            const uint64_t allocationSize = resourceHeap->GetSize();
            mInfo.UsedMemoryUsage += allocationSize;
            mInfo.UsedMemoryCount++;
            mInfo.UsedBlockUsage += allocationSize;

            RESOURCE_ALLOCATION_DESC allocationDesc = {};
            allocationDesc.HeapOffset = kInvalidOffset;
            allocationDesc.RequestSizeInBytes = resourceInfo.SizeInBytes;
            allocationDesc.Method = AllocationMethod::kStandalone;

            GPGMM_RETURN_IF_FAILED(ResourceAllocation::CreateResourceAllocation(
                allocationDesc, mResidencyManager.Get(), this, resourceHeap, nullptr,
                std::move(committedResource), ppResourceAllocationOut));

            return S_OK;
        }

        HRESULT CreateResource(Microsoft::WRL::ComPtr<ID3D12Resource> committedResource,
                               ResourceAllocation** ppResourceAllocationOut) {
            return E_NOTIMPL;
        }

        uint64_t ReleaseMemory(uint64_t bytesToRelease = kInvalidSize) override {
            return 0;
        }

        RESOURCE_ALLOCATOR_INFO GetInfo() const override {
            return mInfo;
        }

        HRESULT CheckFeatureSupport(FEATURE feature,
                                    void* pFeatureSupportData,
                                    uint32_t featureSupportDataSize) const {
            return E_NOTIMPL;
        }

      private:
        ResourceAllocator(const ALLOCATOR_DESC& descriptor,
                          Microsoft::WRL::ComPtr<ResidencyManager> residencyManager)
            : mDevice(std::move(descriptor.Device)),
              mResidencyManager(std::move(residencyManager)) {
        }

        void DeallocateMemory(std::unique_ptr<MemoryAllocation> allocation) override {
            const uint64_t allocationSize = allocation->GetSize();
            mInfo.UsedMemoryUsage -= allocationSize;
            mInfo.UsedMemoryCount--;
            mInfo.UsedBlockUsage -= allocationSize;
            delete allocation->GetMemory();
        }

        Microsoft::WRL::ComPtr<ID3D12Device> mDevice;
        Microsoft::WRL::ComPtr<ResidencyManager> mResidencyManager;
    };

}  // namespace gpgmm::d3d12

#endif  // INCLUDE_GPGMM_D3D12_MIN_H_
