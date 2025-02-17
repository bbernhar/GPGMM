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

#ifndef GPGMM_COMMON_EVENT_MESSAGE_H_
#define GPGMM_COMMON_EVENT_MESSAGE_H_

#include "gpgmm/common/TraceEvent.h"
#include "gpgmm/utils/Log.h"

#include <sstream>
#include <string>

namespace gpgmm {

    enum class EventMessageId {
        kUnknown,
        kSizeExceeded,
        kAlignmentMismatch,
        kAllocatorFailed,
        kPrefetchFailed,
        kBudgetExceeded,
        kBudgetUpdated,
        kBudgetInvalid,
    };

    struct EventMessageInfo {
        std::string Description;
        EventMessageId ID;
    };

    class EventMessage {
      public:
        EventMessage(const LogSeverity& level,
                     const char* name,
                     EventMessageId messageId = EventMessageId::kUnknown);
        ~EventMessage();

        EventMessage(EventMessage&& other) = default;
        EventMessage& operator=(EventMessage&& other) = default;

        template <typename T>
        EventMessage& operator<<(T&& value) {
            mStream << value;
            return *this;
        }

      private:
        LogSeverity mSeverity;
        const char* mName = nullptr;
        EventMessageId mMessageId = EventMessageId::kUnknown;

        std::ostringstream mStream;
    };

    EventMessage DebugEvent(const char* name, EventMessageId messageId = EventMessageId::kUnknown);
    EventMessage InfoEvent(const char* name, EventMessageId messageId = EventMessageId::kUnknown);
    EventMessage WarnEvent(const char* name, EventMessageId messageId = EventMessageId::kUnknown);
    EventMessage ErrorEvent(const char* name, EventMessageId messageId = EventMessageId::kUnknown);

    // Messages of a given severity to be recorded.
    void SetEventMessageLevel(const LogSeverity& level);

}  // namespace gpgmm

#endif  // GPGMM_COMMON_EVENT_MESSAGE_H_
