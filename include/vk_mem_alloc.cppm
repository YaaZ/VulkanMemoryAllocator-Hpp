module;
#ifdef VMA_HPP_ENABLE_VOLK
#include "volk.h"
#endif

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#define VULKAN_HPP_CXX_MODULE
#include <vulkan/vulkan_hpp_macros.hpp>

export module vk_mem_alloc;
import std;
import vulkan;

#include "vk_mem_alloc.hpp"
#include "vk_mem_alloc_raii.hpp"
