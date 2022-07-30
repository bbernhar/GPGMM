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
// WITHOUT WARRANTIES OR CONDITIONS OF Any KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef GPGMM_UTILS_ANY_H_
#define GPGMM_UTILS_ANY_H_

#include "Utils.h"

namespace gpgmm {

    class Any {
      private:
        struct Base {
            Base() = default;
            virtual ~Base() = default;

            Base(const Base& base) = default;

            virtual Base* Create() const = 0;
        };

        template <typename T>
        struct Derived : Base {
            Derived(T const& value) : mValue(value) {
            }

            Base* Create() const {
                return new Derived<T>(*this);
            }

            T mValue;
        };

        Base* mPtr = nullptr;

      public:
        template <typename T>
        Any(T const& value) : mPtr(new Derived<T>(value)) {
        }

        Any(Any const& other) : mPtr(other.mPtr->Create()) {
        }

        Any& operator=(Any const& other) {
            Any(other).swap(*this);
            return *this;
        }

        ~Any() {
            SafeDelete(this->mPtr);
        }

        void swap(Any& other) {
            std::swap(this->mPtr, other.mPtr);
        }

        template <typename T>
        T& get() {
            return dynamic_cast<Derived<T>&>(*this->mPtr).mValue;
        }
    };

}  // namespace gpgmm

#endif  // GPGMM_UTILS_ANY_H_
