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

#ifndef GPGMM_UTILS_INDEXEDLIST_H_
#define GPGMM_UTILS_INDEXEDLIST_H_

#include "Assert.h"

namespace gpgmm {

    // IndexedList is a list that stores elements in memory contigiously.
    // Unlike a std::list, random insertion is not allowed. A added element can only be inserted at
    // the back but can be removed anywhere by index.
    template <typename T>
    class IndexedList {
      public:
        IndexedList() = default;

        explicit IndexedList(uint64_t capacity) {
            mData.reserve(capacity);
        }

        bool empty() const {
            return mSize == 0;
        }

        void clear() {
            mSize = 0;
            mData.clear();
        }

        void push_back(const T& lvalue) {
            if (mData.size() == mSize) {
                mData.push_back(lvalue);
            } else {
                mData[mSize] = lvalue;
            }
            mSize++;
        }

        void push_back(T&& rvalue) {
            if (mData.size() == mSize) {
                mData.push_back(std::move(rvalue));
            } else {
                mData[mSize] = std::move(rvalue);
            }
            mSize++;
        }

        void erase(uint64_t index) {
            ASSERT(index < mSize);
            ASSERT(mSize > 0);
            if (index < mSize - 1) {
                std::swap(mData[index], mData[mSize - 1]);
            }
            mSize--;
        }

        uint64_t capacity() const {
            return mData.capacity();
        }

        uint64_t size() const {
            return mSize;
        }

        T& pop_back() {
            ASSERT(mSize > 0);
            return mData[--mSize];
        }

        T& back() {
            return mData[mSize - 1];
        }

        const T& back() const {
            return mData[mSize - 1];
        }

      private:
        std::vector<T> mData;
        size_t mSize = 0;
    };

}  // namespace gpgmm

#endif  // GPGMM_UTILS_INDEXEDLIST_H_
