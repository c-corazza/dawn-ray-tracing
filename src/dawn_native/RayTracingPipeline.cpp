// Copyright 2017 The Dawn Authors
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

#include "dawn_native/RayTracingPipeline.h"

#include "common/HashUtils.h"
#include "dawn_native/Device.h"

namespace dawn_native {

    MaybeError ValidateRayTracingPipelineDescriptor(
        DeviceBase* device,
        const RayTracingPipelineDescriptor* descriptor) {
        if (descriptor->layout != nullptr) {
            DAWN_TRY(device->ValidateObject(descriptor->layout));
        }

        if (descriptor->rayTracingState == nullptr) {
            return DAWN_VALIDATION_ERROR("Ray Tracing state must not be empty");
        }

        if (descriptor->rayTracingState->shaderBindingTable == nullptr) {
            return DAWN_VALIDATION_ERROR("Missing Shader Binding Table");
        }

        if (descriptor->rayTracingState->shaderBindingTable == nullptr) {
            return DAWN_VALIDATION_ERROR("Shader Binding Table must not be destroyed");
        }

        return {};
    }

    // RayTracingPipelineBase

    RayTracingPipelineBase::RayTracingPipelineBase(DeviceBase* device,
                                                   const RayTracingPipelineDescriptor* descriptor)
        : PipelineBase(device, descriptor->layout),
          mShaderBindingTable(descriptor->rayTracingState->shaderBindingTable) {
    }

    RayTracingPipelineBase::RayTracingPipelineBase(DeviceBase* device, ObjectBase::ErrorTag tag)
        : PipelineBase(device, tag) {
    }

    RayTracingPipelineBase::~RayTracingPipelineBase() {
    }

    // static
    RayTracingPipelineBase* RayTracingPipelineBase::MakeError(DeviceBase* device) {
        return new RayTracingPipelineBase(device, ObjectBase::kError);
    }

    RayTracingShaderBindingTableBase* RayTracingPipelineBase::GetShaderBindingTable() {
        return mShaderBindingTable.Get();
    }

}  // namespace dawn_native
