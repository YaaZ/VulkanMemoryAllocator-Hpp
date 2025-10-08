cmake_minimum_required(VERSION 3.6...3.30 FATAL_ERROR)

project(
    VulkanMemoryAllocator-Hpp
    VERSION 3.3.0
    DESCRIPTION "C++ bindings for the VulkanMemoryAllocator library."
    LANGUAGES CXX)
set(VMA_HPP_PROJECT_VERSION ${PROJECT_VERSION} PARENT_SCOPE)

option(VMA_HPP_ENABLE_CPP20_MODULES "Build VMA-Hpp as C++20 module; requires minimum CMake version 3.28" OFF)
option(VMA_HPP_ENABLE_INSTALL "Install VulkanMemoryAllocator-Hpp" "${PROJECT_IS_TOP_LEVEL}")

add_library(VulkanMemoryAllocator-Hpp INTERFACE)
add_library(VulkanMemoryAllocator-Hpp::VulkanMemoryAllocator-Hpp ALIAS VulkanMemoryAllocator-Hpp)
target_include_directories(VulkanMemoryAllocator-Hpp INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

#if (VMA_HPP_ENABLE_INSTALL)
#    include(GNUInstallDirs)
#    include(CMakePackageConfigHelpers)
#
#    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
#    install(TARGETS VulkanMemoryAllocator-Hpp EXPORT VulkanMemoryAllocator-HppConfig INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
#    install(EXPORT VulkanMemoryAllocator-HppConfig NAMESPACE "VulkanMemoryAllocator-Hpp::" DESTINATION "share/cmake/VulkanMemoryAllocator-Hpp")
#
#    write_basic_package_version_file(VulkanMemoryAllocator-HppConfigVersion.cmake COMPATIBILITY SameMajorVersion ARCH_INDEPENDENT)
#    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/VulkanMemoryAllocator-HppConfigVersion.cmake" DESTINATION "share/cmake/VulkanMemoryAllocator-Hpp")
#endif()
