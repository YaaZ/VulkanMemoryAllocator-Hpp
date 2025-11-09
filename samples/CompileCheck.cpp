#include "common.hpp"

void checkEnums() {
    static_assert(vk::FlagTraits<vma::AllocationCreateFlagBits>::isBitmask, "FlagTraits specialization is not visible");
    constexpr auto flag = static_cast<vma::AllocationCreateFlagBits>(0);
    auto flags = flag | flag;
    flags = flag | flags;
    flags = flags | flag;
    flags = flags | flags;

    static_assert(vk::FlagTraits<vma::AllocationCreateFlagBits>::allFlags, "FlagTraits::allFlags is not visible");
    flags = ~flag;
    flags = ~flags;

    const auto fl = flag, fr = flag;
    const auto fsl = flags, fsr = flags;
    if (fl < fr) throw;
    if (fl < fsr) throw;
    if (fsl < fr) throw;
    if (fsl < fsr) throw;
    if (fl > fr) throw;
    if (fl > fsr) throw;
    if (fsl > fr) throw;
    if (fsl > fsr) throw;
    if (fl <= fr) {}
    if (fl <= fsr) {}
    if (fsl <= fr) {}
    if (fsl <= fsr) {}
    if (fl >= fr) {}
    if (fl >= fsr) {}
    if (fsl >= fr) {}
    if (fsl >= fsr) {}
    if (fl == fr) {}
    if (fl == fsr) {}
    if (fsl == fr) {}
    if (fsl == fsr) {}
    if (fl != fr) throw;
    if (fl != fsr) throw;
    if (fsl != fr) throw;
    if (fsl != fsr) throw;

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

    // Enhanced.
    vma::DefragmentationMove moves[1];
    vma::DefragmentationPassMoveInfo moveInfo { moves };
    moveInfo.setMoves(moves);
}

void checkHandles() {
    vma::Allocator allocator;
    allocator = nullptr;
    allocator = static_cast<VmaAllocator>(VK_NULL_HANDLE);

    if (allocator) throw;
    if (!allocator) {}
    if (allocator == nullptr) {}
    if (nullptr == allocator) {}
    if (allocator != nullptr) throw;
    if (nullptr != allocator) throw;

    const auto al = allocator, ar = allocator;
    if (al > ar) throw;
    if (al < ar) throw;
    if (al >= ar) {}
    if (al <= ar) {}
    if (al == ar) {}
    if (al != ar) throw;

    vma::UniqueAllocator unique;
    vma::UniqueBuffer buffer;

    vma::raii::Allocator raii = nullptr;
    allocator = raii;
    allocator = *raii;
    const auto &rl = raii, &rr = raii;
    if (rl == rr) {}
    if (rl != rr) throw;
}

int main(int, char**) {
    checkEnums();
    checkStructs();
    checkHandles();
    return 0;
}