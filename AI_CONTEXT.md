Orion Engine — AI Development Context
Project Overview

Orion Engine is a Vulkan-based game engine written in C++.

The project is currently in the early foundation/setup stage. The goal is to build a clean, modular game engine rather than putting everything into a single renderer or application class.

The engine name is:

Orion Engine


The code uses an O prefix for engine classes, for example:

OApplication
OVulkanContext
OLog


The project also uses the Orion namespace.

Current Technology Stack

The project currently uses:

C++
CMake
GLFW
Vulkan
Vulkan Bootstrap (vk-bootstrap)
GLM
spdlog

Dependencies are stored in the repository through Git submodules/vendor directories.

Current vendor structure is approximately:

vendor/
├── glfw/
├── glm/
├── spdlog/
└── vkb/


The project should be cloneable using:

git clone --recurse-submodules <repository-url>


After cloning with submodules, the project should be ready to configure and build, assuming the system has the required Vulkan environment and build tools.

Current Project Structure

Current source files are approximately:

src/
├── Engine/
│   ├── OApplication.h
│   ├── OApplication.cpp
│   ├── OVulkanContext.h
│   ├── OVulkanContext.cpp
│   ├── OLog.h
│   └── OLog.cpp
│
└── main.cpp


The project will eventually be expanded into more engine modules.

Potential future structure:

src/
├── Engine/
│   ├── Core/
│   ├── Renderer/
│   ├── Scene/
│   ├── Math/
│   ├── Input/
│   ├── Assets/
│   ├── Utils/
│   └── Platform/
│
└── main.cpp

OApplication

OApplication is responsible for:

Initializing GLFW
Creating the GLFW window
Creating/owning the Vulkan context
Running the main application loop
Cleaning everything up

The application currently creates a:

1280x720


window named:

Orion Engine


GLFW is configured with:

glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);


because Vulkan is being used instead of OpenGL.

The current application loop is essentially:

while (!glfwWindowShouldClose(m_Window))
{
    glfwPollEvents();
}


The application owns:

std::unique_ptr<OVulkanContext> m_VulkanContext;

OVulkanContext

OVulkanContext is responsible for the high-level Vulkan initialization.

Current responsibilities:

Store the GLFW window
Create the Vulkan instance through Vulkan Bootstrap
Store the Vulkan instance
Store the Vulkan debug messenger
Destroy the Vulkan instance during shutdown

Current members include:

GLFWwindow* m_Window = nullptr;

vkb::Instance m_Instance;

VkInstance m_VkInstance = VK_NULL_HANDLE;
VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;


Vulkan Bootstrap is being used specifically to avoid manually writing a large amount of Vulkan instance/device initialization boilerplate.

Vulkan Bootstrap

The project uses Vulkan Bootstrap instead of manually implementing every part of Vulkan instance setup.

The intended initialization is roughly:

vkb::InstanceBuilder builder;

auto instanceResult =
    builder
        .set_app_name("Orion Engine")
        .set_engine_name("Orion Engine")
        .request_validation_layers()
        .require_api_version(1, 3, 0)
        .build();


The exact API for the custom debug callback depends on the installed vk-bootstrap version.

Do NOT blindly assume that:

.set_debug_callback(...)


exists.

If it does not compile, inspect the version/commit of the vendored vk-bootstrap before changing the architecture.

Logging

The project uses spdlog.

A custom wrapper called:

OLog


was created so that the rest of the engine does not need to interact directly with spdlog everywhere.

OLog.h currently has:

#pragma once

#include <memory>
#include <spdlog/logger.h>

namespace Orion
{
    class OLog
    {
    public:
        static void Init();

        static std::shared_ptr<spdlog::logger>& GetLogger()
        {
            return s_Logger;
        }

    private:
        static std::shared_ptr<spdlog::logger> s_Logger;
    };
    }

