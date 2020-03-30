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

set (H "asset.h" "enums.h" "error.h" "game.h" "common_graphics.h" "graphics_utils.h" "list.h" "log.h" "main_menu.h" "maths.hpp" "splash_screen.h" "splash_screen_graphics.h" "utils.h")
set (C "asset.c" "game.c" "common_graphics.c" "graphics_utils.c" "list.c" "log.c" "main.c" "main_menu.c" "splash_screen.c" "splash_screen_graphics.c" "utils.c")

add_executable (Against WIN32 ${H} ${C})
add_compile_definitions (Against _CRT_SECURE_NO_WARNINGS UNICODE WIN32_LEAN_AND_MEAN)
target_link_libraries (Against vulkan-1)
target_link_libraries (Against Shlwapi)

add_custom_command (TARGET Against POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/Against/Assets" $<TARGET_FILE_DIR:Against>)