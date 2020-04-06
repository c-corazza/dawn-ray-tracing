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

#ifndef DAWNNATIVE_VULKAN_RAY_TRACING_ACCELERATION_CONTAINER_H_
#define DAWNNATIVE_VULKAN_RAY_TRACING_ACCELERATION_CONTAINER_H_

#include <vector>

#include "common/vulkan_platform.h"
#include "dawn_native/RayTracingAccelerationContainer.h"
#include "dawn_native/vulkan/BufferVk.h"

namespace dawn_native { namespace vulkan {

    class Device;

    struct VkAccelerationInstance {
        float transform[12];
        uint32_t instanceId : 24;
        uint32_t mask : 8;
        uint32_t instanceOffset : 24;
        uint32_t flags : 8;
        uint64_t accelerationStructureHandle;
    };

    struct ScratchMemoryPool {
        MemoryEntry result;
        MemoryEntry update;
        MemoryEntry build;
    };

    class RayTracingAccelerationContainer : public RayTracingAccelerationContainerBase {
      public:
        static ResultOrError<RayTracingAccelerationContainer*> Create(
            Device* device,
            const RayTracingAccelerationContainerDescriptor* descriptor);
        ~RayTracingAccelerationContainer();

        uint64_t GetHandle() const;
        VkAccelerationStructureNV GetAccelerationStructure() const;
        VkMemoryRequirements2 GetMemoryRequirements(
            VkAccelerationStructureMemoryRequirementsTypeNV type) const;
        uint64_t GetMemoryRequirementSize(
            VkAccelerationStructureMemoryRequirementsTypeNV type) const;

        uint32_t GetInstanceCount() const;

        std::vector<VkGeometryNV>& GetGeometries();

        MemoryEntry& GetInstanceMemory();

        ScratchMemoryPool& GetScratchMemory();
        void DestroyScratchBuildMemory();

      private:
        using RayTracingAccelerationContainerBase::RayTracingAccelerationContainerBase;

        void DestroyImpl() override;
        uint64_t GetHandleImpl() override;
        MaybeError UpdateInstanceImpl(
            uint32_t instanceIndex,
            const RayTracingAccelerationInstanceDescriptor* descriptor) override;

        std::vector<VkGeometryNV> mGeometries;
        std::vector<VkAccelerationInstance> mInstances;

        // AS related
        VkAccelerationStructureNV mAccelerationStructure = VK_NULL_HANDLE;

        // scratch memory
        ScratchMemoryPool mScratchMemory;

        // instance buffer
        MemoryEntry mInstanceMemory;
        uint32_t mInstanceCount;

        MaybeError CreateAccelerationStructure(
            const RayTracingAccelerationContainerDescriptor* descriptor);

        MaybeError ReserveScratchMemory(
            const RayTracingAccelerationContainerDescriptor* descriptor);
        MaybeError AllocateScratchMemory(MemoryEntry& memoryEntry,
                                         VkMemoryRequirements& requirements);

        uint64_t mHandle;
        MaybeError FetchHandle(uint64_t* handle);

        MaybeError Initialize(const RayTracingAccelerationContainerDescriptor* descriptor);
    };

}}  // namespace dawn_native::vulkan

#endif  // DAWNNATIVE_VULKAN_RAY_TRACING_ACCELERATION_CONTAINER_H_