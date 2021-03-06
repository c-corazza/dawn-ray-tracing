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

#ifndef DAWNNATIVE_VULKAN_VULKANINFO_H_
#define DAWNNATIVE_VULKAN_VULKANINFO_H_

#include <vector>

#include "common/vulkan_platform.h"
#include "dawn_native/Error.h"

namespace dawn_native { namespace vulkan {

    class Adapter;
    class Backend;

    extern const char kLayerNameKhronosValidation[];
    extern const char kLayerNameLunargVKTrace[];
    extern const char kLayerNameRenderDocCapture[];
    extern const char kLayerNameFuchsiaImagePipeSwapchain[];

    extern const char kExtensionNameExtDebugMarker[];
    extern const char kExtensionNameExtDebugUtils[];
    extern const char kExtensionNameExtDebugReport[];
    extern const char kExtensionNameExtMetalSurface[];
    extern const char kExtensionNameKhrExternalMemory[];
    extern const char kExtensionNameKhrExternalMemoryCapabilities[];
    extern const char kExtensionNameKhrExternalMemoryFD[];
    extern const char kExtensionNameExtExternalMemoryDmaBuf[];
    extern const char kExtensionNameExtImageDrmFormatModifier[];
    extern const char kExtensionNameFuchsiaExternalMemory[];
    extern const char kExtensionNameKhrExternalSemaphore[];
    extern const char kExtensionNameKhrExternalSemaphoreCapabilities[];
    extern const char kExtensionNameKhrExternalSemaphoreFD[];
    extern const char kExtensionNameFuchsiaExternalSemaphore[];
    extern const char kExtensionNameKhrGetPhysicalDeviceProperties2[];
    extern const char kExtensionNameKhrSurface[];
    extern const char kExtensionNameKhrSwapchain[];
    extern const char kExtensionNameKhrWaylandSurface[];
    extern const char kExtensionNameKhrWin32Surface[];
    extern const char kExtensionNameKhrXcbSurface[];
    extern const char kExtensionNameKhrXlibSurface[];
    extern const char kExtensionNameFuchsiaImagePipeSurface[];
    extern const char kExtensionNameKhrMaintenance1[];
    extern const char kExtensionNameKhrRayTracing[];
    extern const char kExtensionNameKhrGetMemoryRequirements2[];
    extern const char kExtensionNameExtDescriptorIndexing[];
    extern const char kExtensionNameKhrDeferredHostOperations[];
    extern const char kExtensionNameKhrPipelineLibrary[];
    extern const char kExtensionNameKhrBufferDeviceAddress[];
    extern const char kExtensionNameKhrShaderFloat16Int8[];
    extern const char kExtensionNameKhr16BitStorage[];

    // Global information - gathered before the instance is created
    struct VulkanGlobalKnobs {
        // Layers
        bool validation = false;
        bool vktrace = false;
        bool renderDocCapture = false;
        bool fuchsiaImagePipeSwapchain = false;

        // Extensions
        bool debugUtils = false;
        bool debugReport = false;
        bool externalMemoryCapabilities = false;
        bool externalSemaphoreCapabilities = false;
        bool getPhysicalDeviceProperties2 = false;
        bool metalSurface = false;
        bool surface = false;
        bool waylandSurface = false;
        bool win32Surface = false;
        bool xcbSurface = false;
        bool xlibSurface = false;
        bool fuchsiaImagePipeSurface = false;
    };

    struct VulkanGlobalInfo : VulkanGlobalKnobs {
        std::vector<VkLayerProperties> layers;
        std::vector<VkExtensionProperties> extensions;
        uint32_t apiVersion;
        // TODO(cwallez@chromium.org): layer instance extensions
    };

    // Device information - gathered before the device is created.
    struct VulkanDeviceKnobs {
        VkPhysicalDeviceFeatures features;
        VkPhysicalDeviceShaderFloat16Int8FeaturesKHR shaderFloat16Int8Features;
        VkPhysicalDevice16BitStorageFeaturesKHR _16BitStorageFeatures;

        bool debugUtils = false;
        bool debugMarker = false;
        bool externalMemory = false;
        bool externalMemoryFD = false;
        bool externalMemoryDmaBuf = false;
        bool imageDrmFormatModifier = false;
        bool externalMemoryZirconHandle = false;
        bool externalSemaphore = false;
        bool externalSemaphoreFD = false;
        bool externalSemaphoreZirconHandle = false;
        bool swapchain = false;
        bool maintenance1 = false;
        bool rayTracingKHR = false;
        bool memoryRequirements2 = false;
        bool descriptorIndexing = false;
        bool deferredHostOperations = false;
        bool pipelineLibrary = false;
        bool bufferDeviceAddress = false;
        bool shaderFloat16Int8 = false;
        bool _16BitStorage = false;
    };

    struct VulkanDeviceInfo : VulkanDeviceKnobs {
        VkPhysicalDeviceProperties properties;
        std::vector<VkQueueFamilyProperties> queueFamilies;

        std::vector<VkMemoryType> memoryTypes;
        std::vector<VkMemoryHeap> memoryHeaps;

        std::vector<VkLayerProperties> layers;
        std::vector<VkExtensionProperties> extensions;
        // TODO(cwallez@chromium.org): layer instance extensions
    };

    struct VulkanSurfaceInfo {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
        std::vector<bool> supportedQueueFamilies;
    };

    ResultOrError<VulkanGlobalInfo> GatherGlobalInfo(const Backend& backend);
    ResultOrError<std::vector<VkPhysicalDevice>> GetPhysicalDevices(const Backend& backend);
    ResultOrError<VulkanDeviceInfo> GatherDeviceInfo(const Adapter& adapter);

    VkPhysicalDeviceRayTracingPropertiesKHR GetPhysicalDeviceRayTracingProperties(
        const Adapter& adapter);

    ResultOrError<VulkanSurfaceInfo> GatherSurfaceInfo(const Adapter& adapter,
                                                       VkSurfaceKHR surface);
}}  // namespace dawn_native::vulkan

#endif  // DAWNNATIVE_VULKAN_VULKANINFO_H_
