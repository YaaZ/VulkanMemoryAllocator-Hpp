cmake_minimum_required(VERSION 3.8...3.30 FATAL_ERROR)

project(
    VulkanMemoryAllocator-Hpp
    VERSION 3.3.0
    DESCRIPTION "C++ bindings for the VulkanMemoryAllocator library."
    LANGUAGES CXX)
set(VMA_HPP_PROJECT_VERSION ${PROJECT_VERSION} CACHE INTERNAL "VulkanMemoryAllocator Version")

option(VMA_HPP_ENABLE_INSTALL "Install VulkanMemoryAllocator-Hpp" "${PROJECT_IS_TOP_LEVEL}")
option(VMA_HPP_ENABLE_MODULE "Enables building of the VMA-Hpp C++20 module; requires minimum CMake version 3.28" OFF)

add_library(VulkanMemoryAllocator-Hpp INTERFACE)
add_library(VulkanMemoryAllocator-Hpp::VulkanMemoryAllocator-Hpp ALIAS VulkanMemoryAllocator-Hpp)
target_include_directories(VulkanMemoryAllocator-Hpp INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

if (VMA_HPP_ENABLE_MODULE)
    # Set up VulkanMemoryAllocator-HppModule.
    add_library(VulkanMemoryAllocator-HppModule OBJECT)
    add_library(VulkanMemoryAllocator-Hpp::VulkanMemoryAllocator-HppModule ALIAS VulkanMemoryAllocator-HppModule)
    target_sources(VulkanMemoryAllocator-HppModule
            PUBLIC
            FILE_SET module
            TYPE CXX_MODULES
            FILES "${CMAKE_CURRENT_SOURCE_DIR}/vk_mem_alloc.cppm")
    target_link_libraries(VulkanMemoryAllocator-HppModule PUBLIC VulkanMemoryAllocator-Hpp)

    # Clang 16's module support can be broken with extensions enabled.
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL "16.0")
        set_target_properties(VulkanMemoryAllocator-HppModule PROPERTIES CXX_EXTENSIONS OFF)
    endif()

    # VulkanMemoryAllocator dependency.
    if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/vk_mem_alloc.h")
        if (NOT TARGET GPUOpen::VulkanMemoryAllocator)
            find_package(VulkanMemoryAllocator CONFIG REQUIRED)
        endif ()
        target_link_libraries(VulkanMemoryAllocator-HppModule PUBLIC GPUOpen::VulkanMemoryAllocator)
    endif ()

    # Vulkan-Hpp dependency.
    if (TARGET Vulkan::HppModule)
        # Also use std module if Vulkan-HppModule does.
        get_target_property(VMA_HPP_STD_MODULE Vulkan::HppModule CXX_MODULE_STD)
        get_target_property(VMA_HPP_STD_23 Vulkan::HppModule COMPILE_FEATURES)
        list(FIND VMA_HPP_STD_23 cxx_std_23 VMA_HPP_STD_23)
        if (VMA_HPP_STD_MODULE AND NOT VMA_HPP_STD_23 EQUAL -1)
            message("-- Building VMA-Hpp module with Vulkan module (std on)")
            target_compile_features(VulkanMemoryAllocator-HppModule
                    PRIVATE   cxx_std_23
                    INTERFACE cxx_std_20) # Only C++20 is required to consume this module.
            set_target_properties(VulkanMemoryAllocator-HppModule PROPERTIES CXX_MODULE_STD ON)
        else ()
            message("-- Building VMA-Hpp module with Vulkan module (std off)")
            target_compile_features(VulkanMemoryAllocator-HppModule PUBLIC cxx_std_20)
        endif ()
        target_link_libraries(VulkanMemoryAllocator-HppModule PUBLIC Vulkan::HppModule)
        target_compile_definitions(VulkanMemoryAllocator-HppModule PUBLIC VMA_HPP_ENABLE_VULKAN_HPP_MODULE)
    else ()
        message("-- Building VMA-Hpp module with Vulkan headers")
        if (NOT TARGET Vulkan::Headers)
            find_package(Vulkan REQUIRED COMPONENTS)
        endif ()
        target_compile_features(VulkanMemoryAllocator-HppModule PUBLIC cxx_std_20)
        target_link_libraries(VulkanMemoryAllocator-HppModule PUBLIC Vulkan::Headers)
    endif ()
endif ()

#if (VMA_HPP_ENABLE_INSTALL)
#    include(GNUInstallDirs)
#    include(CMakePackageConfigHelpers)
#
#    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
#    install(TARGETS VulkanMemoryAllocator-Hpp EXPORT VulkanMemoryAllocator-HppConfig INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
#    install(EXPORT VulkanMemoryAllocator-HppConfig NAMESPACE "VulkanMemoryAllocator-Hpp::" DESTINATION "share/cmake/VulkanMemoryAllocator-Hpp")
#
#    write_basic_package_version_file(VulkanMemoryAllocator-HppConfigVersion.cmake COMPATIBILITY SameMajorVersion ARCH_INDEPENDENT)
#    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/VulkanMemoryAllocator-HppConfigVersion.cmake" DESTINATION "share/cmake/VulkanMemoryAllocator-Hpp")
#endif ()
