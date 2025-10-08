#ifdef VMA_HPP_ENABLE_MODULE
#ifdef VMA_HPP_ENABLE_VULKAN_HPP_MODULE

#ifdef VULKAN_HPP_ENABLE_STD_MODULE
import std;
#else
#include <compare>
#endif

import vulkan_hpp;
#else
#include <vulkan/vulkan.hpp>
#endif

import vk_mem_alloc_hpp;
#else
#include <vk_mem_alloc.hpp>
#endif

#include <vk_mem_alloc_imported.hpp>
#include <vk_mem_alloc_static_assertions.hpp>
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

void checkEnums() {
    static_assert(vk::FlagTraits<vma::AllocationCreateFlagBits>::isBitmask, "FlagTraits specialization is not visible");
    constexpr auto flag = vma::AllocationCreateFlagBits::eDedicatedMemory;
    auto flags = flag | flag;
    flags = flag | flags;
    flags = flags | flag;
    flags = flags | flags;

    static_assert(vk::FlagTraits<vma::AllocationCreateFlagBits>::allFlags, "FlagTraits::allFlags is not visible");
    flags = ~flag;
    flags = ~flags;

#ifndef VULKAN_HPP_NO_TO_STRING
    to_string(flag);
    to_string(flags);
#endif
}

void checkStructs() {
    vma::VulkanFunctions functions = vma::functionsFromDispatcher();
    functions = vma::functionsFromDispatcher(vk::detail::DispatchLoaderDynamic {});
#if !defined( VK_NO_PROTOTYPES )
    functions = vma::functionsFromDispatcher(vk::detail::DispatchLoaderStatic {});
#endif
    functions = vma::VulkanFunctions { nullptr };
    functions.setVkGetInstanceProcAddr(nullptr);
    functions.vkGetInstanceProcAddr = nullptr;

    if (vma::DeviceMemoryCallbacks {} != vma::DeviceMemoryCallbacks {}) throw;
    if (vma::VulkanFunctions {} != vma::VulkanFunctions {}) throw;
    if (vma::AllocatorCreateInfo {} != vma::AllocatorCreateInfo {}) throw;
    if (vma::AllocatorInfo {} != vma::AllocatorInfo {}) throw;
    if (vma::Statistics {} != vma::Statistics {}) throw;
    if (vma::DetailedStatistics {} != vma::DetailedStatistics {}) throw;
    if (vma::TotalStatistics {} != vma::TotalStatistics {}) throw;
    if (vma::Budget {} != vma::Budget {}) throw;
    if (vma::AllocationCreateInfo {} != vma::AllocationCreateInfo {}) throw;
    if (vma::PoolCreateInfo {} != vma::PoolCreateInfo {}) throw;
    if (vma::AllocationInfo {} != vma::AllocationInfo {}) throw;
    if (vma::AllocationInfo2 {} != vma::AllocationInfo2 {}) throw;
    if (vma::DefragmentationInfo {} != vma::DefragmentationInfo {}) throw;
    if (vma::DefragmentationMove {} != vma::DefragmentationMove {}) throw;
    if (vma::DefragmentationPassMoveInfo {} != vma::DefragmentationPassMoveInfo {}) throw;
    if (vma::DefragmentationStats {} != vma::DefragmentationStats {}) throw;
    if (vma::VirtualBlockCreateInfo {} != vma::VirtualBlockCreateInfo {}) throw;
    if (vma::VirtualAllocationCreateInfo {} != vma::VirtualAllocationCreateInfo {}) throw;
    if (vma::VirtualAllocationInfo {} != vma::VirtualAllocationInfo {}) throw;
}

int main(int, char**) {
    checkEnums();
    checkStructs();
    return 0;
}