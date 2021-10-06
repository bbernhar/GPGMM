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

#include "src/tests/D3D12Test.h"
#include "src/d3d12/d3d12_platform.h"

#include <gpgmm_d3d12.h>

void D3D12GPGMMTest::SetUp() {
    GPGMMTestBase::SetUp();

    ComPtr<IDXGIAdapter3> adapter3;
    ComPtr<ID3D12Device> device;
    D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));
    ASSERT_NE(device.Get(), nullptr);

    LUID adapterLUID = device->GetAdapterLuid();
    ComPtr<IDXGIFactory1> dxgiFactory;
    CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));
    ASSERT_NE(dxgiFactory.Get(), nullptr);

    ComPtr<IDXGIFactory4> dxgiFactory4;
    dxgiFactory.As(&dxgiFactory4);
    ASSERT_NE(dxgiFactory4.Get(), nullptr);

    dxgiFactory4->EnumAdapterByLuid(adapterLUID, IID_PPV_ARGS(&adapter3));
    ASSERT_NE(adapter3.Get(), nullptr);

    D3D12_FEATURE_DATA_ARCHITECTURE arch = {};
    device->CheckFeatureSupport(D3D12_FEATURE_ARCHITECTURE, &arch, sizeof(arch));

    D3D12_FEATURE_DATA_D3D12_OPTIONS options = {};
    device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &options, sizeof(options));

    gpgmm::d3d12::ALLOCATOR_DESC allocatorDesc = {};
    allocatorDesc.Adapter = adapter3;
    allocatorDesc.Device = device;
    allocatorDesc.IsUMA = arch.UMA;
    allocatorDesc.ResourceHeapTier = options.ResourceHeapTier;

    mResourceAllocator = std::make_unique<gpgmm::d3d12::ResourceAllocator>(allocatorDesc);
}

void D3D12GPGMMTest::TearDown() {
    GPGMMTestBase::TearDown();
    // TODO
}
