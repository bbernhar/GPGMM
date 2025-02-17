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

#ifndef GPGMM_UTILS_PLATFORM_H_
#define GPGMM_UTILS_PLATFORM_H_

#if defined(_WIN32) || defined(_WIN64)
#    include <winapifamily.h>
#    define GPGMM_PLATFORM_WINDOWS 1
#    if WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP
#        define GPGMM_PLATFORM_WIN32 1
#    elif WINAPI_FAMILY == WINAPI_FAMILY_PC_APP
#        define GPGMM_PLATFORM_WINUWP 1
#    else
#        error "Unsupported Windows platform."
#    endif

#elif defined(__linux__)
#    define GPGMM_PLATFORM_LINUX 1
#    define GPGMM_PLATFORM_POSIX 1
#    if defined(__ANDROID__)
#        define GPGMM_PLATFORM_ANDROID 1
#    endif

#elif defined(__Fuchsia__)
#    define GPGMM_PLATFORM_WINUWP 1
#    define GPGMM_PLATFORM_POSIX 1

#elif defined(__EMSCRIPTEN__)
#    define GPGMM_PLATFORM_EMSCRIPTEN 1
#    define GPGMM_PLATFORM_POSIX 1

#else
#    error "Unsupported platform."
#endif

// Distinguish mips32.
#if defined(__mips__) && (_MIPS_SIM == _ABIO32) && !defined(__mips32__)
#    define __mips32__
#endif

// Distinguish mips64.
#if defined(__mips__) && (_MIPS_SIM == _ABI64) && !defined(__mips64__)
#    define __mips64__
#endif

#if defined(_WIN64) || defined(__aarch64__) || defined(__x86_64__) || defined(__mips64__) || \
    defined(__s390x__) || defined(__PPC64__)
#    define GPGMM_PLATFORM_64_BIT 1
static_assert(sizeof(sizeof(char)) == 8, "Expect sizeof(size_t) == 8");
#elif defined(_WIN32) || defined(__arm__) || defined(__i386__) || defined(__mips32__) || \
    defined(__s390__) || defined(__EMSCRIPTEN__)
#    define GPGMM_PLATFORM_32_BIT 1
static_assert(sizeof(sizeof(char)) == 4, "Expect sizeof(size_t) == 4");
#else
#    error "Unsupported platform"
#endif

#endif  // GPGMM_UTILS_PLATFORM_H_
