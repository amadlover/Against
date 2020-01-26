# CMakeList.txt : CMake project for AgainstCMake, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

include_directories (${CMAKE_SOURCE_DIR})
include_directories ($ENV{VULKAN_SDK}/Include)

link_directories ($ENV{VULKAN_SDK}/Lib32)
link_directories ($ENV{VULKAN_SDK}/Lib)

add_executable (Against "event.c" "game.c" "graphics.c" "graphics_utils.c" "list.c" "log.c" "main.c" "main_menu.c" "splash_screen.c" "utils.c")
add_compile_definitions (Against _CRT_SECURE_NO_WARNINGS)
target_link_libraries (Against vulkan-1)

# TODO: Add tests and install targets if needed.
