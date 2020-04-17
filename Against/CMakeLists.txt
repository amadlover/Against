# CMakeList.txt : CMake project for AgainstCMake, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

include_directories (${CMAKE_SOURCE_DIR})
include_directories ($ENV{VULKAN_SDK}/Include)
include_directories (${CMAKE_SOURCE_DIR}/External/glm)
include_directories (${CMAKE_SOURCE_DIR}/External/cgltf)
include_directories (${CMAKE_SOURCE_DIR}/External/cglm)
include_directories (${CMAKE_SOURCE_DIR}/External/stb)

link_directories ($ENV{VULKAN_SDK}/Lib32)
link_directories ($ENV{VULKAN_SDK}/Lib)

set (H "asset.h" "enums.h" "error.h" "game.h" "common_graphics.h" "graphics_utils.h" "log.h" "utils.h" "test_scene.h" "gltf.h" "graphics_pipeline.h")
set (C "asset.c" "game.c" "common_graphics.c" "graphics_utils.c" "log.c" "main.c" "utils.c" "test_scene.c" "gltf.c")

add_executable (Against WIN32 ${H} ${C})
add_compile_definitions (Against _CRT_SECURE_NO_WARNINGS UNICODE WIN32_LEAN_AND_MEAN)
target_link_libraries (Against vulkan-1)
target_link_libraries (Against Shlwapi)

add_custom_command (TARGET Against POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/Against/Assets" $<TARGET_FILE_DIR:Against>)