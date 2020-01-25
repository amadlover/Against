# CMakeList.txt : CMake project for AgainstCMake, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

include_directories($ENV{VULKAN_SDK}/Include)
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/External/glm/)

link_directories($ENV{VULKAN_SDK}/Lib32)

add_executable (Against WIN32 "error.c" "event.c" "game.c" "graphics.c" "list.c" "main_menu.c" "maths.c" "maths.cpp" "splash_screen.c" "test.c" "utils.c" "main.c")
target_link_libraries(Against vulkan-1)
add_compile_definitions(_CRT_SECURE_NO_WARNINGS)