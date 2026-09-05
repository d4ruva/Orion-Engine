# Orion Engine

Orion Engine is a C++ game engine project built from the ground up with a focus on understanding and implementing the core systems behind a modern real-time engine.

The project is currently in active development, with Vulkan rendering being the primary focus.

## Current Status

Orion is in the early stages of development.

Current work includes:

- C++ engine architecture
- GLFW window creation
- Vulkan initialization
- Vulkan 1.3 support
- Vulkan validation layers
- Vulkan debug messenger
- Physical device selection
- Logical device creation
- Graphics queue selection
- Swapchain creation
- Swapchain image management
- Command pool creation
- Command buffer allocation
- Synchronization primitives
- Vulkan Synchronization 2
- Dynamic Rendering preparation
- Vulkan image layout transitions
- `vkQueueSubmit2`
- Swapchain image acquisition and presentation
- Basic GPU-side image clearing
- Initial graphics pipeline work
- Shader management
- Render pass infrastructure
- Custom logging infrastructure

The engine is currently capable of bringing up a Vulkan context and submitting GPU work to clear and present swapchain images.

## Goals

The long-term goal is to build a modular and extensible game engine while gaining a deep understanding of the underlying systems rather than hiding everything behind high-level abstractions.

Planned areas include:

- Rendering abstraction
- Vulkan renderer
- Resource management
- GPU memory allocation
- Buffer management
- Image and texture management
- Descriptor management
- Pipeline management
- Frame synchronization
- Multiple frames in flight
- Swapchain recreation
- Dynamic rendering
- Render graph
- Material system
- Shader system
- Asset management
- Scene system
- Entity/component architecture
- Input system
- Camera system
- Math library
- ECS
- Audio
- Physics
- Editor
- Serialization
- Profiling
- Debugging tools
- Multithreading
- Job system
- Eventually, higher-level gameplay systems

## Rendering

Vulkan is currently the main graphics API.

The renderer is being developed around modern Vulkan functionality, including:

- Vulkan 1.3
- Synchronization 2
- Dynamic Rendering
- Modern pipeline configuration
- Explicit image layout transitions
- Explicit synchronization
- Command buffer based rendering

The project intentionally keeps Vulkan code relatively explicit during development. This makes synchronization, resource lifetime, command submission, and GPU/CPU interaction easier to reason about before introducing higher-level abstractions.

## Dependencies

The project currently uses:

- C++
- Vulkan
- GLFW
- Vulkan-Hpp / Vulkan C API components as required by the renderer
- vk-bootstrap
- Custom engine logging

The exact dependency versions and build configuration are defined by the project's build system.

## Project Structure

The project is being organized around engine-level systems rather than a single monolithic renderer.

The architecture is expected to evolve significantly as more systems are introduced.

A simplified conceptual structure is:

```text
Orion/
├── Engine/
│   ├── Core/
│   ├── Graphics/
│   ├── Renderer/
│   ├── Platform/
│   └── ...
├── Sandbox/
├── Shaders/
├── Assets/
├── CMakeLists.txt
└── ...

The exact structure may change as the project grows.

Development Philosophy
Orion is primarily a learning-driven engine project.

The intention is not to immediately produce a production-ready engine. Instead, the project focuses on understanding why each system exists and how the systems interact.

Important principles include:

Prefer explicit ownership.
Understand resource lifetimes.
Make GPU synchronization explicit.
Keep Vulkan validation enabled during development.
Avoid hiding important rendering behavior behind premature abstractions.
Build abstractions only after understanding the underlying API.
Keep systems modular.
Prefer correctness before optimization.
Use validation layers aggressively.
Keep the renderer debuggable.
Gradually introduce abstractions as the engine architecture stabilizes.
Vulkan Validation
Validation layers are enabled during development and are treated as an important part of the development workflow.

Vulkan validation errors should generally be treated as real engine bugs rather than warnings to ignore.

The renderer is being developed with particular attention to:

Image layouts
Pipeline stages
Access masks
Synchronization
Semaphore lifetime
Fence lifetime
Command buffer lifetime
Swapchain image lifetime
Resource destruction order
Queue ownership
Swapchain recreation
Synchronization
Orion uses Vulkan Synchronization 2 and vkQueueSubmit2.

Synchronization is intentionally kept explicit.

The renderer tracks synchronization between:

Acquire
   ↓
Image Available Semaphore
   ↓
GPU Submission
   ↓
Rendering / Transfer
   ↓
Image Layout → PRESENT_SRC_KHR
   ↓
Render Finished Semaphore
   ↓
Presentation

As the renderer evolves, synchronization will move toward a proper frame-in-flight architecture with per-frame synchronization resources.

Building
Orion uses CMake.

A typical development build looks like:

cmake -S . -B out/Debug -DCMAKE_BUILD_TYPE=Debug
cmake --build out/Debug

Run the engine with:

./out/Debug/Orion

The exact build configuration may change as the project evolves.

Development
Debug builds should be preferred during renderer development because Vulkan validation provides valuable information about incorrect API usage.

When working on the renderer:

Keep validation layers enabled.
Check Vulkan return values.
Run with validation enabled.
Fix validation errors before adding more rendering functionality.
Verify resource lifetime and destruction order.
Avoid relying on implicit synchronization.
Keep GPU ownership and image layouts explicit.
Roadmap
Core
 Application layer
 Window abstraction
 Logging
 Event system
 Input system
 File system abstraction
 Configuration system
Vulkan
 Vulkan instance
 Validation layers
 Debug messenger
 Surface creation
 Physical device selection
 Logical device
 Graphics queue
 Swapchain
 Command pool
 Command buffer
 Synchronization primitives
 Synchronization 2
 vkQueueSubmit2
 Robust frame synchronization
 Multiple frames in flight
 Swapchain recreation
 Proper resource lifetime management
 Dynamic rendering
 GPU memory allocator
 Buffer abstraction
 Image abstraction
 Descriptor management
 Pipeline abstraction
 Shader system
 Texture system
 Render graph
Rendering
 Triangle
 Mesh rendering
 Vertex/index buffers
 Camera
 Uniform buffers
 Descriptor sets
 Textures
 Materials
 Lighting
 Depth testing
 Multisampling
 HDR
 Post-processing
 PBR
 Shadows
 GPU-driven rendering
Engine
 ECS
 Scene system
 Asset manager
 Serialization
 Resource handles
 Job system
 Threading infrastructure
 Profiling
 Physics
 Audio
Editor
 Editor application
 Scene hierarchy
 Inspector
 Asset browser
 Viewport
 Entity creation
 Transform tools
 Material editor
Contributing
Orion is currently a personal development project.

The architecture is expected to change frequently while the foundational systems are being developed.

If contributing, prefer small and focused changes that preserve the clarity of the engine's core systems.

License
License information will be added as the project matures.
