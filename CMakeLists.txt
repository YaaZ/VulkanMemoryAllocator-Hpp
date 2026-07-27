cmake_minimum_required(VERSION 3.8...3.30 FATAL_ERROR)

project(VulkanMemoryAllocator-Hpp-Generator VERSION 3.4.0 LANGUAGES CXX)

if (CMAKE_VERSION VERSION_LESS "3.21")
    # https://cmake.org/cmake/help/latest/variable/PROJECT_IS_TOP_LEVEL.html
    string(COMPARE EQUAL ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR} PROJECT_IS_TOP_LEVEL)
endif ()

option(VMA_HPP_GENERATOR_BUILD   "Build the HPP generator" ${PROJECT_IS_TOP_LEVEL})
option(VMA_HPP_RUN_GENERATOR     "Run the HPP generator"   ${PROJECT_IS_TOP_LEVEL})
option(VMA_HPP_SAMPLES_BUILD     "Build samples"           ${PROJECT_IS_TOP_LEVEL})
option(VMA_HPP_DO_UPDATE         "Update VMA submodule"    OFF)
set(VMA_HPP_INPUT_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/VulkanMemoryAllocator/include/vk_mem_alloc.h" CACHE FILEPATH "Input vk_mem_alloc.h file")

if (VMA_HPP_DO_UPDATE)
    include(tools/UpdateSubmodule.cmake)
endif ()

if (NOT TARGET Vulkan::Headers AND NOT TARGET Vulkan::Hpp)
    # Set up Vulkan targets.
    # Use -DFETCHCONTENT_SOURCE_DIR_VULKAN=@ to use find_package, or pass a custom path
    if ("${FETCHCONTENT_SOURCE_DIR_VULKAN}" STREQUAL "@")
        find_package(VulkanHeaders CONFIG)
        if (NOT VulkanHeaders_FOUND)
            find_package(Vulkan)
        endif ()
    else ()
        include(FetchContent)
        FetchContent_Declare(
                vulkan
                GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Headers.git
                GIT_TAG main
                GIT_SHALLOW TRUE
        )
        FetchContent_MakeAvailable(vulkan)
    endif ()
endif ()
if (TARGET Vulkan::Hpp)
    set(VMA_HPP_VULKAN_HEADERS Vulkan::Hpp)
elseif (TARGET Vulkan::Headers)
    set(VMA_HPP_VULKAN_HEADERS Vulkan::Headers)
endif ()
if (TARGET VulkanHppModule)
    set(VMA_HPP_VULKAN_MODULE VulkanHppModule)
elseif (TARGET Vulkan-HppModule)
    set(VMA_HPP_VULKAN_MODULE Vulkan-HppModule)
endif ()

add_subdirectory(VulkanMemoryAllocator)
add_subdirectory(include)

if (NOT ${PROJECT_VERSION} STREQUAL ${VMA_HPP_PROJECT_VERSION})
    message(FATAL_ERROR "VulkanMemoryAllocator-Hpp-Generator and VulkanMemoryAllocator-Hpp versions do not match (${PROJECT_VERSION} vs ${VMA_HPP_PROJECT_VERSION})")
endif ()

if (VMA_HPP_DO_UPDATE)
    function(generate_files)
        include(tools/GenerateFiles.cmake)
    endfunction()
    generate_files()
endif ()

if (VMA_HPP_GENERATOR_BUILD)
    add_executable(VmaHppGenerator tools/VmaHppGenerator.cpp)
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
    add_custom_target(GenerateVmaHpp ALL DEPENDS ${VMA_HPP_GENERATED_FILES})
    add_dependencies(VulkanMemoryAllocator-Hpp GenerateVmaHpp)
endif ()

if (VMA_HPP_SAMPLES_BUILD)
    if (TARGET VulkanMemoryAllocator-HppModule)
        add_library(VulkanMemoryAllocator-Hpp-Samples STATIC)
        target_link_libraries(VulkanMemoryAllocator-Hpp-Samples PUBLIC VulkanMemoryAllocator-HppModule ${VMA_HPP_VULKAN_MODULE})
        target_compile_definitions(VulkanMemoryAllocator-Hpp-Samples PUBLIC USE_MODULES)
        target_compile_definitions(VulkanMemoryAllocator-HppModule PUBLIC VK_NO_PROTOTYPES)
        target_compile_definitions(${VMA_HPP_VULKAN_MODULE} PUBLIC VK_NO_PROTOTYPES)
        target_compile_features(VulkanMemoryAllocator-Hpp-Samples PUBLIC cxx_std_23)
    else ()
        add_library(VulkanMemoryAllocator-Hpp-Samples INTERFACE)
        target_link_libraries(VulkanMemoryAllocator-Hpp-Samples INTERFACE VulkanMemoryAllocator-Hpp GPUOpen::VulkanMemoryAllocator ${VMA_HPP_VULKAN_HEADERS})
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
