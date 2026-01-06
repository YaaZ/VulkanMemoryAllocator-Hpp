cmake_minimum_required(VERSION 3.8...3.30 FATAL_ERROR)

project(
    VulkanMemoryAllocator-Hpp
    VERSION 3.3.0
    DESCRIPTION "C++ bindings for the VulkanMemoryAllocator library."
    LANGUAGES CXX)
set(VMA_HPP_PROJECT_VERSION ${PROJECT_VERSION} CACHE INTERNAL "VulkanMemoryAllocator Version")

option(VMA_HPP_ENABLE_INSTALL "Install VulkanMemoryAllocator-Hpp" OFF)

add_library(VulkanMemoryAllocator-Hpp INTERFACE)
add_library(VulkanMemoryAllocator-Hpp::VulkanMemoryAllocator-Hpp ALIAS VulkanMemoryAllocator-Hpp)
target_include_directories(VulkanMemoryAllocator-Hpp INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_link_libraries(VulkanMemoryAllocator-Hpp INTERFACE GPUOpen::VulkanMemoryAllocator)

if (TARGET Vulkan::HppModule)
    message(STATUS "VMA-Hpp module is available")

    # Set up VulkanMemoryAllocator-HppModule.
    add_library(VulkanMemoryAllocator-HppModule OBJECT)
    add_library(VulkanMemoryAllocator::HppModule ALIAS VulkanMemoryAllocator-HppModule)
    target_sources(VulkanMemoryAllocator-HppModule PUBLIC
            FILE_SET CXX_MODULES
            FILES "${CMAKE_CURRENT_SOURCE_DIR}/vk_mem_alloc.cppm")
    target_compile_features(VulkanMemoryAllocator-HppModule PUBLIC cxx_std_23)
    set_target_properties(VulkanMemoryAllocator-HppModule PROPERTIES CXX_MODULE_STD ON)
    endif ()
endif ()

if (VMA_HPP_ENABLE_INSTALL)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)

    if(TARGET VulkanMemoryAllocator-HppModule)
        set(CPPM_PATTERN "*.cppm")
    else()
        set(CPPM_PATTERN "")
    endif()

    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} PATTERN "${CPPM_PATTERN}" EXCLUDE PATTERN "CMakeLists.txt" EXCLUDE)
    install(TARGETS VulkanMemoryAllocator-Hpp EXPORT VulkanMemoryAllocator-HppConfig INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

    if(TARGET VulkanMemoryAllocator-HppModule)
        install(TARGETS VulkanMemoryAllocator-HppModule EXPORT VulkanMemoryAllocator-HppConfig FILE_SET CXX_MODULES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
        install(EXPORT VulkanMemoryAllocator-HppConfig NAMESPACE "VulkanMemoryAllocator-Hpp::" DESTINATION "share/cmake/VulkanMemoryAllocator-Hpp" CXX_MODULES_DIRECTORY "")
    else()
        install(EXPORT VulkanMemoryAllocator-HppConfig NAMESPACE "VulkanMemoryAllocator-Hpp::" DESTINATION "share/cmake/VulkanMemoryAllocator-Hpp")
    endif()

    write_basic_package_version_file(VulkanMemoryAllocator-HppConfigVersion.cmake COMPATIBILITY SameMajorVersion ARCH_INDEPENDENT)
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/VulkanMemoryAllocator-HppConfigVersion.cmake" DESTINATION "share/cmake/VulkanMemoryAllocator-Hpp")
endif ()
