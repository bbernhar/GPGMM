// Copyright 2017 The Dawn Authors
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

#ifndef GPGMM_COMMON_MATH_H_
#define GPGMM_COMMON_MATH_H_

#include "Assert.h"

#include <cstddef>
#include <cstdint>

#include <limits>

namespace gpgmm {

    // The following are not valid for 0
    uint32_t ScanForward(uint32_t bits);
    uint32_t Log2(uint32_t number);
    uint32_t Log2(uint64_t number);

    bool IsPowerOfTwo(uint64_t number);
    uint64_t NextPowerOfTwo(uint64_t number);
    bool IsAligned(uint32_t number, size_t multiple);

    template <typename T>
    T AlignTo(T number, size_t multiple) {
        ASSERT(multiple != 0);
        if (!IsPowerOfTwo(multiple)) {
            return ((number + multiple - 1) / multiple) * multiple;
        }
        ASSERT(number <= std::numeric_limits<T>::max() - (multiple - 1));
        ASSERT(IsPowerOfTwo(multiple));
        T multipleT = static_cast<T>(multiple);
        return (number + (multipleT - 1)) & ~(multipleT - 1);
    }

}  // namespace gpgmm

#endif  // GPGMM_COMMON_MATH_H_
