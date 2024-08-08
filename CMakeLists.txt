cmake_minimum_required(VERSION 3.6 FATAL_ERROR)

project(
    VulkanMemoryAllocator-Hpp
    VERSION 3.1.0
    DESCRIPTION "C++ bindings for the VulkanMemoryAllocator library."
    LANGUAGES C CXX)

add_library(VulkanMemoryAllocator-Hpp INTERFACE)
add_library(VulkanMemoryAllocator-Hpp::VulkanMemoryAllocator-Hpp ALIAS VulkanMemoryAllocator-Hpp)

target_include_directories(VulkanMemoryAllocator-Hpp INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

if (CMAKE_VERSION VERSION_LESS "3.21")
    # https://cmake.org/cmake/help/latest/variable/PROJECT_IS_TOP_LEVEL.html
    string(COMPARE EQUAL ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR} PROJECT_IS_TOP_LEVEL)
endif()

option(VMA_HPP_ENABLE_INSTALL "Install VulkanMemoryAllocator-Hpp" ${PROJECT_IS_TOP_LEVEL})
if (VMA_HPP_ENABLE_INSTALL)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)

    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    install(TARGETS VulkanMemoryAllocator-Hpp EXPORT VulkanMemoryAllocator-HppConfig INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    install(EXPORT VulkanMemoryAllocator-HppConfig NAMESPACE "VulkanMemoryAllocator-Hpp::" DESTINATION "share/cmake/VulkanMemoryAllocator-Hpp")

    write_basic_package_version_file(VulkanMemoryAllocator-HppConfigVersion.cmake COMPATIBILITY SameMajorVersion ARCH_INDEPENDENT)
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/VulkanMemoryAllocator-HppConfigVersion.cmake" DESTINATION "share/cmake/VulkanMemoryAllocator-Hpp")
endif()
