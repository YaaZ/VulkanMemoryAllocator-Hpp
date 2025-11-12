#include "common.hpp"

int main(int, char**) {
    Vulkan vulkan;

    auto allocator = value(vma::raii::createAllocator(vulkan.instance, vulkan.device, vma::AllocatorCreateInfo {
        {}, vulkan.physicalDevice
    }));

    auto buffer = value(allocator.createBuffer(
      vk::BufferCreateInfo { {}, 1024, vk::BufferUsageFlagBits::eTransferSrc },
      vma::AllocationCreateInfo { vma::AllocationCreateFlagBits::eHostAccessSequentialWrite, vma::MemoryUsage::eAuto }));

    auto statsString = allocator.buildStatsString(false);

    return 0;
}
