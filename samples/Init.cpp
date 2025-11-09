#include "common.hpp"

int main(int, char**) {
    Vulkan vulkan;

    auto functions = vma::functionsFromDispatchers(*vulkan.device.getDispatcher(), *vulkan.instance.getDispatcher());
    auto allocator = vma::createAllocatorUnique(vma::AllocatorCreateInfo {
        {}, vulkan.physicalDevice, vulkan.device
    }.setInstance(vulkan.instance).setPVulkanFunctions(&functions));

    auto buffer = allocator->createBufferUnique(
      vk::BufferCreateInfo { {}, 1024, vk::BufferUsageFlagBits::eTransferSrc },
      vma::AllocationCreateInfo { vma::AllocationCreateFlagBits::eHostAccessSequentialWrite, vma::MemoryUsage::eAuto });
    return 0;
}
