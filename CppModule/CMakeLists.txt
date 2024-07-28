cmake_minimum_required(VERSION 3.29)

project(CppModules LANGUAGES CXX)

# Vulkan must be >= 
set(VULKAN_PATH "/usr/include" CACHE STRING "Path to Vulkan include dir")
if (NOT VULKAN_PATH)
  message(FATAL_ERROR "VULKAN_PATH not set")
endif()

option(VMA_BUILD_WITH_VULKAN_MODULE OFF)
if (${VMA_BUILD_WITH_VULKAN_MODULE})
  message(STATUS "Building with Vulkan module at ${VULKAN_PATH}")
  add_library(Vulkan-Hpp-Module)
  target_sources(Vulkan-Hpp-Module PUBLIC
    FILE_SET CXX_MODULES
    BASE_DIRS ${VULKAN_PATH}/vulkan
    FILES ${VULKAN_PATH}/vulkan/vulkan.cppm
  )
  target_compile_features(Vulkan-Hpp-Module PUBLIC cxx_std_20)
  target_include_directories(Vulkan-Hpp-Module PUBLIC ${VULKAN_PATH})
else()
  message(STATUS "Building with Vulkan headers at ${VULKAN_PATH}")
  add_library(Vulkan-Hpp-Module INTERFACE)
  target_compile_features(Vulkan-Hpp-Module INTERFACE cxx_std_20)
  target_include_directories(Vulkan-Hpp-Module INTERFACE ${VULKAN_PATH})
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

if (VMA_BUILD_WITH_VULKAN_MODULE)
  option(VMA_BUILD_EXAMPLE ON)
  if (VMA_BUILD_EXAMPLE)
    add_executable(Modules-Example modules-example.cpp)
    target_compile_features(Modules-Example PRIVATE cxx_std_20)
    target_link_libraries(Modules-Example PRIVATE 
      VulkanMemoryAllocator-Hpp-Module
      Vulkan-Hpp-Module
    )
  endif()
endif()
