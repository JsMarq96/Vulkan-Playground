cmake_minimum_required(VERSION 3.13)

SET(CMAKE_CXX_FLAGS  "-g")
set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(VULKAN_PLAYGROUND)

file(GLOB CPP_SOURCES "src/*.cpp")
file(GLOB CPP_SUBFOLDER_SOURCES "src/**/*.cpp")
file(GLOB H_SOURCES "src/*.h")
file(GLOB C_SOURCES "src/*.c")
file(GLOB C_SUBFOLDER_SOURCES "src/**/*.c")
set(includes_dir "includes/")

add_executable(VULKAN_PLAYGROUND ${CPP_SOURCES} ${CPP_SUBFOLDER_SOURCES} ${C_SOURCES} ${C_SUBFOLDER_SOURCES} ${H_SOURCES})

add_subdirectory(glm)
target_link_libraries(VULKAN_PLAYGROUND glm)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory("glfw")
target_link_libraries(VULKAN_PLAYGROUND "glfw")

find_package(Vulkan REQUIRED)
target_link_libraries(VULKAN_PLAYGROUND ${Vulkan_LIBRARIES})
include_directories(${Vulkan_INCLUDE_DIR})