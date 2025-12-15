# VulkanMemoryAllocator-Hpp <!--VER-->3.3.0<!--/VER-->

VMA-Hpp provides C++ bindings for [VulkanMemoryAllocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator),
consistent and compatible with Vulkan C++ bindings ([Vulkan-Hpp](https://github.com/KhronosGroup/Vulkan-Hpp)).


## Getting Started

#### Requirements

- C++11 or newer
- [Vulkan](https://github.com/KhronosGroup/Vulkan-Headers) **<!--VK_HEADERS-->1.4.327<!--/VK_HEADERS-->** (headers) / **<!--VK_MODULES-->1.4.336<!--/VK_MODULES-->** (module) or newer
- [VulkanMemoryAllocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator)
  - *[GitHub releases](https://github.com/YaaZ/VulkanMemoryAllocator-Hpp/releases) is the recommended way to get VMA-Hpp,
    they already include a compatible `vk_mem_alloc.h` header*
  - *Compatible VMA version is tracked as a git submodule, use any other version at your own risk*

#### Get the library

```cmake
# Download VMA-Hpp release
include(FetchContent)
FetchContent_Declare(
        vmahpp
        URL      https://github.com/YaaZ/VulkanMemoryAllocator-Hpp/releases/download/<tag>/VulkanMemoryAllocator-Hpp-<version>.tar.gz
        URL_HASH SHA256=<hash> # Copy from the asset list on the release page
)
FetchContent_MakeAvailable(vmahpp)

# Or add a local directory
add_subdirectory(VulkanMemoryAllocator-Hpp/include) # You don't need the top-level one

# Or find an installed library
find_package(VulkanMemoryAllocator-Hpp CONFIG REQUIRED)
```

#### Use headers

```cmake
# Link the interface library
target_link_libraries(<target> PRIVATE VulkanMemoryAllocator-Hpp::VulkanMemoryAllocator-Hpp)

# Or don't bother and just include the headers
target_include_directories(<target> PRIVATE ${vmahpp_SOURCE_DIR})
```

```c++
// In one translation unit
#define VMA_IMPLEMENTATION

#include "vk_mem_alloc.hpp"
```

#### Use C++23 module

```cmake
# VMA-Hpp module requires Vulkan::HppModule target:
find_package(VulkanHeaders CONFIG) # Or fetch from https://github.com/KhronosGroup/Vulkan-Headers

# Link the library
target_link_libraries(<target> PRIVATE VulkanMemoryAllocator-Hpp::VulkanMemoryAllocator-HppModule)

# ...
# Or compile it yourself:
target_include_directories(<target> PRIVATE ${vmahpp_SOURCE_DIR})
target_sources(<target> PRIVATE
        FILE_SET CXX_MODULES
        FILES ${vmahpp_SOURCE_DIR}/vk_mem_alloc.cppm)
```

```c++
// Do not define VMA_IMPLEMENTATION
import vk_mem_alloc; // Also imports vulkan and std
```


## Features

#### Vulkan-Hpp compatibility

VMA-Hpp is built on top of [Vulkan-Hpp](https://github.com/KhronosGroup/Vulkan-Hpp) and mirrors most of its
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
// VMA handle inherits from the Vulkan one
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
