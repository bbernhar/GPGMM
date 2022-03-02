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

#ifndef GPGMM_SERIALIZER_H_
#define GPGMM_SERIALIZER_H_

#include "gpgmm/TraceEvent.h"
#include "gpgmm/common/JSONEncoder.h"
#include "gpgmm/common/Log.h"

namespace gpgmm {

    // Messages of a given severity to be recorded.
    // Set the new level and returns the previous level so it may be restored by the caller.
    LogSeverity SetRecordEventLevel(const LogSeverity& level);
    const LogSeverity& GetRecordEventLevel();

    // Forward declare common types.
    struct ALLOCATOR_MESSAGE;
    struct POOL_INFO;
    struct MEMORY_ALLOCATOR_INFO;

    class Serializer {
      public:
        static JSONDict Serialize(const ALLOCATOR_MESSAGE& desc);
        static JSONDict Serialize(const MEMORY_ALLOCATOR_INFO& info);
        static JSONDict Serialize(const POOL_INFO& desc);
        static JSONDict Serialize(void* objectPtr);
    };

    template <typename T, typename DescT, typename SerializerT = Serializer>
    static void RecordObject(const char* name, T* objPtr, const DescT& desc) {
        if (IsEventTracerEnabled()) {
            const auto& args = SerializerT::Serialize(desc);
            TRACE_EVENT_OBJECT_SNAPSHOT_WITH_ID(name, objPtr, args);
        }
    }

    template <typename T, typename SerializerT>
    static void RecordEvent(const char* name, const T& obj) {
        if (IsEventTracerEnabled()) {
            const auto& args = SerializerT::Serialize(obj);
            TRACE_EVENT_INSTANT(name, args);
        }
    }

    template <typename T, typename SerializerT, typename... Args>
    static void RecordCall(const char* name, const Args&... args) {
        if (IsEventTracerEnabled()) {
            const T& obj{args...};
            return RecordEvent<T, SerializerT>(name, obj);
        }
    }

    template <typename T, typename SerializerT, typename... Args>
    static void RecordMessage(const LogSeverity& severity, const char* name, const Args&... args) {
        const T& obj{args...};
        if (severity >= GetLogMessageLevel()) {
            gpgmm::Log(severity) << name << SerializerT::Serialize(obj).ToString();
        }
        if (severity >= GetRecordEventLevel()) {
            return RecordEvent<T, SerializerT>(name, obj);
        }
    }

    template <typename... Args>
    static void RecordMessage(const LogSeverity& severity, const char* name, const Args&... args) {
        return gpgmm::RecordMessage<ALLOCATOR_MESSAGE, Serializer>(severity, name, args...);
    }

}  // namespace gpgmm

#endif  // GPGMM_SERIALIZER_H_
