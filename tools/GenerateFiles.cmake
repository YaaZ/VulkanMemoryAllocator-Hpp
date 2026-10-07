
function(generate_configuration_table)
    # Find testing configurations in the GH workflow.
    file(READ "${CMAKE_CURRENT_SOURCE_DIR}/.github/workflows/build-custom-vulkan.yml" WORKFLOW_SOURCE)
    set(REGEX "build +\\$\{{ *env\.vk *\\|\\| *env\.modules *&& *'v?([^']+)' *\\|\\| *'v([^']+)'}} *([^\n$]*)")
    string(REGEX MATCHALL "${REGEX}" CONFIGS ${WORKFLOW_SOURCE})
    # Build a table of configurations.
    set(CONFIG_TABLE "\n\n  <table>
    <tr><th colspan='2'>Minimal Vulkan</th><th>Tested configurations</th></tr>
    <tr><th>#include</th><th>import</th><th>Flags</th></tr>")
    foreach(CONFIG ${CONFIGS})
        string(REGEX MATCH "${REGEX}" _ ${CONFIG})
        if ("${CMAKE_MATCH_3}" STREQUAL "")
            set(MIN_VULKAN "${CMAKE_MATCH_2}" PARENT_SCOPE) # Remember Vulkan version for default configuration.
        endif()
        string(APPEND CONFIG_TABLE "\n    <tr><td>${CMAKE_MATCH_2}</td><td>${CMAKE_MATCH_1}</td><td>${CMAKE_MATCH_3}</td></tr>")
    endforeach()
    set(CONFIG_TABLE "${CONFIG_TABLE}\n  </table>" PARENT_SCOPE)
endfunction()
generate_configuration_table()

# Find VMA version in the VMA header.
file(READ "${VMA_HPP_INPUT_HEADER}" VMA_SOURCE)
string(REGEX MATCH "<b>Version\\s*([^<]+)\\s*</b>" _ ${VMA_SOURCE})
string(STRIP ${CMAKE_MATCH_1} VMA_VERSION)

# Update versions in the README.
function(replace_readme PLACEHOLDER CONTENT)
    string(REGEX REPLACE "<!--${PLACEHOLDER}-->([^<]*<[^!])*[^<]*<!--/${PLACEHOLDER}-->" "<!--${PLACEHOLDER}-->${CONTENT}<!--/${PLACEHOLDER}-->" README_SOURCE "${README_SOURCE}")
    set(README_SOURCE "${README_SOURCE}" PARENT_SCOPE)
endfunction()
message(STATUS "Updating README with VMA ${VMA_VERSION} and Vulkan ${MIN_VULKAN}")
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/README.md" README_SOURCE)
string(REPLACE ";" "\\;" README_SOURCE "${README_SOURCE}") # Do not mess up semicolons in the README
replace_readme(VER "${VMA_VERSION}")
replace_readme(MIN_VK "${MIN_VULKAN}")
replace_readme(MIN_VK_TABLE "${CONFIG_TABLE}")
file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/README.md" ${README_SOURCE})

# Generate "imported" utility header.
set(IMPORTED_HEADER "// Generated from the list of VMA-Hpp headers (vk_mem_alloc*.hpp).
// See https://clang.llvm.org/docs/StandardCPlusPlusModules.html#providing-a-header-to-skip-parsing-redundant-headers")
file(GLOB HEADER_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/include" "${CMAKE_CURRENT_SOURCE_DIR}/include/vk_mem_alloc*.hpp")
foreach(HEADER_FILE ${HEADER_FILES})
    if (NOT ${HEADER_FILE} STREQUAL "vk_mem_alloc_imported.hpp" AND
            NOT ${HEADER_FILE} STREQUAL "vk_mem_alloc_static_assertions.hpp")
        string(REGEX MATCH "vk_mem_alloc(.*)\.hpp" _ ${HEADER_FILE})
        string(TOUPPER "${CMAKE_MATCH_1}" HEADER_GUARD)
        string(APPEND IMPORTED_HEADER "

#ifndef VULKAN_MEMORY_ALLOCATOR${HEADER_GUARD}_HPP
#define VULKAN_MEMORY_ALLOCATOR${HEADER_GUARD}_HPP
#endif")
    endif ()
endforeach()
file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/include/vk_mem_alloc_imported.hpp" ${IMPORTED_HEADER})