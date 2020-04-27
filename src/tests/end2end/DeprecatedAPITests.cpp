// Copyright 2020 The Dawn Authors
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

// This file contains test for deprecated parts of Dawn's API while following WebGPU's evolution.
// It contains test for the "old" behavior that will be deleted once users are migrated, tests that
// a deprecation warning is emitted when the "old" behavior is used, and tests that an error is
// emitted when both the old and the new behavior are used (when applicable).

#include "tests/DawnTest.h"

#include "common/Constants.h"
#include "utils/ComboRenderPipelineDescriptor.h"
#include "utils/WGPUHelpers.h"

class DeprecationTests : public DawnTest {
  protected:
    void TestSetUp() override {
        // Skip when validation is off because warnings might be emitted during validation calls
        DAWN_SKIP_TEST_IF(IsDawnValidationSkipped());
    }

    void TearDown() override {
        if (!UsesWire()) {
            EXPECT_EQ(mLastWarningCount,
                      dawn_native::GetDeprecationWarningCountForTesting(device.Get()));
        }
    }

    size_t mLastWarningCount = 0;
};

#define EXPECT_DEPRECATION_WARNING(statement)                                    \
    do {                                                                         \
        if (UsesWire()) {                                                        \
            statement;                                                           \
        } else {                                                                 \
            size_t warningsBefore =                                              \
                dawn_native::GetDeprecationWarningCountForTesting(device.Get()); \
            statement;                                                           \
            size_t warningsAfter =                                               \
                dawn_native::GetDeprecationWarningCountForTesting(device.Get()); \
            EXPECT_EQ(mLastWarningCount, warningsBefore);                        \
            EXPECT_EQ(warningsAfter, warningsBefore + 1);                        \
            mLastWarningCount = warningsAfter;                                   \
        }                                                                        \
    } while (0)

// Tests for Device::CreateQueue -> Device::GetDefaultQueue.

// Test that using CreateQueue produces a deprecation warning
TEST_P(DeprecationTests, CreateQueueIsDeprecated) {
    EXPECT_DEPRECATION_WARNING(device.CreateQueue());
}

// Test that queues created from CreateQueue can be used for things
TEST_P(DeprecationTests, CreateQueueReturnsFunctionalQueue) {
    wgpu::Queue q;
    EXPECT_DEPRECATION_WARNING(q = device.CreateQueue());

    q.Submit(0, nullptr);
}

// Tests for BindGroupLayoutEntry::textureDimension -> viewDimension

// Test that creating a BGL with textureDimension produces a deprecation warning.
TEST_P(DeprecationTests, BGLEntryTextureDimensionIsDeprecated) {
    wgpu::BindGroupLayoutEntry entryDesc;
    entryDesc.binding = 0;
    entryDesc.visibility = wgpu::ShaderStage::None;
    entryDesc.type = wgpu::BindingType::SampledTexture;
    entryDesc.textureDimension = wgpu::TextureViewDimension::e2D;

    wgpu::BindGroupLayoutDescriptor bglDesc;
    bglDesc.bindingCount = 0;
    bglDesc.bindings = nullptr;
    bglDesc.entryCount = 1;
    bglDesc.entries = &entryDesc;
    EXPECT_DEPRECATION_WARNING(device.CreateBindGroupLayout(&bglDesc));
}

// Test that creating a BGL with default viewDimension and textureDimension doesn't emit a warning
TEST_P(DeprecationTests, BGLEntryTextureDimensionAndViewUndefinedEmitsNoWarning) {
    wgpu::BindGroupLayoutEntry entryDesc;
    entryDesc.binding = 0;
    entryDesc.visibility = wgpu::ShaderStage::None;
    entryDesc.type = wgpu::BindingType::Sampler;

    wgpu::BindGroupLayoutDescriptor bglDesc;
    bglDesc.bindingCount = 0;
    bglDesc.bindings = nullptr;
    bglDesc.entryCount = 1;
    bglDesc.entries = &entryDesc;
    device.CreateBindGroupLayout(&bglDesc);
}
// Test that creating a BGL with both textureDimension and viewDimension is an error
TEST_P(DeprecationTests, BGLEntryTextureAndViewDimensionIsInvalid) {
    wgpu::BindGroupLayoutEntry entryDesc;
    entryDesc.binding = 0;
    entryDesc.visibility = wgpu::ShaderStage::None;
    entryDesc.type = wgpu::BindingType::SampledTexture;
    entryDesc.textureDimension = wgpu::TextureViewDimension::e2D;
    entryDesc.viewDimension = wgpu::TextureViewDimension::e2D;

    wgpu::BindGroupLayoutDescriptor bglDesc;
    bglDesc.bindingCount = 0;
    bglDesc.bindings = nullptr;
    bglDesc.entryCount = 1;
    bglDesc.entries = &entryDesc;
    ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&bglDesc));
}

