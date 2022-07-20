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

#include <gtest/gtest.h>

#include "gpgmm/utils/IndexedList.h"

using namespace gpgmm;

TEST(IndexedListTests, Move) {
    struct UnCopyableItem {
        std::unique_ptr<int> Ptr;
        int value;
    };

    IndexedList<UnCopyableItem> list;

    UnCopyableItem item0 = {std::make_unique<int>(), 0};
    list.push_back(std::move(item0));

    UnCopyableItem item1 = {std::make_unique<int>(), 1};
    list.push_back(std::move(item1));

    UnCopyableItem item1Again = std::move(list.pop_back());

    UnCopyableItem item3 = {std::make_unique<int>(), 3};
    list.push_back(std::move(item3));

    EXPECT_EQ(item1Again.value, 1);

    UnCopyableItem item3Again = std::move(list.pop_back());
    EXPECT_EQ(item3Again.value, 3);
}

TEST(IndexedListTests, Insert) {
    IndexedList<int> list;
    list.push_back(0);
    list.push_back(1);
    list.push_back(2);
    list.push_back(3);
    list.push_back(4);
    EXPECT_EQ(list.size(), 5u);
}

TEST(IndexedListTests, Remove) {
    IndexedList<int> list;
    list.push_back(0);
    list.push_back(1);
    list.push_back(2);
    list.push_back(3);
    list.push_back(4);
    EXPECT_EQ(list.size(), 5u);

    // Before = [0,1,2,3,4]
    // After = [0,1,4,3]
    list.erase(2);
    EXPECT_EQ(list.size(), 4u);

    // Before = [0,1,4,3]
    // After = [0,1,4]
    list.erase(3);
    EXPECT_EQ(list.size(), 3u);

    // Before = [0,1,4]
    // After = [1,4]
    list.erase(0);
    EXPECT_EQ(list.size(), 2u);

    // Before = [1,4]
    // After = [4]
    EXPECT_EQ(list.pop_back(), 1);
    EXPECT_EQ(list.size(), 1u);

    // Before = [4]
    // After = []
    EXPECT_EQ(list.pop_back(), 4);
    EXPECT_EQ(list.size(), 0u);
}
