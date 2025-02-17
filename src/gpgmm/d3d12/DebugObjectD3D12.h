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

#ifndef GPGMM_D3D12_DEBUGOBJECTD3D12_H_
#define GPGMM_D3D12_DEBUGOBJECTD3D12_H_

#include "gpgmm/d3d12/IUnknownImplD3D12.h"

#include "gpgmm/d3d12/d3d12_platform.h"

#include <string>

namespace gpgmm::d3d12 {
    class DebugObject : public IUnknownImpl {
      public:
        virtual ~DebugObject() override = default;

        LPCWSTR GetDebugName() const;
        HRESULT SetDebugName(LPCWSTR Name);

        DEFINE_IUNKNOWNIMPL_OVERRIDES()

      protected:
        // Derived classes should override to associate the name with the containing ID3D12Object.
        virtual HRESULT SetDebugNameImpl(LPCWSTR name) = 0;

      private:
        std::wstring mDebugName;
    };

}  // namespace gpgmm::d3d12

#endif  // GPGMM_D3D12_DEBUGOBJECTD3D12_H_