// Test that creating a BGL with both textureDimension still does correct state tracking
TEST_P(DeprecationTests, BGLEntryTextureDimensionStateTracking) {
    // Create a BGL that expects a cube map
    wgpu::BindGroupLayoutEntry entryDesc = {};
    entryDesc.binding = 0;
    entryDesc.visibility = wgpu::ShaderStage::None;
    entryDesc.type = wgpu::BindingType::SampledTexture;
    entryDesc.textureDimension = wgpu::TextureViewDimension::Cube;

    wgpu::BindGroupLayoutDescriptor bglDesc = {};
    bglDesc.bindingCount = 0;
    bglDesc.bindings = nullptr;
    bglDesc.entryCount = 1;
    bglDesc.entries = &entryDesc;
    wgpu::BindGroupLayout layout;
    EXPECT_DEPRECATION_WARNING(layout = device.CreateBindGroupLayout(&bglDesc));

    // Create a 2D array view and a cube view
    wgpu::TextureDescriptor textureDesc = {};
    textureDesc.usage = wgpu::TextureUsage::Sampled;
    textureDesc.size = {1, 1, 1};
    textureDesc.arrayLayerCount = 6;
    textureDesc.format = wgpu::TextureFormat::RGBA8Unorm;
    wgpu::Texture texture = device.CreateTexture(&textureDesc);

    wgpu::TextureViewDescriptor viewDesc = {};
    viewDesc.dimension = wgpu::TextureViewDimension::e2DArray;
    viewDesc.baseArrayLayer = 0;
    viewDesc.arrayLayerCount = 6;
    wgpu::TextureView arrayView = texture.CreateView(&viewDesc);

    viewDesc.dimension = wgpu::TextureViewDimension::Cube;
    wgpu::TextureView cubeView = texture.CreateView(&viewDesc);

    // textureDimension is correctly taken into account and only the BindGroup with the Cube view is
    // valid.
    utils::MakeBindGroup(device, layout, {{0, cubeView}});
    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, layout, {{0, arrayView}}));
}

// Test for BindGroupLayout::bindings/bindingCount -> entries/entryCount

// Test that creating a BGL with bindings emits a deprecation warning.
TEST_P(DeprecationTests, BGLDescBindingIsDeprecated) {
    wgpu::BindGroupLayoutEntry entryDesc;
    entryDesc.binding = 0;
    entryDesc.visibility = wgpu::ShaderStage::None;
    entryDesc.type = wgpu::BindingType::Sampler;

    wgpu::BindGroupLayoutDescriptor bglDesc;
    bglDesc.bindingCount = 1;
    bglDesc.bindings = &entryDesc;
    bglDesc.entryCount = 0;
    bglDesc.entries = nullptr;
    EXPECT_DEPRECATION_WARNING(device.CreateBindGroupLayout(&bglDesc));
}

// Test that creating a BGL with both entries and bindings is an error
TEST_P(DeprecationTests, BGLDescBindingAndEntriesIsInvalid) {
    wgpu::BindGroupLayoutEntry entryDesc;
    entryDesc.binding = 0;
    entryDesc.visibility = wgpu::ShaderStage::None;
    entryDesc.type = wgpu::BindingType::Sampler;

    wgpu::BindGroupLayoutDescriptor bglDesc;
    bglDesc.bindingCount = 1;
    bglDesc.bindings = &entryDesc;
    bglDesc.entryCount = 1;
    bglDesc.entries = &entryDesc;
    ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&bglDesc));
}

// Test that creating a BGL with both entries and bindings to 0 doesn't emit warnings
TEST_P(DeprecationTests, BGLDescBindingAndEntriesBothZeroEmitsNoWarning) {
    wgpu::BindGroupLayoutDescriptor bglDesc;
    bglDesc.bindingCount = 0;
    bglDesc.bindings = nullptr;
    bglDesc.entryCount = 0;
    bglDesc.entries = nullptr;
    device.CreateBindGroupLayout(&bglDesc);
}

