#ifdef USE_MODULES
import vk_mem_alloc_hpp;
#else
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.hpp>
#include <vk_mem_alloc_raii.hpp>
#endif

#include <vk_mem_alloc_imported.hpp>
#include <vk_mem_alloc_static_assertions.hpp>
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

class Vulkan {
    static constexpr const char* LAYERS { "VK_LAYER_KHRONOS_validation" };
    static constexpr float QUEUE_PRIORITY[] { 1.0f };
    static constexpr vk::DeviceQueueCreateInfo QUEUE_CREATE_INFO { {}, 0, 1, QUEUE_PRIORITY };
public:

    vk::raii::Context context {};
    vk::raii::Instance instance { context, vk::InstanceCreateInfo { {}, {}, LAYERS, } };
    vk::raii::PhysicalDevice physicalDevice = instance.enumeratePhysicalDevices()[0];
    vk::raii::Device device { physicalDevice, vk::DeviceCreateInfo { {}, QUEUE_CREATE_INFO } };
};
