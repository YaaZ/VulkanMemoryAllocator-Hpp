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

template<class T> T value(T&& t) { return std::forward<T>(t); }
template<class T> T value(vk::ResultValue<T>&& t) { return std::forward<T>(t.value); }

static constexpr const char* LAYERS { "VK_LAYER_KHRONOS_validation" };
static constexpr float QUEUE_PRIORITY[] { 1.0f };
static constexpr vk::DeviceQueueCreateInfo QUEUE_CREATE_INFO { {}, 0, 1, QUEUE_PRIORITY };

struct Vulkan {
    vk::raii::Context context {};
    vk::raii::Instance instance = value(context.createInstance(vk::InstanceCreateInfo { {}, {}, LAYERS, }));
    vk::raii::PhysicalDevice physicalDevice = value(instance.enumeratePhysicalDevices())[0];
    vk::raii::Device device = value(physicalDevice.createDevice(vk::DeviceCreateInfo { {}, QUEUE_CREATE_INFO }));
};
