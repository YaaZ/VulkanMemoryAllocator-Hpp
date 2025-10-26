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

  namespace detail {
    template<int N, int... I> struct Seq : Seq<N-1, N-1, I...> {};
    template<int... I> struct Seq<0, I...> {};
    struct Placeholder {} VULKAN_HPP_CONST_OR_CONSTEXPR placeholder;

    // Base converter - creates an object (delegating to the final converter), substituting argument placeholders.
    template<class Dst, class Wrapper = void, class = void> struct Converter {
      static Dst none() { return Dst(nullptr); }
      template<class... Src> Dst convert(Src&&... src) const {
        return create(static_cast<const Wrapper&>(*this), std::forward<Src>(src)...);
      }
    private:
      template<class... Src, class... Args> Dst create(const std::tuple<Args...>& args, Src&&... src) const {
        return create(Seq<sizeof...(Args)>(), args, std::forward<Src>(src)...);
      }
      template<class... Src, class... Args, int... I> Dst create(Seq<0, I...>, const std::tuple<Args...>& args, Src&&... src) const {
        return Converter<Dst>::create(convertParam(std::forward<Args>(std::get<I>(args)), std::forward<Src>(src)...)...);
      }
      template<class T, class... Src> static T&& convertParam(T&& t, Src&&...) { return std::forward<T>(t); }
      template<class Src> static Src&& convertParam(Placeholder, Src&& src) { return std::forward<Src>(src); }
    };

    // Final converter - creates an object from the provided arguments.
    template<class Dst> struct Converter<Dst> {
      template<class... Args> static Dst create(Args&&... args) { return Dst(std::forward<Args>(args)...); }
    };

    // std::vector converter.
    template<class Dst, class Allocator, class Wrapper>
    struct Converter<std::vector<Dst, Allocator>, Wrapper> : Converter<Dst, Wrapper> {
      static std::vector<Dst, Allocator> none() { return {}; }
      template<class Src, class Alloc> std::vector<Dst, Allocator> convert(std::vector<Src, Alloc>&& src) const {
        std::vector<Dst, Allocator> result;
        result.reserve(src.size());
        for (Src& s : src) result.push_back(Converter<Dst, Wrapper>::convert(std::forward<Src>(s)));
        return std::move(result);
      }
    };

    // ResultValue converter.
    template<class Dst, class Wrapper>
    struct Converter<VULKAN_HPP_NAMESPACE::ResultValue<Dst>, Wrapper> : Converter<Dst, Wrapper> {
      template<class... Src> VULKAN_HPP_NAMESPACE::ResultValue<Dst> convert(VULKAN_HPP_NAMESPACE::Result result, Src&&... src) const {
        return VULKAN_HPP_NAMESPACE::ResultValue<Dst>(result, result < VULKAN_HPP_NAMESPACE::Result::eSuccess ?
          Converter<Dst, Wrapper>::none() : Converter<Dst, Wrapper>::convert(std::forward<Src>(src)...));
      }
      template<class Src> VULKAN_HPP_NAMESPACE::ResultValue<Dst> convert(VULKAN_HPP_NAMESPACE::ResultValue<Src>&& src) const {
        return convert(src.result, std::forward<Src>(src.value));
      }
    };

    template<class Src, class Dst, class... Args> class Wrapper : public Wrapper<void, Dst, Args...> {
      Src src;
    public:
      explicit Wrapper(Wrapper<void, Dst, Args...>&& base, Src&& src) :
        Wrapper<void, Dst, Args...>(std::move(base)), src(std::forward<Src>(src)) {}
      operator Dst() && { return this->convert(std::forward<Src>(src)); }
    };

    template<class Dst, class... Args> class Wrapper<void, Dst, Args...> :
    public std::tuple<Args&&...>, public Converter<Dst, Wrapper<void, Dst, Args...>> {
    public:
      explicit Wrapper(Args&&... args) : std::tuple<Args&&...>(std::forward<Args>(args)...) {}
      template<class Src> friend Wrapper<Src, Dst, Args...> operator,(Src&& src, Wrapper&& w) {
        return Wrapper<Src, Dst, Args...>(std::move(w), std::forward<Src>(src));
      }
      operator Dst() && { return this->convert(); }
    };

    template<class Dst, class... Args> Wrapper<void, Dst, Args...> wrap(Args&&... args) {
      return Wrapper<void, Dst, Args...>(std::forward<Args>(args)...);
    }
  }
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
