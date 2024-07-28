module;

#define VMA_BUILD_MODULE

#ifndef VMA_USE_VULKAN_HPP_MODULE
#define VMA_USE_VULKAN_HPP_MODULE 0
#endif

#ifndef VMA_USE_STD_MODULE
#define VMA_USE_STD_MODULE 0
#endif

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include <vulkan/vulkan_hpp_macros.hpp>

#if !VMA_USE_VULKAN_HPP_MODULE
#include <vulkan/vulkan.hpp>
#endif

#if !VMA_USE_STD_MODULE
#include <string>
#include <vector>
#endif

export module vk_mem_alloc_hpp;

#if VMA_USE_VULKAN_HPP_MODULE
import vulkan_hpp;
#endif

#if VMA_USE_STD_MODULE
import std.compat;
#endif

#include "vk_mem_alloc.hpp"


module : private;

#ifndef VULKAN_HPP_NO_SMART_HANDLE
// Instantiate unique handle templates.
// This is a workaround for MSVC bugs, but wouldn't harm on other compilers anyway.
namespace VULKAN_HPP_NAMESPACE {
  template class UniqueHandle<Buffer, VMA_HPP_NAMESPACE::Dispatcher>;
  template class UniqueHandle<Image, VMA_HPP_NAMESPACE::Dispatcher>;
  template class UniqueHandle<VMA_HPP_NAMESPACE::Allocator, VMA_HPP_NAMESPACE::Dispatcher>;
  template class UniqueHandle<VMA_HPP_NAMESPACE::Pool, VMA_HPP_NAMESPACE::Dispatcher>;
  template class UniqueHandle<VMA_HPP_NAMESPACE::Allocation, VMA_HPP_NAMESPACE::Dispatcher>;
  template class UniqueHandle<VMA_HPP_NAMESPACE::VirtualAllocation, VMA_HPP_NAMESPACE::Dispatcher>;
  template class UniqueHandle<VMA_HPP_NAMESPACE::VirtualBlock, VMA_HPP_NAMESPACE::Dispatcher>;
}
#endif
