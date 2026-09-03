cmake_minimum_required(VERSION 4.4)

project(Orion VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# FOR NVIM
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# FOR VULKAN

find_package(Vulkan REQUIRED)

add_subdirectory(vendor/glfw)
add_subdirectory(vendor/glm)
add_subdirectory(vendor/vkb)
add_subdirectory(vendor/spdlog)

file(GLOB_RECURSE ENGINE_SOURCES src/*.cpp src/*.h)

# add_executable(Orion src/main.cpp src/Engine/OApplication.h src/Engine/OVulkanContext.h src/Engine/OApplication.cpp src/Engine/OVulkanContext.cpp)

add_executable(Orion ${ENGINE_SOURCES})

target_include_directories(Orion PUBLIC src/Engine vendor/glfw/include)
target_link_libraries(Orion PRIVATE Vulkan::Vulkan vk-bootstrap::vk-bootstrap glfw glm spdlog::spdlog)
