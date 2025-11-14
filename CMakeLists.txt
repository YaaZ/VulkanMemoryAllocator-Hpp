cmake_minimum_required(VERSION 3.8...3.30 FATAL_ERROR)

project(VulkanMemoryAllocator-Hpp-Generator VERSION 3.3.0 LANGUAGES CXX)

if (CMAKE_VERSION VERSION_LESS "3.21")
    # https://cmake.org/cmake/help/latest/variable/PROJECT_IS_TOP_LEVEL.html
    string(COMPARE EQUAL ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR} PROJECT_IS_TOP_LEVEL)
endif ()

option(VMA_HPP_GENERATOR_BUILD   "Build the HPP generator" ${PROJECT_IS_TOP_LEVEL})
option(VMA_HPP_RUN_GENERATOR     "Run the HPP generator"   ${PROJECT_IS_TOP_LEVEL})
option(VMA_HPP_SAMPLES_BUILD     "Build samples"           ${PROJECT_IS_TOP_LEVEL})
option(VMA_HPP_DO_UPDATE         "Update VMA submodule"    OFF)
set(VMA_HPP_INPUT_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/VulkanMemoryAllocator/include/vk_mem_alloc.h" CACHE FILEPATH "Input vk_mem_alloc.h file")
set(VMA_HPP_VULKAN_MIN_VERSION "1.4.335"                         CACHE INTERNAL "Minimal Vulkan-Headers revision")
set(VMA_HPP_VULKAN_REVISION    "v${VMA_HPP_VULKAN_MIN_VERSION}" CACHE STRING   "Vulkan-Headers revision")
set(VMA_HPP_VMA_REVISION       "v${PROJECT_VERSION}"             CACHE STRING   "VulkanMemoryAllocator revision" FORCE)

if (VMA_HPP_DO_UPDATE)
    # Find Git.
    find_package(Git)
    if (NOT Git_FOUND)
        message(FATAL_ERROR "Git not found. VMA_HPP_DO_UPDATE is not available.")
    endif ()

    # Init the submodule.
    execute_process(COMMAND ${GIT_EXECUTABLE} submodule update
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" COMMAND_ERROR_IS_FATAL ANY)

    # Checkout proper revision.
    message(STATUS "Updating VulkanMemoryAllocator submodule to ${VMA_HPP_VMA_REVISION}")
    execute_process(COMMAND ${GIT_EXECUTABLE} -c advice.detachedHead=false checkout "${VMA_HPP_VMA_REVISION}"
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/VulkanMemoryAllocator" COMMAND_ERROR_IS_FATAL ANY)
    execute_process(COMMAND ${GIT_EXECUTABLE} pull
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/VulkanMemoryAllocator" ERROR_QUIET)
endif ()

if (${VMA_HPP_VULKAN_REVISION} STREQUAL "system")
    find_package(VulkanHeaders CONFIG)
    if (NOT VulkanHeaders_FOUND)
        find_package(Vulkan REQUIRED)
    endif ()
else ()
    include(FetchContent)
    FetchContent_Declare(
            vulkan
            GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Headers.git
            GIT_TAG ${VMA_HPP_VULKAN_REVISION}
            GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(vulkan)
endif ()

add_subdirectory(VulkanMemoryAllocator)
add_subdirectory(include)

if (NOT ${PROJECT_VERSION} STREQUAL ${VMA_HPP_PROJECT_VERSION})
    message(FATAL_ERROR "VulkanMemoryAllocator-Hpp-Generator and VulkanMemoryAllocator-Hpp versions do not match (${PROJECT_VERSION} vs ${VMA_HPP_PROJECT_VERSION})")
endif ()

if (VMA_HPP_DO_UPDATE)
    # Find VMA version in the VMA header.
    file(READ "${VMA_HPP_INPUT_HEADER}" VMA_HPP_VMA_SOURCE)
    string(REGEX MATCH "<b>Version\\s*([^<]+)\\s*</b>" _ ${VMA_HPP_VMA_SOURCE})
    string(STRIP ${CMAKE_MATCH_1} VMA_HPP_VMA_VERSION)

    # Update versions in the README.
    message(STATUS "Updating README with VMA ${VMA_HPP_VMA_VERSION} and Vulkan ${VMA_HPP_VULKAN_MIN_VERSION}")
    file(READ "${CMAKE_CURRENT_SOURCE_DIR}/README.md" VMA_HPP_README)
    string(REPLACE ";" "\\;" VMA_HPP_README "${VMA_HPP_README}") # Do not mess up semicolons in the README
    string(REGEX REPLACE "<!--VER-->[^<]*<!--/VER-->" "<!--VER-->${VMA_HPP_VMA_VERSION}<!--/VER-->" VMA_HPP_README "${VMA_HPP_README}")
    string(REGEX REPLACE "<!--VK-->[^<]*<!--/VK-->" "<!--VK-->${VMA_HPP_VULKAN_MIN_VERSION}<!--/VK-->" VMA_HPP_README "${VMA_HPP_README}")
    file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/README.md" ${VMA_HPP_README})

    # Generate "imported" utility header.
    set(VMA_HPP_IMPORTED_HEADER "// Generated from the list of VMA-Hpp headers (vk_mem_alloc*.hpp).
// See https://clang.llvm.org/docs/StandardCPlusPlusModules.html#providing-a-header-to-skip-parsing-redundant-headers")
    file(GLOB VMA_HPP_HEADER_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/include" "${CMAKE_CURRENT_SOURCE_DIR}/include/vk_mem_alloc*.hpp")
    foreach(VMA_HPP_HEADER_FILE ${VMA_HPP_HEADER_FILES})
        if (NOT ${VMA_HPP_HEADER_FILE} STREQUAL "vk_mem_alloc_imported.hpp" AND
            NOT ${VMA_HPP_HEADER_FILE} STREQUAL "vk_mem_alloc_static_assertions.hpp")
            string(REGEX MATCH "vk_mem_alloc(.*)\.hpp" _ ${VMA_HPP_HEADER_FILE})
            string(TOUPPER "${CMAKE_MATCH_1}" VMA_HPP_HEADER_GUARD)
            string(APPEND VMA_HPP_IMPORTED_HEADER "

#ifndef VULKAN_MEMORY_ALLOCATOR${VMA_HPP_HEADER_GUARD}_HPP
#define VULKAN_MEMORY_ALLOCATOR${VMA_HPP_HEADER_GUARD}_HPP
#endif")
        endif ()
    endforeach()
    file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/include/vk_mem_alloc_imported.hpp" ${VMA_HPP_IMPORTED_HEADER})
endif ()

if (VMA_HPP_GENERATOR_BUILD)
    add_executable(VmaHppGenerator VmaHppGenerator.cpp)
    set_property(TARGET VmaHppGenerator PROPERTY CXX_STANDARD 17)
    set_property(TARGET VmaHppGenerator PROPERTY CXX_STANDARD_REQUIRED ON)
    set_property(TARGET VmaHppGenerator PROPERTY CXX_EXTENSIONS OFF)
    target_compile_definitions(VmaHppGenerator PRIVATE BASE_PATH="${CMAKE_CURRENT_SOURCE_DIR}" INPUT_HEADER="${VMA_HPP_INPUT_HEADER}")
endif ()

if (VMA_HPP_RUN_GENERATOR)
    # Find generated files.
    set(VMA_HPP_GENERATED_FILES "")
    file(GLOB VMA_HPP_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/vk_mem_alloc*")
    foreach(VMA_HPP_FILE ${VMA_HPP_FILES})
        file(STRINGS ${VMA_HPP_FILE} VMA_HPP_FILE_GENERATED LIMIT_COUNT 1 REGEX "// Generated .*")
        if (VMA_HPP_FILE_GENERATED)
            list(APPEND VMA_HPP_GENERATED_FILES ${VMA_HPP_FILE})
        endif ()
    endforeach()

    add_custom_command(
            COMMAND VmaHppGenerator
            OUTPUT ${VMA_HPP_GENERATED_FILES}
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            COMMENT "Running VmaHppGenerator"
            DEPENDS VmaHppGenerator "${VMA_HPP_INPUT_HEADER}")
    add_custom_target(GenerateVmaHpp DEPENDS ${VMA_HPP_GENERATED_FILES})
    add_dependencies(VulkanMemoryAllocator-Hpp GenerateVmaHpp)
endif ()

if (VMA_HPP_SAMPLES_BUILD)
    if (TARGET VulkanMemoryAllocator-HppModule)
        add_library(VulkanMemoryAllocator-Hpp-Samples STATIC)
        target_link_libraries(VulkanMemoryAllocator-Hpp-Samples PUBLIC VulkanMemoryAllocator-HppModule Vulkan::HppModule)
        target_compile_definitions(VulkanMemoryAllocator-Hpp-Samples PUBLIC USE_MODULES)
        target_compile_definitions(VulkanMemoryAllocator-HppModule PUBLIC VK_NO_PROTOTYPES)
        target_compile_definitions(Vulkan-HppModule PUBLIC VK_NO_PROTOTYPES)
        target_compile_features(VulkanMemoryAllocator-Hpp-Samples PUBLIC cxx_std_23)
    else ()
        add_library(VulkanMemoryAllocator-Hpp-Samples INTERFACE)
        target_link_libraries(VulkanMemoryAllocator-Hpp-Samples INTERFACE VulkanMemoryAllocator-Hpp GPUOpen::VulkanMemoryAllocator Vulkan::Headers)
        target_compile_definitions(VulkanMemoryAllocator-Hpp-Samples INTERFACE VK_NO_PROTOTYPES)
    endif ()

    file(GLOB VMA_HPP_SAMPLES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/samples" "${CMAKE_CURRENT_SOURCE_DIR}/samples/*.cpp")
    foreach(VMA_HPP_SAMPLE_FILE ${VMA_HPP_SAMPLES})
        string(REPLACE ".cpp" "" VMA_HPP_SAMPLE_NAME ${VMA_HPP_SAMPLE_FILE})
        set(VMA_HPP_SAMPLE_NAME "Sample_${VMA_HPP_SAMPLE_NAME}")
        add_executable(${VMA_HPP_SAMPLE_NAME} "${CMAKE_CURRENT_SOURCE_DIR}/samples/${VMA_HPP_SAMPLE_FILE}")
        target_link_libraries(${VMA_HPP_SAMPLE_NAME} PRIVATE VulkanMemoryAllocator-Hpp-Samples)
    endforeach()
endif ()