// Test that creating a BGL with bindings still does correct state tracking
TEST_P(DeprecationTests, BGLDescBindingStateTracking) {
    wgpu::BindGroupLayoutEntry entryDesc;
    entryDesc.binding = 0;
    entryDesc.type = wgpu::BindingType::Sampler;
    entryDesc.visibility = wgpu::ShaderStage::None;

    wgpu::BindGroupLayoutDescriptor bglDesc;
    bglDesc.bindingCount = 1;
    bglDesc.bindings = &entryDesc;
    bglDesc.entryCount = 0;
    bglDesc.entries = nullptr;
    wgpu::BindGroupLayout layout;
    EXPECT_DEPRECATION_WARNING(layout = device.CreateBindGroupLayout(&bglDesc));

    // Test a case where if |bindings| wasn't taken into account, no validation error would happen
    // because the layout would be empty
    wgpu::BindGroupDescriptor badBgDesc;
    badBgDesc.layout = layout;
    badBgDesc.bindingCount = 0;
    badBgDesc.bindings = nullptr;
    badBgDesc.entryCount = 0;
    badBgDesc.entries = nullptr;
    ASSERT_DEVICE_ERROR(device.CreateBindGroup(&badBgDesc));
}

// Test for BindGroup::bindings/bindingCount -> entries/entryCount

// Test that creating a BG with bindings emits a deprecation warning.
TEST_P(DeprecationTests, BGDescBindingIsDeprecated) {
    wgpu::SamplerDescriptor samplerDesc = {};
    wgpu::Sampler sampler = device.CreateSampler(&samplerDesc);

    wgpu::BindGroupLayout layout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::BindingType::Sampler}});

    wgpu::BindGroupEntry entryDesc;
    entryDesc.binding = 0;
    entryDesc.sampler = sampler;

    wgpu::BindGroupDescriptor bgDesc;
    bgDesc.layout = layout;
    bgDesc.bindingCount = 1;
    bgDesc.bindings = &entryDesc;
    bgDesc.entryCount = 0;
    bgDesc.entries = nullptr;
    EXPECT_DEPRECATION_WARNING(device.CreateBindGroup(&bgDesc));
}

// Test that creating a BG with both entries and bindings is an error
TEST_P(DeprecationTests, BGDescBindingAndEntriesIsInvalid) {
    wgpu::SamplerDescriptor samplerDesc = {};
    wgpu::Sampler sampler = device.CreateSampler(&samplerDesc);

    wgpu::BindGroupLayout layout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::BindingType::Sampler}});

    wgpu::BindGroupEntry entryDesc = {};
    entryDesc.binding = 0;
    entryDesc.sampler = sampler;

    wgpu::BindGroupDescriptor bgDesc;
    bgDesc.layout = layout;
    bgDesc.bindingCount = 1;
    bgDesc.bindings = &entryDesc;
    bgDesc.entryCount = 1;
    bgDesc.entries = &entryDesc;
    ASSERT_DEVICE_ERROR(device.CreateBindGroup(&bgDesc));
}

// Test that creating a BG with both entries and bindings to 0 doesn't emit warnings
TEST_P(DeprecationTests, BGDescBindingAndEntriesBothZeroEmitsNoWarning) {
    wgpu::BindGroupLayout layout = utils::MakeBindGroupLayout(device, {});

    wgpu::BindGroupDescriptor bgDesc;
    bgDesc.layout = layout;
    bgDesc.bindingCount = 0;
    bgDesc.bindings = nullptr;
    bgDesc.entryCount = 0;
    bgDesc.entries = nullptr;
    device.CreateBindGroup(&bgDesc);
}

// Test that creating a BG with bindings still does correct state tracking
TEST_P(DeprecationTests, BGDescBindingStateTracking) {
    wgpu::BindGroupLayout layout = utils::MakeBindGroupLayout(device, {});

    // Test a case where if |bindings| wasn't taken into account, no validation error would happen
    // because it would match the empty layout.
    wgpu::SamplerDescriptor samplerDesc = {};
    wgpu::Sampler sampler = device.CreateSampler(&samplerDesc);

    wgpu::BindGroupEntry entryDesc = {};
    entryDesc.binding = 0;
    entryDesc.sampler = sampler;

    wgpu::BindGroupDescriptor bgDesc;
    bgDesc.layout = layout;
    bgDesc.bindingCount = 1;
    bgDesc.bindings = &entryDesc;
    bgDesc.entryCount = 0;
    bgDesc.entries = nullptr;
    EXPECT_DEPRECATION_WARNING(ASSERT_DEVICE_ERROR(device.CreateBindGroup(&bgDesc)));
}

