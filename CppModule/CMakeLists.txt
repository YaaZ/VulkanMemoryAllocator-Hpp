cmake_minimum_required(VERSION 3.28)

find_package(VulkanHeaders REQUIRED)

option(VMA_BUILD_WITH_VULKAN_MODULE OFF)
if (${VMA_BUILD_WITH_VULKAN_MODULE})
  add_library(Vulkan-Hpp-Module)
  target_sources(Vulkan-Hpp-Module PUBLIC
    FILE_SET CXX_MODULES
    BASE_DIRS ${Vulkan_INCLUDE_DIR}/vulkan
    FILES ${Vulkan_INCLUDE_DIR}/vulkan/vulkan.cppm
  )
  target_compile_features(Vulkan-Hpp-Module PUBLIC cxx_std_20)
  target_link_libraries(Vulkan-Hpp-Module PUBLIC Vulkan::Headers)
else()
  add_library(Vulkan-Hpp-Module INTERFACE)
  target_compile_features(Vulkan-Hpp-Module INTERFACE cxx_std_20)
  target_link_libraries(Vulkan-Hpp-Module INTERFACE Vulkan::Headers)
endif()

add_library(VulkanMemoryAllocator-Hpp-Module)
target_sources(VulkanMemoryAllocator-Hpp-Module PUBLIC
  FILE_SET CXX_MODULES
    BASE_DIRS ../src
    FILES ../src/vk_mem_alloc.cppm
)
target_compile_features(VulkanMemoryAllocator-Hpp-Module PUBLIC cxx_std_20)
target_link_libraries(VulkanMemoryAllocator-Hpp-Module PRIVATE Vulkan-Hpp-Module VulkanMemoryAllocator-Hpp)
target_compile_definitions(VulkanMemoryAllocator-Hpp-Module PRIVATE
  VMA_USE_VULKAN_HPP_MODULE=$<BOOL:${VMA_BUILD_WITH_VULKAN_MODULE}>
)

target_compile_options(VulkanMemoryAllocator-Hpp-Module PRIVATE -w)
