// Macros
#ifndef VMA_HPP_NAMESPACE
#define VMA_HPP_NAMESPACE vma
#endif
#ifndef VMA_HPP_RAII_NAMESPACE
#define VMA_HPP_RAII_NAMESPACE raii
#endif
#include <vulkan/vulkan_hpp_macros.hpp>

// VMA header
#ifndef AMD_VULKAN_MEMORY_ALLOCATOR_H
#include <vk_mem_alloc.h>
#endif

// The body
#ifndef VULKAN_MEMORY_ALLOCATOR_HPP
#define VULKAN_MEMORY_ALLOCATOR_HPP

#ifdef VULKAN_HPP_ENABLE_STD_MODULE
import VULKAN_HPP_STD_MODULE;
#else
#include <algorithm>
#include <array>
#include <vector>
#ifndef VULKAN_HPP_NO_TO_STRING
#include <string>
#endif
#endif

#ifdef VMA_HPP_ENABLE_VULKAN_HPP_MODULE
import vulkan_hpp;
#else
#ifndef VULKAN_HPP
#include <vulkan/vulkan.hpp>
#endif
#endif

#define VMA_HPP_NAMESPACE_STRING VULKAN_HPP_STRINGIFY(VMA_HPP_NAMESPACE)
#define VMA_HPP_RAII_NAMESPACE_STRING VULKAN_HPP_STRINGIFY(VMA_HPP_NAMESPACE::VMA_HPP_RAII_NAMESPACE)

namespace VMA_HPP_NAMESPACE {
  using VULKAN_HPP_NAMESPACE::operator&;
  using VULKAN_HPP_NAMESPACE::operator|;
  using VULKAN_HPP_NAMESPACE::operator^;
  using VULKAN_HPP_NAMESPACE::operator~;
  using VULKAN_HPP_NAMESPACE::operator<;
  using VULKAN_HPP_NAMESPACE::operator<=;
  using VULKAN_HPP_NAMESPACE::operator>;
  using VULKAN_HPP_NAMESPACE::operator>=;
  using VULKAN_HPP_NAMESPACE::operator==;
  using VULKAN_HPP_NAMESPACE::operator!=;
}

#include "vk_mem_alloc_enums.hpp"

#ifndef VULKAN_HPP_NO_TO_STRING
#include "vk_mem_alloc_to_string.hpp"
#endif

#include "vk_mem_alloc_handles.hpp"

#include "vk_mem_alloc_structs.hpp"

#include "vk_mem_alloc_funcs.hpp"

namespace VMA_HPP_NAMESPACE {
  template <class InstanceDispatcher, class DeviceDispatcher> VULKAN_HPP_CONSTEXPR VulkanFunctions
  functionsFromDispatcher(InstanceDispatcher const * instance, DeviceDispatcher const * device) VULKAN_HPP_NOEXCEPT {
    return VulkanFunctions { instance->vkGetInstanceProcAddr, device->vkGetDeviceProcAddr };
  }

  template <class Dispatch = VULKAN_HPP_DEFAULT_DISPATCHER_TYPE> VULKAN_HPP_CONSTEXPR VulkanFunctions
  functionsFromDispatcher(Dispatch const & dispatch VULKAN_HPP_DEFAULT_DISPATCHER_ASSIGNMENT) VULKAN_HPP_NOEXCEPT {
    return VulkanFunctions { dispatch.vkGetInstanceProcAddr, dispatch.vkGetDeviceProcAddr };
  }

#if !defined( VK_NO_PROTOTYPES )
  template <> VULKAN_HPP_CONSTEXPR VulkanFunctions
  functionsFromDispatcher<VULKAN_HPP_DISPATCH_LOADER_STATIC_TYPE>(VULKAN_HPP_DISPATCH_LOADER_STATIC_TYPE const &) VULKAN_HPP_NOEXCEPT {
    return VulkanFunctions { &vkGetInstanceProcAddr, &vkGetDeviceProcAddr };
  }
#endif
} // namespace VMA_HPP_NAMESPACE

#endif