// Tests for ShaderModuleDescriptor.code/codeSize -> ShaderModuleSPIRVDescriptor

static const char kEmptyShader[] = R"(#version 450
void main() {
})";

// That creating a ShaderModule without the chained descriptor gives a warning.
TEST_P(DeprecationTests, ShaderModuleNoSubDescriptorIsDeprecated) {
    std::vector<uint32_t> spirv =
        CompileGLSLToSpirv(utils::SingleShaderStage::Compute, kEmptyShader);

    wgpu::ShaderModuleDescriptor descriptor;
    descriptor.codeSize = static_cast<uint32_t>(spirv.size());
    descriptor.code = spirv.data();
    EXPECT_DEPRECATION_WARNING(device.CreateShaderModule(&descriptor));
}

// That creating a ShaderModule with both inline code and the chained descriptor is an error.
TEST_P(DeprecationTests, ShaderModuleBothInlinedAndChainedIsInvalid) {
    std::vector<uint32_t> spirv =
        CompileGLSLToSpirv(utils::SingleShaderStage::Compute, kEmptyShader);

    wgpu::ShaderModuleSPIRVDescriptor spirvDesc;
    spirvDesc.codeSize = static_cast<uint32_t>(spirv.size());
    spirvDesc.code = spirv.data();

    wgpu::ShaderModuleDescriptor descriptor;
    descriptor.nextInChain = &spirvDesc;
    descriptor.codeSize = static_cast<uint32_t>(spirv.size());
    descriptor.code = spirv.data();
    ASSERT_DEVICE_ERROR(device.CreateShaderModule(&descriptor));
}

// That creating a ShaderModule with both inline code still does correct state tracking
TEST_P(DeprecationTests, ShaderModuleInlinedCodeStateTracking) {
    std::vector<uint32_t> spirv =
        CompileGLSLToSpirv(utils::SingleShaderStage::Compute, kEmptyShader);

    wgpu::ShaderModuleDescriptor descriptor;
    descriptor.codeSize = static_cast<uint32_t>(spirv.size());
    descriptor.code = spirv.data();
    wgpu::ShaderModule module;
    EXPECT_DEPRECATION_WARNING(module = device.CreateShaderModule(&descriptor));

    // Creating a compute pipeline works, because it is a compute module.
    wgpu::ComputePipelineDescriptor computePipelineDesc;
    computePipelineDesc.layout = nullptr;
    computePipelineDesc.computeStage.module = module;
    computePipelineDesc.computeStage.entryPoint = "main";
    device.CreateComputePipeline(&computePipelineDesc);

    utils::ComboRenderPipelineDescriptor renderPipelineDesc(device);
    renderPipelineDesc.vertexStage.module =
        utils::CreateShaderModule(device, utils::SingleShaderStage::Vertex, kEmptyShader);
    renderPipelineDesc.cFragmentStage.module = module;
    ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&renderPipelineDesc));
}

// Tests for BufferCopyView.rowPitch/imageHeight -> bytesPerRow/rowsPerImage

class BufferCopyViewDeprecationTests : public DeprecationTests {
  protected:
    void TestSetUp() override {
        DeprecationTests::TestSetUp();

        wgpu::BufferDescriptor bufferDesc;
        bufferDesc.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
        bufferDesc.size = kTextureBytesPerRowAlignment * 2;
        buffer = device.CreateBuffer(&bufferDesc);

        wgpu::TextureDescriptor textureDesc;
        textureDesc.usage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst;
        textureDesc.size = {2, 2, 1};
        textureDesc.format = wgpu::TextureFormat::RGBA8Unorm;
        texture = device.CreateTexture(&textureDesc);
    }

