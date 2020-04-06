// Copyright 2018 The Dawn Authors
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

#ifndef DAWNNATIVE_VULKAN_RAY_TRACING_SHADER_BINDING_TABLE_H_
#define DAWNNATIVE_VULKAN_RAY_TRACING_SHADER_BINDING_TABLE_H_

#include "common/vulkan_platform.h"
#include "dawn_native/RayTracingShaderBindingTable.h"
#include "dawn_native/ResourceMemoryAllocation.h"

#include <vector>

namespace dawn_native { namespace vulkan {

    class Device;

    class RayTracingShaderBindingTable : public RayTracingShaderBindingTableBase {
      public:
        static ResultOrError<RayTracingShaderBindingTable*> Create(Device* device, const RayTracingShaderBindingTableDescriptor* descriptor);
        ~RayTracingShaderBindingTable();

        std::vector<VkPipelineShaderStageCreateInfo>& GetStages();
        std::vector<VkRayTracingShaderGroupCreateInfoNV>& GetGroups();

        VkBuffer GetGroupBufferHandle() const;
        ResourceMemoryAllocation GetGroupBufferResource() const;

        uint32_t GetShaderGroupHandleSize() const;

      private:
        using RayTracingShaderBindingTableBase::RayTracingShaderBindingTableBase;

        void DestroyImpl() override;

        std::vector<VkPipelineShaderStageCreateInfo> mStages;
        std::vector<VkRayTracingShaderGroupCreateInfoNV> mGroups;

        VkPhysicalDeviceRayTracingPropertiesNV mRayTracingProperties;

        // group handle buffer
        VkBuffer mGroupBuffer = VK_NULL_HANDLE;
        ResourceMemoryAllocation mGroupBufferResource;

        MaybeError ValidateGroupStageIndex(int32_t index, VkShaderStageFlagBits validStage) const;

        MaybeError Initialize(const RayTracingShaderBindingTableDescriptor* descriptor);
    };

}}  // namespace dawn_native::vulkan

#endif  // DAWNNATIVE_VULKAN_RAY_TRACING_SHADER_BINDING_TABLE_H_
