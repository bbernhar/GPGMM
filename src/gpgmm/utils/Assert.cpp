// Copyright 2017 The Dawn Authors
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

#include "Assert.h"
#include "Log.h"

#include <cstdlib>

namespace gpgmm {

    void HandleAssertionFailure(const char* file,
                                const char* function,
                                int line,
                                const char* condition) {
        gpgmm::ErrorLog() << "Assertion failure at " << file << ":" << line << " (" << function
                          << "): " << condition;
#if defined(GPGMM_ABORT_ON_ASSERT)
        abort();
#else
        GPGMM_BREAKPOINT();
#endif
    }

}  // namespace gpgmm
