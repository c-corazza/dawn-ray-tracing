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

#ifndef DAWNNATIVE_D3D12_RAY_TRACING_ACCELERATION_CONTAINER_H_
#define DAWNNATIVE_D3D12_RAY_TRACING_ACCELERATION_CONTAINER_H_

#include "dawn_native/RayTracingAccelerationContainer.h"
#include "dawn_native/d3d12/BufferD3D12.h"
#include "dawn_native/d3d12/ResourceHeapAllocationD3D12.h"
#include "dawn_native/d3d12/d3d12_platform.h"

namespace dawn_native { namespace d3d12 {

    class Device;

    struct SBTEntry {
        SBTEntry(std::wstring entryPoint, std::vector<void*> data);
        const std::vector<void*> mData;
        const std::wstring mEntryPoint;
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
        ~RayTracingAccelerationContainer() override;

        ScratchMemoryPool& GetScratchMemory();
        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& GetBuildInformation();

        void DestroyScratchBuildMemory();

      private:
        using RayTracingAccelerationContainerBase::RayTracingAccelerationContainerBase;

        void DestroyImpl() override;
        MaybeError UpdateInstanceImpl(
            uint32_t instanceIndex,
            const RayTracingAccelerationInstanceDescriptor* descriptor) override;

        MaybeError AllocateScratchMemory(MemoryEntry& memoryEntry,
                                         uint64_t size,
                                         D3D12_RESOURCE_STATES initialResourceState);

        // scratch memory
        ScratchMemoryPool mScratchMemory;

        MemoryEntry mInstanceMemory;

        std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> mGeometries;
        std::vector<D3D12_RAYTRACING_INSTANCE_DESC> mInstances;

        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS mBuildInformation;

        MaybeError Initialize(const RayTracingAccelerationContainerDescriptor* descriptor);
    };

}}  // namespace dawn_native::d3d12

#endif  // DAWNNATIVE_D3D12_RAY_TRACING_ACCELERATION_CONTAINER_H_