#define ORION_TRACE(...)    ::Orion::OLog::GetLogger()->trace(__VA_ARGS__)
#define ORION_INFO(...)     ::Orion::OLog::GetLogger()->info(__VA_ARGS__)
#define ORION_WARN(...)     ::Orion::OLog::GetLogger()->warn(__VA_ARGS__)
#define ORION_ERROR(...)    ::Orion::OLog::GetLogger()->error(__VA_ARGS__)
#define ORION_CRITICAL(...) ::Orion::OLog::GetLogger()->critical(__VA_ARGS__)


OLog.cpp defines the static member:

std::shared_ptr<spdlog::logger> OLog::s_Logger = nullptr;


and initializes the logger with:

spdlog::set_pattern("[%T] [%^%l%$] %v");

s_Logger = spdlog::stdout_color_mt("ORION");
s_Logger->set_level(spdlog::level::trace);

Important Logger Bug That Was Fixed

Initially OLog.cpp incorrectly contained:

std::shared_ptr<spdlog::logger> s_Logger;


This created:

Orion::s_Logger


instead of defining:

Orion::OLog::s_Logger


The correct definition is:

std::shared_ptr<spdlog::logger> OLog::s_Logger = nullptr;


This was causing linker errors such as:

undefined reference to `Orion::OLog::s_Logger'
undefined reference to `Orion::OLog::Init()'


The issue was eventually also related to CMake not recompiling the newly added OLog.cpp.

Deleting the build directory and regenerating CMake fixed the stale build issue.

If new .cpp files are added and the linker says their functions are undefined, first verify that the corresponding .cpp.o appears in the linker command.

CMake

The project uses CMake.

A useful approach for automatically finding source files is:

file(GLOB_RECURSE ORION_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
)


This avoids manually adding every .cpp and .h file to CMakeLists.txt.

CONFIGURE_DEPENDS is preferred because it helps CMake detect newly added files.

If CMake gets into a stale state, clean the build:

rm -rf build
cmake -B build
cmake --build build


The user specifically found that doing a clean CMake build fixed the missing OLog.cpp linker problem.

Vulkan Debug Messenger Goal

The current goal is to replace Vulkan Bootstrap's default debug messenger behavior with a custom Vulkan debug callback that routes validation messages through OLog/spdlog.

The desired architecture is:

Vulkan Validation Layers
        |
        v
Custom Vulkan Debug Callback
        |
        v
OLog
        |
        v
spdlog
        |
        v
Terminal


The user explicitly wants to replace the default Bootstrap debug messenger.

Therefore, do NOT use:

.use_default_debug_messenger()


when implementing the custom callback.

Validation layers should still be enabled:

.request_validation_layers()


Validation layers and the debug messenger are separate concepts.

The callback should map Vulkan severity to Orion logging levels:

Vulkan VERBOSE  -> ORION_TRACE
Vulkan INFO     -> ORION_INFO
Vulkan WARNING  -> ORION_WARN
Vulkan ERROR    -> ORION_ERROR


The callback should return:

VK_FALSE


so Vulkan does not abort the operation.

Initialization Order

The logger must be initialized before Vulkan creates its debug messenger because the callback may immediately receive messages.

The intended order is:

main()
 |
 +-- OLog::Init()
 |
 +-- OApplication
       |
       +-- GLFW initialization
       |
       +-- Window creation
       |
       +-- OVulkanContext
             |
             +-- Vulkan instance
             |
             +-- Custom debug messenger
             |
             +-- Vulkan validation messages -> OLog


Therefore main.cpp should initialize the logger first:

#include "Engine/OApplication.h"
#include "Engine/OLog.h"

int main()
{
    Orion::OLog::Init();

    ORION_INFO("Starting Orion Engine...");

    Orion::OApplication app;
    app.Run();

    ORION_INFO("Orion Engine shutting down.");

    return 0;
}

Current State

Completed:

GLFW installed
GLM installed
Vulkan configured
Vulkan Bootstrap installed
spdlog installed
CMake project configured
OApplication created
OVulkanContext created
OLog created
Basic logging macros created
Vulkan validation layers enabled/requested
Goal established to route Vulkan validation messages through custom OLog
CMake source-file management improved
Basic window initialization works
Vulkan instance initialization works

The project is currently at the early Vulkan initialization stage.

