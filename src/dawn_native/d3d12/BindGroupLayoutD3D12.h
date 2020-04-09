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

#ifndef DAWNNATIVE_D3D12_BINDGROUPLAYOUTD3D12_H_
#define DAWNNATIVE_D3D12_BINDGROUPLAYOUTD3D12_H_

#include "dawn_native/BindGroupLayout.h"

#include "common/SlabAllocator.h"
#include "dawn_native/d3d12/d3d12_platform.h"

namespace dawn_native { namespace d3d12 {

    class BindGroup;
    class Device;

    class BindGroupLayout : public BindGroupLayoutBase {
      public:
        BindGroupLayout(Device* device, const BindGroupLayoutDescriptor* descriptor);

        BindGroup* AllocateBindGroup(Device* device, const BindGroupDescriptor* descriptor);
        void DeallocateBindGroup(BindGroup* bindGroup);

        enum DescriptorType {
            CBV,
            UAV,
            SRV,
            Sampler,
            AccelerationContainer,
            Count,
        };

        const std::array<uint32_t, kMaxBindingsPerGroup>& GetBindingOffsets() const;
        uint32_t GetCbvUavSrvDescriptorTableSize() const;
        uint32_t GetSamplerDescriptorTableSize() const;
        uint32_t GetCbvUavSrvDescriptorCount() const;
        uint32_t GetSamplerDescriptorCount() const;
        const D3D12_DESCRIPTOR_RANGE* GetCbvUavSrvDescriptorRanges() const;
        const D3D12_DESCRIPTOR_RANGE* GetSamplerDescriptorRanges() const;

      private:
        std::array<uint32_t, kMaxBindingsPerGroup> mBindingOffsets;
        std::array<uint32_t, DescriptorType::Count> mDescriptorCounts;
        D3D12_DESCRIPTOR_RANGE mRanges[DescriptorType::Count];

        SlabAllocator<BindGroup> mBindGroupAllocator;
    };

}}  // namespace dawn_native::d3d12

#endif  // DAWNNATIVE_D3D12_BINDGROUPLAYOUTD3D12_H_
