// Macros
#ifndef VMA_HPP_NAMESPACE
#define VMA_HPP_NAMESPACE vma
#endif
#ifndef VMA_HPP_RAII_NAMESPACE
#define VMA_HPP_RAII_NAMESPACE raii
#endif
#include <vulkan/vulkan_hpp_macros.hpp>
#define VMA_HPP_NAMESPACE_STRING VULKAN_HPP_STRINGIFY(VMA_HPP_NAMESPACE)
#define VMA_HPP_RAII_NAMESPACE_STRING VULKAN_HPP_STRINGIFY(VMA_HPP_NAMESPACE::VMA_HPP_RAII_NAMESPACE)

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
#include <vulkan/vulkan_raii.hpp>
#endif
#endif

#ifndef VULKAN_HPP_NO_SMART_HANDLE
namespace VMA_HPP_NAMESPACE {
struct Dispatcher {}; // VMA uses function pointers from VmaAllocator instead
class Allocator;

template <class T>
VULKAN_HPP_NAMESPACE::UniqueHandle<T, Dispatcher>
createUniqueHandle(const T &t) VULKAN_HPP_NOEXCEPT {
  return VULKAN_HPP_NAMESPACE::UniqueHandle<T, Dispatcher>(t);
}
template <class T, class O>
VULKAN_HPP_NAMESPACE::UniqueHandle<T, Dispatcher>
createUniqueHandle(const T &t, O o) VULKAN_HPP_NOEXCEPT {
  return VULKAN_HPP_NAMESPACE::UniqueHandle<T, Dispatcher>(t, o);
}
template <class F, class S, class O>
std::pair<VULKAN_HPP_NAMESPACE::UniqueHandle<F, Dispatcher>,
          VULKAN_HPP_NAMESPACE::UniqueHandle<S, Dispatcher>>
createUniqueHandle(const std::pair<F, S> &t, O o) VULKAN_HPP_NOEXCEPT {
  return {VULKAN_HPP_NAMESPACE::UniqueHandle<F, Dispatcher>(t.first, o),
          VULKAN_HPP_NAMESPACE::UniqueHandle<S, Dispatcher>(t.second, o)};
}

template <class T, class UniqueVectorAllocator, class VectorAllocator, class O>
std::vector<VULKAN_HPP_NAMESPACE::UniqueHandle<T, Dispatcher>,
            UniqueVectorAllocator>
createUniqueHandleVector(const std::vector<T, VectorAllocator> &vector, O o,
                         const UniqueVectorAllocator &vectorAllocator)
    VULKAN_HPP_NOEXCEPT {
  std::vector<VULKAN_HPP_NAMESPACE::UniqueHandle<T, Dispatcher>,
              UniqueVectorAllocator>
      result(vectorAllocator);
  result.reserve(vector.size());
  for (const T &t : vector)
    result.emplace_back(t, o);
  return result;
}

template <class T, class Owner> class Deleter {
  Owner owner;

public:
  Deleter() = default;
  Deleter(Owner owner) VULKAN_HPP_NOEXCEPT : owner(owner) {}

protected:
  void destroy(const T &t)
      VULKAN_HPP_NOEXCEPT; // Implemented manually for each handle type
};
template <class T> class Deleter<T, void> {
protected:
  void destroy(const T &t) VULKAN_HPP_NOEXCEPT { t.destroy(); }
};
} // namespace VMA_HPP_NAMESPACE
namespace VULKAN_HPP_NAMESPACE {
template <> class UniqueHandleTraits<Buffer, VMA_HPP_NAMESPACE::Dispatcher> {
public:
  using deleter =
      VMA_HPP_NAMESPACE::Deleter<Buffer, VMA_HPP_NAMESPACE::Allocator>;
};
template <> class UniqueHandleTraits<Image, VMA_HPP_NAMESPACE::Dispatcher> {
public:
  using deleter =
      VMA_HPP_NAMESPACE::Deleter<Image, VMA_HPP_NAMESPACE::Allocator>;
};
} // namespace VULKAN_HPP_NAMESPACE
namespace VMA_HPP_NAMESPACE {
using UniqueBuffer =
    VULKAN_HPP_NAMESPACE::UniqueHandle<VULKAN_HPP_NAMESPACE::Buffer,
                                       Dispatcher>;
using UniqueImage =
    VULKAN_HPP_NAMESPACE::UniqueHandle<VULKAN_HPP_NAMESPACE::Image, Dispatcher>;
} // namespace VMA_HPP_NAMESPACE
#endif

// #include "vk_mem_alloc_validation.hpp"

#include "vk_mem_alloc_enums.hpp"

#ifndef VULKAN_HPP_NO_TO_STRING
#include "vk_mem_alloc_to_string.hpp"
#endif

#include "vk_mem_alloc_handles.hpp"

#include "vk_mem_alloc_structs.hpp"

// #include "vk_mem_alloc_funcs.hpp"

namespace VMA_HPP_NAMESPACE {

#ifndef VULKAN_HPP_NO_SMART_HANDLE
#define VMA_HPP_DESTROY_IMPL(NAME)                                             \
  template <>                                                                  \
  VULKAN_HPP_INLINE void VULKAN_HPP_NAMESPACE::UniqueHandleTraits<             \
      NAME, Dispatcher>::deleter::destroy(const NAME &t) VULKAN_HPP_NOEXCEPT

VMA_HPP_DESTROY_IMPL(VULKAN_HPP_NAMESPACE::Buffer) {
  owner.destroyBuffer(t, nullptr);
}
VMA_HPP_DESTROY_IMPL(VULKAN_HPP_NAMESPACE::Image) {
  owner.destroyImage(t, nullptr);
}
VMA_HPP_DESTROY_IMPL(Pool) { owner.destroyPool(t); }
VMA_HPP_DESTROY_IMPL(Allocation) { owner.freeMemory(t); }
VMA_HPP_DESTROY_IMPL(VirtualAllocation) { owner.virtualFree(t); }

#undef VMA_HPP_DESTROY_IMPL
#endif

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
