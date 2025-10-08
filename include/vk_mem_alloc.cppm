// Generated from the Vulkan Memory Allocator (vk_mem_alloc.h).
module;
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.hpp>
//#include <vk_mem_alloc_raii.hpp> TODO
export module vk_mem_alloc_hpp;

export namespace VMA_HPP_NAMESPACE {
#ifndef VULKAN_HPP_NO_TO_STRING
  using VMA_HPP_NAMESPACE::to_string;
#endif
  using VMA_HPP_NAMESPACE::functionsFromDispatcher;
  using VMA_HPP_NAMESPACE::operator|;
  using VMA_HPP_NAMESPACE::operator&;
  using VMA_HPP_NAMESPACE::operator^;
  using VMA_HPP_NAMESPACE::operator~;
  using VMA_HPP_NAMESPACE::AllocatorCreateFlagBits;
  using VMA_HPP_NAMESPACE::AllocatorCreateFlags;
  using VMA_HPP_NAMESPACE::MemoryUsage;
  using VMA_HPP_NAMESPACE::AllocationCreateFlagBits;
  using VMA_HPP_NAMESPACE::AllocationCreateFlags;
  using VMA_HPP_NAMESPACE::PoolCreateFlagBits;
  using VMA_HPP_NAMESPACE::PoolCreateFlags;
  using VMA_HPP_NAMESPACE::DefragmentationFlagBits;
  using VMA_HPP_NAMESPACE::DefragmentationFlags;
  using VMA_HPP_NAMESPACE::DefragmentationMoveOperation;
  using VMA_HPP_NAMESPACE::VirtualBlockCreateFlagBits;
  using VMA_HPP_NAMESPACE::VirtualBlockCreateFlags;
  using VMA_HPP_NAMESPACE::VirtualAllocationCreateFlagBits;
  using VMA_HPP_NAMESPACE::VirtualAllocationCreateFlags;
  using VMA_HPP_NAMESPACE::DeviceMemoryCallbacks;
  using VMA_HPP_NAMESPACE::VulkanFunctions;
  using VMA_HPP_NAMESPACE::AllocatorCreateInfo;
  using VMA_HPP_NAMESPACE::AllocatorInfo;
  using VMA_HPP_NAMESPACE::Statistics;
  using VMA_HPP_NAMESPACE::DetailedStatistics;
  using VMA_HPP_NAMESPACE::TotalStatistics;
  using VMA_HPP_NAMESPACE::Budget;
  using VMA_HPP_NAMESPACE::AllocationCreateInfo;
  using VMA_HPP_NAMESPACE::PoolCreateInfo;
  using VMA_HPP_NAMESPACE::AllocationInfo;
  using VMA_HPP_NAMESPACE::AllocationInfo2;
  using VMA_HPP_NAMESPACE::DefragmentationInfo;
  using VMA_HPP_NAMESPACE::DefragmentationMove;
  using VMA_HPP_NAMESPACE::DefragmentationPassMoveInfo;
  using VMA_HPP_NAMESPACE::DefragmentationStats;
  using VMA_HPP_NAMESPACE::VirtualBlockCreateInfo;
  using VMA_HPP_NAMESPACE::VirtualAllocationCreateInfo;
  using VMA_HPP_NAMESPACE::VirtualAllocationInfo;
}

module : private;
namespace VULKAN_HPP_NAMESPACE {
  // This is needed for template specializations to be visible outside the module when importing vulkan_hpp (is this a MSVC bug?).
  template<> struct FlagTraits<VMA_HPP_NAMESPACE::AllocatorCreateFlagBits>;
  template<> struct FlagTraits<VMA_HPP_NAMESPACE::AllocationCreateFlagBits>;
  template<> struct FlagTraits<VMA_HPP_NAMESPACE::PoolCreateFlagBits>;
  template<> struct FlagTraits<VMA_HPP_NAMESPACE::DefragmentationFlagBits>;
  template<> struct FlagTraits<VMA_HPP_NAMESPACE::VirtualBlockCreateFlagBits>;
  template<> struct FlagTraits<VMA_HPP_NAMESPACE::VirtualAllocationCreateFlagBits>;
}