    enum CopyType {
        B2T,
        T2B,
    };
    void DoCopy(CopyType type, const wgpu::BufferCopyView& bufferView) {
        wgpu::TextureCopyView textureCopyView;
        textureCopyView.texture = texture;
        textureCopyView.mipLevel = 0;
        textureCopyView.arrayLayer = 0;
        textureCopyView.origin = {0, 0};
        wgpu::Extent3D copySize = {2, 2, 1};

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        switch (type) {
            case B2T:
                encoder.CopyBufferToTexture(&bufferView, &textureCopyView, &copySize);
                break;
            case T2B:
                encoder.CopyTextureToBuffer(&textureCopyView, &bufferView, &copySize);
                break;
        }
        encoder.Finish();
    }

    wgpu::Buffer buffer;
    wgpu::Texture texture;
};

// Test that using rowPitch produces a deprecation warning.
TEST_P(BufferCopyViewDeprecationTests, RowPitchIsDeprecated) {
    wgpu::BufferCopyView view;
    view.buffer = buffer;
    view.rowPitch = 256;
    EXPECT_DEPRECATION_WARNING(DoCopy(B2T, view));
    EXPECT_DEPRECATION_WARNING(DoCopy(T2B, view));
}

// Test that using imageHeight produces a deprecation warning.
TEST_P(BufferCopyViewDeprecationTests, ImageHeightIsDeprecated) {
    wgpu::BufferCopyView view;
    view.buffer = buffer;
    view.imageHeight = 2;
    view.bytesPerRow = 256;
    EXPECT_DEPRECATION_WARNING(DoCopy(B2T, view));
    EXPECT_DEPRECATION_WARNING(DoCopy(T2B, view));
}

// Test that using both rowPitch and bytesPerRow produces a validation error.
TEST_P(BufferCopyViewDeprecationTests, BothRowPitchAndBytesPerRowIsInvalid) {
    wgpu::BufferCopyView view;
    view.buffer = buffer;
    view.rowPitch = 256;
    view.bytesPerRow = 256;
    ASSERT_DEVICE_ERROR(DoCopy(B2T, view));
    ASSERT_DEVICE_ERROR(DoCopy(T2B, view));
}

// Test that using both imageHeight and rowsPerImage produces a validation error.
TEST_P(BufferCopyViewDeprecationTests, BothImageHeightAndRowsPerImageIsInvalid) {
    wgpu::BufferCopyView view;
    view.buffer = buffer;
    view.imageHeight = 2;
    view.bytesPerRow = 256;
    view.rowsPerImage = 2;
    ASSERT_DEVICE_ERROR(DoCopy(B2T, view));
    ASSERT_DEVICE_ERROR(DoCopy(T2B, view));
}

// Test that rowPitch is correctly taken into account for validation
TEST_P(BufferCopyViewDeprecationTests, RowPitchTakenIntoAccountForValidation) {
    wgpu::BufferCopyView view;
    view.buffer = buffer;
    view.rowPitch = 256;
    EXPECT_DEPRECATION_WARNING(DoCopy(B2T, view));
    EXPECT_DEPRECATION_WARNING(DoCopy(T2B, view));

    view.rowPitch = 128;
    ASSERT_DEVICE_ERROR(EXPECT_DEPRECATION_WARNING(DoCopy(B2T, view)));
    ASSERT_DEVICE_ERROR(EXPECT_DEPRECATION_WARNING(DoCopy(T2B, view)));
}

// Test that imageHeight is correctly taken into account for validation
TEST_P(BufferCopyViewDeprecationTests, ImageHeightTakenIntoAccountForValidation) {
    wgpu::BufferCopyView view;
    view.buffer = buffer;
    view.imageHeight = 2;
    view.bytesPerRow = 256;
    EXPECT_DEPRECATION_WARNING(DoCopy(B2T, view));
    EXPECT_DEPRECATION_WARNING(DoCopy(T2B, view));

    view.imageHeight = 1;
    ASSERT_DEVICE_ERROR(EXPECT_DEPRECATION_WARNING(DoCopy(B2T, view)));
    ASSERT_DEVICE_ERROR(EXPECT_DEPRECATION_WARNING(DoCopy(T2B, view)));
}

DAWN_INSTANTIATE_TEST(BufferCopyViewDeprecationTests,
                      D3D12Backend(),
                      MetalBackend(),
                      NullBackend(),
                      OpenGLBackend(),
                      VulkanBackend());

DAWN_INSTANTIATE_TEST(DeprecationTests,
                      D3D12Backend(),
                      MetalBackend(),
                      NullBackend(),
                      OpenGLBackend(),
                      VulkanBackend());
