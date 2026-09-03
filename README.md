Orion Engine

Orion Engine is a Vulkan-based game engine written in C++.

Current Features
C++
CMake build system
GLFW for window creation
Vulkan for graphics
Vulkan Bootstrap for Vulkan initialization
GLM for mathematics
spdlog for logging
Custom Vulkan debug messenger connected to the Orion logger
Vulkan validation layers
Basic application framework
Basic Vulkan context
Project Structure
Orion/
├── src/
│   ├── Engine/
│   │   ├── OApplication.h
│   │   ├── OApplication.cpp
│   │   ├── OVulkanContext.h
│   │   ├── OVulkanContext.cpp
│   │   ├── OLog.h
│   │   └── OLog.cpp
│   │
│   └── main.cpp
│
├── vendor/
│   ├── glfw/
│   ├── glm/
│   ├── spdlog/
│   └── vkb/
│
├── CMakeLists.txt
└── README.md

Requirements
C++ compiler with C++17 or newer support
CMake
Vulkan SDK / Vulkan runtime
Git
Installation

Clone the repository with all submodules:

git clone --recurse-submodules <repository-url>
cd Orion


The repository includes its dependencies as Git submodules, so no additional dependency installation is required beyond having the Vulkan environment and build tools installed.

If the repository was already cloned without submodules:

git submodule update --init --recursive

Building

Configure the project:

cmake -B build


Build Orion Engine:

cmake --build build


Run the executable:

./build/Orion

Logging

Orion Engine uses spdlog for logging.

Example:

ORION_TRACE("Trace message");
ORION_INFO("Information message");
ORION_WARN("Warning message");
ORION_ERROR("Error message");
ORION_CRITICAL("Critical message");


Vulkan validation messages are routed through the Orion logger using a custom Vulkan debug callback.

Vulkan

Vulkan initialization is handled through Vulkan Bootstrap.

The current Vulkan context provides:

Vulkan instance creation
Vulkan API version selection
Validation layer support
Custom debug messenger
Vulkan debug logging through spdlog
Status

Orion Engine is currently in early development.

The next major renderer milestones are:

Vulkan surface
Physical device selection
Logical device
Queue management
Swapchain
Command buffers
Synchronization
Rendering pipeline
First rendered triangle