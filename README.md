# VulkanMemoryAllocator-Hpp <!--VER-->3.3.0<!--/VER-->

### Supports Vulkan <!--VK-->1.4<!--/VK-->

VMA-Hpp provides C++ bindings for [VulkanMemoryAllocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator),
consistent and compatible with Vulkan C++ bindings ([Vulkan-Hpp](https://github.com/KhronosGroup/Vulkan-Hpp)).


## Getting Started

VMA-Hpp needs [Vulkan-Hpp](https://github.com/KhronosGroup/Vulkan-Hpp) and inherits its requirements (C++11 or newer).
A known compatible version is specified in the [submodule](https://git-scm.com/book/en/v2/Git-Tools-Submodules "git submodule update"),
but VMA-Hpp is not tied to any specific version of Vulkan headers, feel free to use your own version as long as it compiles.

VMA-Hpp also needs [VMA](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator), and this dependency is strict.
Patch revisions of VMA are interchangeable, but anything affecting the API may fail to compile or introduce runtime bugs.

*[GitHub releases](https://github.com/YaaZ/VulkanMemoryAllocator-Hpp/releases) are the recommended way to get VMA-Hpp,
they already include a compatible `vk_mem_alloc.h` header.*

#### Header-Only

```cmake
# Add VMA-Hpp as a CMake subdirectory
add_subdirectory(VulkanMemoryAllocator-Hpp/include)
# Or find installed VMA-Hpp
find_package(VulkanMemoryAllocator-Hpp CONFIG REQUIRED)

# Then link the interface library
target_link_libraries(<target> PRIVATE VulkanMemoryAllocator-Hpp::VulkanMemoryAllocator-Hpp)

# ...
# Or don't bother and just include the headers
target_include_directories(<target> PRIVATE VulkanMemoryAllocator-Hpp/include)
```

```c++
// In one translation unit
#define VMA_IMPLEMENTATION

#include "vk_mem_alloc.hpp"
```

#### C++23 Module

```cmake
# VMA-Hpp module requires Vulkan::HppModule target:
add_subdirectory(Vulkan-Headers)
add_subdirectory(VulkanMemoryAllocator-Hpp/include)
# Or
find_package(VulkanHeaders CONFIG)
find_package(VulkanMemoryAllocator-Hpp CONFIG REQUIRED)

# Then link the library
target_link_libraries(<target> PRIVATE VulkanMemoryAllocator-Hpp::VulkanMemoryAllocator-HppModule)

# ...
# Or compile it yourself:
target_include_directories(<target> PRIVATE VulkanMemoryAllocator-Hpp/include)
target_sources(<target> PRIVATE
        FILE_SET CXX_MODULES
        FILES VulkanMemoryAllocator-Hpp/include/vk_mem_alloc.cppm)
```

```c++
// Do not define VMA_IMPLEMENTATION
import vk_mem_alloc_hpp; // Also imports vulkan_hpp and std
```

#### External VulkanMemoryAllocator

If your VMA-Hpp is missing a bundled `vk_mem_alloc.h`, or you need a custom one:

```cmake
# Add VMA as a CMake subdirectory
add_subdirectory(VulkanMemoryAllocator)
# Or find installed VMA
find_package(VulkanMemoryAllocator CONFIG REQUIRED)

# Then link the interface library
target_link_libraries(<target> PRIVATE GPUOpen::VulkanMemoryAllocator)
```

You can also include `vk_mem_alloc.h` from an arbitrary location before including `vk_mem_alloc.hpp`.


## Features

#### Vulkan-Hpp compatibility

VMA-Hpp is built on top of Vulkan-Hpp and reflects most of its
[features](https://github.com/KhronosGroup/Vulkan-Hpp?tab=readme-ov-file#usage-):

- `vma::` and `vma::raii::` namespaces
- Naming convention (e.g. `vmaCreateAllocator` -> `vma::createAllocator`)
- C++ enums (including flags), structs, handles (plain, unique, RAII)
- Result checks, exceptions and custom success codes.
- Respecting `VULKAN_HPP_NO_EXCEPTIONS`, `VULKAN_HPP_NO_CONSTRUCTORS`, `VULKAN_HPP_NO_SETTERS`, etc.
- Enhanced functions accepting references, `vk::ArrayProxy`, `vk::Optional`, custom vector allocators, etc.
- `vma::to_string`

#### VK_ERROR_FEATURE_NOT_PRESENT custom handling

Query functions `vma::findMemoryTypeIndexForBufferInfo`, `vma::findMemoryTypeIndexForImageInfo` and `vma::findMemoryTypeIndex`
return `vk::MaxMemoryTypes` instead of throwing an exception when VMA returns `VK_ERROR_FEATURE_NOT_PRESENT`.

#### vma::functionsFromDispatcher

```c++
// Creates a VMA function table (vma::VulkanFunctions) from an arbitrary dispatcher
vma::functionsFromDispatcher(dispatcher);
// No-arg version retrieves function pointers from the default dispatcher (VULKAN_HPP_DEFAULT_DISPATCHER)
vma::functionsFromDispatcher();
// Multi-dispatcher version takes the first-found field among all dispatchers, for each function pointer
vma::functionsFromDispatchers(deviceDispatcher, instanceDispatcher);
```

#### vma::UniqueBuffer and vma::UniqueImage

`*Unique` variants of VMA-Hpp functions return special handle variants `vma::UniqueBuffer` and `vma::UniqueImage`
instead of `vk::UniqueBuffer` and `vk::UniqueImage`.
Those use `vmaDestroyBuffer` and `vmaDestroyImage` instead of `vkDestroyBuffer` and `vkDestroyImage`.

### RAII

#### vma::raii::createAllocator

RAII variant of `createAllocator` function differs from the original version - it takes a `vk::raii::Instance` and `vk::raii::Device`
and requires corresponding members of `vma::AllocatorCreateInfo` to be unset, as well as `pVulkanFunctions`, which is
automatically retrieved from image and device dispatchers.
```c++
vk::raii::Instance instance = ...;
vk::raii::PhysicalDevice physicalDevice = ...;
vk::raii::Device device = ...;
vma::raii::Allocator alloc { instance, device,
    vma::AllocatorCreateInfo { {}, physicalDevice } };
```

#### vma::raii::Buffer and vma::raii::Image

Special handle variants combine resource and allocation in a single RAII object:
```c++
vma::raii::Allocator alloc = ...;
vma::raii::Buffer buffer = alloc.createBuffer(...);
// VMA variant inherits from the base Vulkan one
const vk::raii::Buffer& vkbuf = buffer;
// And also contains an allocation handle
const vma::raii::Allocation& allocation = buffer.getAllocation();
```

#### Combining function variants

Some RAII functions have "combining" variants, binding resource and allocation together by moving from existing handles:

```c++
vma::raii::Allocator alloc = ...;
vma::raii::Allocation allocation = ...;
// Default variant of vmaCreateAliasingBuffer
vk::raii::Buffer buffer = alloc.createAliasingBuffer(allocation, ...);
// Combining variant of vmaCreateAliasingBuffer
vma::raii::Buffer combinedBuffer = alloc.createAliasingBuffer(std::move(allocation), ...);
// Combining variant of vmaAllocateMemoryForBuffer
combinedBuffer = alloc.allocateMemoryForBuffer(std::move(buffer), ...);
// Combining variant of vmaBindBufferMemory
combinedBuffer = std::move(allocation).bindBuffer(std::move(buffer));
```
