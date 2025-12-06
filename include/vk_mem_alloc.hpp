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
#include "vk_mem_alloc.h"
#endif

// The body
#ifndef VULKAN_MEMORY_ALLOCATOR_HPP
#define VULKAN_MEMORY_ALLOCATOR_HPP

#ifdef VMA_HPP_CXX_MODULE
import vulkan;
#else
#include <vulkan/vulkan.hpp>
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

#endif
