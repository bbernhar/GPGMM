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
//

#include "src/tests/capture_replay_tests/GPGMMCaptureReplayTests.h"

#include <fstream>
#include <string>
#include <unordered_map>

#include <gpgmm_d3d12.h>
#include <json/json.h>

#include "src/TraceEvent.h"
#include "src/common/SystemUtils.h"
#include "src/tests/D3D12Test.h"

enum TraceEventName {
    CreateResource,
    ResourceAllocation,
    DeallocateMemory,
};

namespace {

    std::string GetTraceEventName(const TraceEventName& traceEventName) {
        switch (traceEventName) {
            case TraceEventName::CreateResource:
                return "CreateResource";
            case TraceEventName::ResourceAllocation:
                return "ResourceAllocation";
            case TraceEventName::DeallocateMemory:
                return "DeallocateMemory";
            default:
                UNREACHABLE();
        }
        return "";
    }

    gpgmm::d3d12::ALLOCATION_DESC ConvertToGPGMMAllocationDesc(
        const Json::Value& allocationDescriptorJsonValue) {
        gpgmm::d3d12::ALLOCATION_DESC allocationDescriptor = {};
        allocationDescriptor.Flags = static_cast<gpgmm::d3d12::ALLOCATION_FLAGS>(
            allocationDescriptorJsonValue["Flags"].asInt());
        allocationDescriptor.HeapType =
            static_cast<D3D12_HEAP_TYPE>(allocationDescriptorJsonValue["HeapType"].asInt());
        return allocationDescriptor;
    }

    D3D12_CLEAR_VALUE ConvertToD3D12ClearValue(const Json::Value& clearValueJsonValue) {
        D3D12_CLEAR_VALUE clearValue = {};
        clearValue.Format = static_cast<DXGI_FORMAT>(clearValueJsonValue["Format"].asInt());
        // TODO: union of color - depthstencil.
        return clearValue;
    }

    D3D12_RESOURCE_DESC ConvertToD3D12ResourceDesc(const Json::Value& resourceDescriptorJsonValue) {
        D3D12_RESOURCE_DESC resourceDescriptor = {};
        resourceDescriptor.Dimension =
            static_cast<D3D12_RESOURCE_DIMENSION>(resourceDescriptorJsonValue["Dimension"].asInt());
        resourceDescriptor.Alignment = resourceDescriptorJsonValue["Alignment"].asUInt64();
        resourceDescriptor.Width = resourceDescriptorJsonValue["Width"].asUInt64();
        resourceDescriptor.Height = resourceDescriptorJsonValue["Height"].asUInt();
        resourceDescriptor.DepthOrArraySize =
            resourceDescriptorJsonValue["DepthOrArraySize"].asUInt();
        resourceDescriptor.MipLevels = resourceDescriptorJsonValue["MipLevels"].asUInt();

        const Json::Value& resourceDescriptorSampleDescJsonValue =
            resourceDescriptorJsonValue["SampleDesc"];

        resourceDescriptor.SampleDesc.Count =
            resourceDescriptorSampleDescJsonValue["Count"].asUInt();
        resourceDescriptor.SampleDesc.Quality =
            resourceDescriptorSampleDescJsonValue["Quality"].asUInt();

        resourceDescriptor.Format =
            static_cast<DXGI_FORMAT>(resourceDescriptorJsonValue["Format"].asInt());
        resourceDescriptor.Layout =
            static_cast<D3D12_TEXTURE_LAYOUT>(resourceDescriptorJsonValue["Layout"].asInt());
        resourceDescriptor.Flags =
            static_cast<D3D12_RESOURCE_FLAGS>(resourceDescriptorJsonValue["Flags"].asInt());

        return resourceDescriptor;
    }

}  // namespace

class D3D12EventTraceReplay : public D3D12GPGMMTest, public CaptureReplyTestWithParams {
  protected:
    void SetUp() override {
        D3D12GPGMMTest::SetUp();
    }

    void RunTest(const TraceFile& traceFile) {
        Json::Value root;  // will contains the root value after parsing.
        Json::Reader reader;

        std::ifstream traceFileStream(traceFile.path, std::ifstream::binary);

        ASSERT_TRUE(reader.parse(traceFileStream, root, false));

        std::unordered_map<std::string, ComPtr<gpgmm::d3d12::ResourceAllocation>> allocationToIDMap;

        ComPtr<gpgmm::d3d12::ResourceAllocation> newAllocationWithoutID;

        const Json::Value& traceEvents = root["traceEvents"];

        for (Json::Value::ArrayIndex eventIndex = 0; eventIndex < traceEvents.size();
             eventIndex++) {
            const Json::Value& event = traceEvents[eventIndex];
            if (event["name"].asString() == GetTraceEventName(TraceEventName::CreateResource)) {
                const Json::Value& args = event["args"];

                const gpgmm::d3d12::ALLOCATION_DESC allocationDescriptor =
                    ConvertToGPGMMAllocationDesc(args["allocationDescriptor"]);

                const D3D12_RESOURCE_STATES initialUsage =
                    static_cast<D3D12_RESOURCE_STATES>(args["initialUsage"].asInt());

                const D3D12_CLEAR_VALUE* clearValuePtr = nullptr;
                D3D12_CLEAR_VALUE clearValue = {};
                const Json::Value& clearValueJsonValue = args["clearValue"];
                if (!clearValueJsonValue.empty()) {
                    clearValue = ConvertToD3D12ClearValue(clearValueJsonValue);
                    clearValuePtr = &clearValue;
                }

                const D3D12_RESOURCE_DESC resourceDescriptor =
                    ConvertToD3D12ResourceDesc(args["resourceDescriptor"]);

                ASSERT_TRUE(SUCCEEDED(mResourceAllocator->CreateResource(
                    allocationDescriptor, resourceDescriptor, initialUsage, clearValuePtr,
                    &newAllocationWithoutID)));

            } else if (event["name"].asString() ==
                       GetTraceEventName(TraceEventName::ResourceAllocation)) {
                switch (*event["ph"].asCString()) {
                    case TRACE_EVENT_PHASE_CREATE_OBJECT: {
                        ASSERT_TRUE(newAllocationWithoutID != nullptr);
                        const std::string& traceEventID = event["id"].asString();
                        ASSERT_TRUE(allocationToIDMap.insert({traceEventID, newAllocationWithoutID})
                                        .second);
                        ASSERT_TRUE(newAllocationWithoutID.Reset() == 1);
                    } break;

                    case TRACE_EVENT_PHASE_DELETE_OBJECT: {
                        const std::string& traceEventID = event["id"].asString();
                        allocationToIDMap.erase(traceEventID);
                    } break;

                    default:
                        break;
                }
            }
        }

        ASSERT_TRUE(allocationToIDMap.empty());
    }
};

TEST_P(D3D12EventTraceReplay, Run) {
    RunTest(GetParam());
}

GPGMM_INSTANTIATE_CAPTURE_REPLAY_TEST(D3D12EventTraceReplay);
