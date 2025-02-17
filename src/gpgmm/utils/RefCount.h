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

#ifndef GPGMM_UTILS_REFCOUNT_H_
#define GPGMM_UTILS_REFCOUNT_H_

#include "Utils.h"

#include <atomic>
#include <cstdint>

namespace gpgmm {

    template <typename T>
    class ScopedRef;

    class RefCounted {
      public:
        // Always require an initial refcount to construct because it is not known
        // what is being referenced (count vs object).
        RefCounted() = delete;

        explicit RefCounted(uint_fast32_t initialCount);

        // Increments ref by one.
        void Ref();

        // Decrements ref by one. If count is positive, returns false.
        // Otherwise, returns true when it reaches zero.
        bool Unref();

        // Get the ref count.
        uint_fast32_t GetRefCount() const;

        // Returns true if calling Unref() will reach a zero refcount.
        bool HasOneRef() const;

      private:
        friend ScopedRef<RefCounted>;

        mutable std::atomic_uint_fast32_t mRef;
    };

    // RAII style wrapper around RefCounted based objects.
    template <typename T>
    class ScopedRef {
      public:
        ScopedRef() = default;
        ScopedRef(std::nullptr_t) {
        }

        ScopedRef(T* ptr) : mPtr(ptr) {
            SafeRef(mPtr);
        }

        ScopedRef(const ScopedRef& other) {
            Attach(other.mPtr);
            SafeRef(mPtr);
        }

        ScopedRef(ScopedRef&& other) {
            if (this != &other) {
                mPtr = other.Detach();
            }
        }

        ~ScopedRef() {
            SafeRelease(mPtr);
        }

        ScopedRef& operator=(const ScopedRef& other) {
            SafeRef(other.mPtr);
            SafeUnref(mPtr);
            Attach(other.mPtr);
            return *this;
        }

        T* Get() const {
            return mPtr;
        }

        T* Detach() {
            T* ptr = mPtr;
            mPtr = nullptr;
            return ptr;
        }

        void Attach(T* ptr) {
            mPtr = ptr;
        }

        static ScopedRef<T> Acquire(T* ptr) {
            ScopedRef<T> ref;
            ref.Attach(ptr);
            return ref;
        }

        const T* operator->() const {
            return mPtr;
        }

        T* operator->() {
            return mPtr;
        }

        bool operator==(const ScopedRef& other) const {
            return mPtr == other.mPtr;
        }

        bool operator!=(const ScopedRef& other) const {
            return !operator==(other);
        }

      private:
        static void SafeRelease(T*& ptr) {
            if (SafeUnref(ptr)) {
                SafeDelete(ptr);
                ptr = nullptr;
            }
        }

        static void SafeRef(T* ptr) {
            if (ptr != nullptr) {
                ptr->Ref();
            }
        }

        static bool SafeUnref(T* ptr) {
            if (ptr != nullptr) {
                return ptr->Unref();
            }
            return false;
        }

        T* mPtr = nullptr;
    };
}  // namespace gpgmm

#endif  // GPGMM_UTILS_REFCOUNT_H_
