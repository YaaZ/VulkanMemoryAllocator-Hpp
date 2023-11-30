cmake_minimum_required(VERSION 3.6 FATAL_ERROR)

project(
    VulkanMemoryAllocator-Hpp
    VERSION 0
    DESCRIPTION "C++ bindings for the VulkanMemoryAllocator library."
    LANGUAGES C CXX)

find_package(Vulkan REQUIRED)

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/VulkanMemoryAllocator/CMakeLists.txt)
    add_subdirectory(VulkanMemoryAllocator)
else()
    find_package(VulkanMemoryAllocator REQUIRED)
endif()

add_library(VulkanMemoryAllocator-Hpp INTERFACE)

target_include_directories(VulkanMemoryAllocator-Hpp
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(VulkanMemoryAllocator-Hpp
    INTERFACE
        Vulkan::Vulkan
        VulkanMemoryAllocator::VulkanMemoryAllocator
)

install(
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/vk_mem_alloc_enums.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/vk_mem_alloc_funcs.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/vk_mem_alloc_handles.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/vk_mem_alloc_structs.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/vk_mem_alloc.hpp
    DESTINATION
        include
)

install(
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/VulkanMemoryAllocator-HppConfig.cmake
    DESTINATION
        lib/cmake/VulkanMemoryAllocator-Hpp
)

install(
    TARGETS VulkanMemoryAllocator-Hpp
    EXPORT VulkanMemoryAllocator-HppTargets
)

install(
    EXPORT VulkanMemoryAllocator-HppTargets
    FILE VulkanMemoryAllocator-HppTargets.cmake
    NAMESPACE VulkanMemoryAllocator-Hpp::
    DESTINATION lib/cmake/VulkanMemoryAllocator-Hpp
)