Next Vulkan Roadmap

The next implementation steps should be:

1. Custom Vulkan debug messenger
2. GLFW VkSurfaceKHR
3. Physical device selection
4. Queue family discovery
5. Logical device creation
6. Graphics queue
7. Present queue
8. Swapchain
9. Swapchain images
10. Image views
11. Depth buffer
12. Render pass / dynamic rendering
13. Graphics pipeline
14. Shader loading
15. Command pool
16. Command buffers
17. Synchronization
18. Clear screen
19. First triangle


Do not jump directly into a large engine architecture before getting the Vulkan renderer working.

The immediate goal should be:

GLFW Window
      |
      v
Vulkan Instance
      |
      v
Validation + Custom Logger
      |
      v
Surface
      |
      v
GPU
      |
      v
Logical Device
      |
      v
Swapchain
      |
      v
Clear Screen
      |
      v
Triangle

Future Renderer Architecture

As the renderer grows, avoid putting every Vulkan object into OVulkanContext.

The intended direction is to split responsibilities into classes such as:

Renderer/
├── OVulkanContext
├── OVulkanDevice
├── OVulkanSwapchain
├── OVulkanPipeline
├── OVulkanCommandPool
├── OVulkanBuffer
├── OVulkanImage
├── OVulkanTexture
└── OVulkanRenderer


Conceptually:

OApplication
      |
      v
OVulkanRenderer
      |
      +--- OVulkanContext
      +--- OVulkanDevice
      +--- OVulkanSwapchain
      +--- OVulkanPipeline
      +--- ...


OVulkanContext should remain relatively focused on Vulkan instance/context-level functionality rather than becoming a giant class containing the entire renderer.

Future Libraries

Potential libraries discussed for later stages:

VMA (Vulkan Memory Allocator) for GPU memory allocation
stb_image for texture loading
fastgltf or tinygltf for glTF models
Dear ImGui for editor/debug UI
spdlog for logging

Do not add all of these immediately. Add dependencies when the corresponding engine feature needs them.

Engine Philosophy

The project should prioritize:

Clean architecture
Small focused classes
Explicit ownership of Vulkan resources
RAII where appropriate
Good logging
Validation layers during development
Minimal unnecessary abstraction
Learning Vulkan fundamentals while building the engine
Avoiding giant source files
Keeping renderer components modular

The user is actively learning Vulkan, so explain important Vulkan concepts rather than hiding everything behind abstractions.

Vulkan Bootstrap is being used to reduce repetitive initialization boilerplate, but the user still wants to understand what Vulkan is doing.

Immediate Task For Next AI

The most immediate task is:

Finish the custom Vulkan debug messenger integration

The goal is:

Vulkan validation message
        ↓
Custom callback
        ↓
ORION_TRACE / INFO / WARN / ERROR
        ↓
spdlog


The default vk-bootstrap debug messenger should NOT be used.

Before providing code for the callback, determine the exact version/commit of the vendored vk-bootstrap if necessary, because the custom debug callback API may differ between versions.

After that, move to:

VkSurfaceKHR


created from the existing GLFW window.

Then continue toward physical device and logical device creation.

Useful Commands

Clean build:

rm -rf build
cmake -B build
cmake --build build


Clone project with all dependencies:

git clone --recurse-submodules <repository-url>


Initialize submodules after a normal clone:

git submodule update --init --recursive

Current Mental Model

The engine currently looks like:

                 Orion Engine
                      |
                 OApplication
                      |
              +-------+-------+
              |               |
           GLFW Window    OLog/spdlog
              |
              v
       OVulkanContext
              |
              v
       Vulkan Bootstrap
              |
              v
       Vulkan Instance
              |
              v
   Custom Debug Messenger
              |
              v
          OLog/spdlog


The next major expansion is:

OVulkanContext
      |
      +-- Surface
      |
      +-- Physical Device
      |
      +-- Logical Device
      |
      +-- Queues
      |
      +-- Swapchain


This document represents the current state and decisions of Orion Engine and should be used as context when continuing development with another AI agent.