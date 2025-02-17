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

#include <gtest/gtest.h>

#include "gpgmm/common/TraceEvent.h"

#include <thread>
#include <vector>

static constexpr const char* kDummyTrace = "DummyTrace.json";

using namespace gpgmm;

class EventTraceWriterTests : public testing::Test {
  public:
    void SetUp() override {
        StartupEventTrace(kDummyTrace, TraceEventPhase::None);
    }

    void TearDown() override {
        FlushEventTraceToDisk();
    }
};

TEST_F(EventTraceWriterTests, SingleThreadWrites) {
    constexpr uint32_t kEventCount = 64u;
    for (size_t i = 0; i < kEventCount; i++) {
        TRACE_EVENT_INSTANT0(TraceEventCategory::kDefault, "InstantEvent");
    }

    // 1 event per thread + 1 metadata event for main thread name.
    EXPECT_EQ(GetQueuedEventsForTesting(), 64 + 1u);
}

TEST_F(EventTraceWriterTests, MultiThreadWrites) {
    constexpr uint32_t kThreadCount = 64u;
    std::vector<std::thread> threads(kThreadCount);
    for (size_t threadIdx = 0; threadIdx < threads.size(); threadIdx++) {
        threads[threadIdx] = std::thread(
            [&]() { TRACE_EVENT_INSTANT0(TraceEventCategory::kDefault, "InstantEvent"); });
    }

    for (std::thread& thread : threads) {
        thread.join();
    }

    // 1 event per thread + 1 metadata event for main thread name.
    EXPECT_EQ(GetQueuedEventsForTesting(), 64 + 1u);
}
