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

#include "dawn_native/vulkan/RayTracingShaderBindingTableVk.h"
#include "dawn_native/vulkan/ResourceHeapVk.h"
#include "dawn_native/vulkan/StagingBufferVk.h"
#include "dawn_native/vulkan/ShaderModuleVk.h"

#include "dawn_native/vulkan/DeviceVk.h"
#include "dawn_native/vulkan/VulkanError.h"
#include "dawn_native/vulkan/AdapterVk.h"
#include "dawn_native/vulkan/UtilsVulkan.h"

namespace dawn_native { namespace vulkan {

    namespace {

    }  // anonymous namespace

    // static
    ResultOrError<RayTracingShaderBindingTable*> RayTracingShaderBindingTable::Create(
        Device* device,
        const RayTracingShaderBindingTableDescriptor* descriptor) {
        std::unique_ptr<RayTracingShaderBindingTable> geometry =
            std::make_unique<RayTracingShaderBindingTable>(device, descriptor);
        DAWN_TRY(geometry->Initialize(descriptor));
        return geometry.release();
    }

    MaybeError RayTracingShaderBindingTable::Initialize(
        const RayTracingShaderBindingTableDescriptor* descriptor) {
        Device* device = ToBackend(GetDevice());
        Adapter* adapter = ToBackend(device->GetAdapter());

        // validate ray tracing calls
        if (device->fn.GetRayTracingShaderGroupHandlesNV == nullptr) {
            return DAWN_VALIDATION_ERROR("Invalid Call to GetRayTracingShaderGroupHandlesNV");
        }

        mRayTracingProperties = GetRayTracingProperties(*adapter);

        if (descriptor->shaderCount > 0) {
            for (unsigned int ii = 0; ii < descriptor->shaderCount; ++ii) {
                RayTracingShaderBindingTableShadersDescriptor shader = descriptor->shaders[ii];
                auto type = VK_SHADER_UNUSED_NV;
                auto generalShader = VK_SHADER_UNUSED_NV;
                auto closestHitShader = VK_SHADER_UNUSED_NV;
                auto anyHitShader = VK_SHADER_UNUSED_NV;
                auto intersectionShader = VK_SHADER_UNUSED_NV;
                switch (shader.stage) {
                    case wgpu::ShaderStage::RayGeneration:
                        type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
                        generalShader = mGroups.size();
                        mRayGenerationCount++;
                        break;
                    case wgpu::ShaderStage::RayAnyHit:
                        type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;
                        anyHitShader = mGroups.size();
                        mRayAnyHitCount++;
                        break;
                    case wgpu::ShaderStage::RayClosestHit:
                        type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;
                        closestHitShader = mGroups.size();
                        mRayClosestHitCount++;
                        break;
                    case wgpu::ShaderStage::RayMiss:
                        type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
                        generalShader = mGroups.size();
                        mRayMissCount++;
                        break;
                };

                VkRayTracingShaderGroupCreateInfoNV stageInfo{};
                stageInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
                stageInfo.type = (VkRayTracingShaderGroupTypeNV) type;
                stageInfo.generalShader = generalShader;
                stageInfo.closestHitShader = closestHitShader;
                stageInfo.anyHitShader = anyHitShader;
                stageInfo.intersectionShader = intersectionShader;
                mGroups.push_back(stageInfo);

                VkPipelineShaderStageCreateInfo pipelineShaderStageInfo{};
                pipelineShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                pipelineShaderStageInfo.stage =
                    static_cast<VkShaderStageFlagBits>(VulkanShaderStageFlags(shader.stage));
                pipelineShaderStageInfo.module = ToBackend(shader.module)->GetHandle();
                pipelineShaderStageInfo.pName = "main";

                mStages.push_back(pipelineShaderStageInfo);
            };
        }

        return {};
    }

    RayTracingShaderBindingTable::~RayTracingShaderBindingTable() {

    }

    std::vector<VkRayTracingShaderGroupCreateInfoNV>& RayTracingShaderBindingTable::GetGroups() {
        return mGroups;
    }

    std::vector<VkPipelineShaderStageCreateInfo>& RayTracingShaderBindingTable::GetStages() {
        return mStages;
    }

    uint32_t RayTracingShaderBindingTable::GetShaderGroupHandleSize() const {
        return mRayTracingProperties.shaderGroupHandleSize;
    }

    uint32_t RayTracingShaderBindingTable::GetOffsetImpl(wgpu::ShaderStage stageKind) {
        uint32_t offset = 0;
        switch (stageKind) {
            // 0
            case wgpu::ShaderStage::RayGeneration: {
                offset = 0;
            } break;
            // 1
            case wgpu::ShaderStage::RayClosestHit: {
                offset = mRayGenerationCount;
            } break;
            // 2
            case wgpu::ShaderStage::RayAnyHit: {
                offset = mRayGenerationCount + mRayClosestHitCount;
            } break;
            // 3
            case wgpu::ShaderStage::RayMiss: {
                offset = mRayGenerationCount + mRayClosestHitCount + mRayAnyHitCount;
            } break;
        };
        return offset * mRayTracingProperties.shaderGroupHandleSize;
    }

}}  // namespace dawn_native::vulkan
