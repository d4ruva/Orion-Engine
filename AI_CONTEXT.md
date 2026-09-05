
AI_CONTEXT.md

```md
# Orion Engine — AI Context

## Project Overview

Orion Engine is a C++ game engine being developed from the ground up.

The project is currently focused heavily on Vulkan rendering and low-level engine architecture.

The primary purpose of the project is to understand engine architecture and modern graphics programming deeply while gradually building reusable engine systems.

This document provides context for AI-assisted development.

AI assistants should read this file before proposing architectural changes or modifying core systems.

---

# Core Philosophy

Orion is a learning-driven engine project.

The goal is not to hide complexity immediately.

The goal is to understand the complexity first and then introduce abstractions that make the system easier to use without hiding important behavior.

Priorities are:

1. Correctness
2. Understanding
3. Debuggability
4. Clear ownership
5. Maintainability
6. Performance
7. Abstraction

Do not optimize prematurely.

Do not introduce large abstractions simply because they are common in other engines.

Prefer small abstractions that are justified by repeated usage or clear ownership boundaries.

---

# Current Graphics API

The renderer uses Vulkan.

The project targets Vulkan 1.3 and currently relies on modern Vulkan functionality.

Important Vulkan features include:

- Vulkan 1.3
- Synchronization 2
- Dynamic Rendering
- `vkQueueSubmit2`
- Vulkan validation layers

The renderer should continue moving toward modern Vulkan APIs instead of introducing legacy synchronization APIs unless there is a specific reason.

---

# Vulkan Initialization

The current initialization flow is approximately:

```text
GLFW Window
    ↓
VkInstance
    ↓
VkSurfaceKHR
    ↓
Physical Device
    ↓
Logical Device
    ↓
Graphics Queue
    ↓
Swapchain
    ↓
Swapchain Images
    ↓
Command Pool
    ↓
Command Buffers
    ↓
Synchronization Objects

vk-bootstrap is currently used to simplify:

Vulkan instance creation
Validation layer configuration
Debug callback setup
Physical device selection
Logical device creation
Swapchain creation
Do not replace vk-bootstrap without a strong architectural reason.

Vulkan Features
The physical device selection currently requires Vulkan 1.3 features including:

VkPhysicalDeviceVulkan13Features{
    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
    .synchronization2 = VK_TRUE,
    .dynamicRendering = VK_TRUE,
};

These features are intentional.

If new Vulkan functionality is introduced, verify:

The feature is supported.
The feature is enabled during device creation.
The correct Vulkan structures are chained.
The required extension is available if the feature is extension-based.
Validation remains clean.
Never assume a Vulkan feature is available simply because the physical device supports Vulkan 1.3.

Renderer Philosophy
The renderer should remain explicit.

Important concepts should not be hidden behind overly generic APIs.

In particular, maintain clear handling of:

VkImage
VkImageView
VkBuffer
VkDeviceMemory
VkCommandBuffer
VkSemaphore
VkFence
VkPipeline
VkPipelineLayout
VkDescriptorSet
VkSwapchainKHR
Resource lifetime is a first-class architectural concern.

Image Layouts
Image layouts are explicit and must be tracked correctly.

Swapchain images commonly transition through:

PRESENT_SRC_KHR
        ↓
GENERAL / TRANSFER_DST_OPTIMAL
        ↓
PRESENT_SRC_KHR

The exact intermediate layout should match the command being performed.

For example, vkCmdClearColorImage requires the image to be in an appropriate layout such as:

VK_IMAGE_LAYOUT_GENERAL

or:

VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL

depending on the intended usage.

Do not arbitrarily use image layouts.

Image Barriers
The project uses Synchronization 2.

The preferred barrier structure is:

VkImageMemoryBarrier2
VkDependencyInfo
vkCmdPipelineBarrier2(...)

The project should prefer VK_PIPELINE_STAGE_2_* and VK_ACCESS_2_* flags.

Do not regress to vkCmdPipelineBarrier unless there is a specific compatibility reason.

Synchronization 2
vkQueueSubmit2 is the preferred submission API.

A modern submission should conceptually look like:

VkSemaphoreSubmitInfo
        ↓
VkCommandBufferSubmitInfo
        ↓
VkSubmitInfo2
        ↓
vkQueueSubmit2

Avoid mixing old VkSubmitInfo concepts into the renderer unless necessary.

Semaphore Semantics
Binary semaphore lifetime is extremely important.

The renderer currently uses:

Image-available semaphore
Render-finished semaphore
In-flight fence
A major Vulkan rule:

A binary semaphore cannot simply be reused because the CPU has submitted the previous frame.

Presentation can continue using the semaphore after vkQueuePresentKHR returns.

Therefore, swapchain semaphore reuse must be designed deliberately.

Preferred long-term architecture:

Frame 0:
    imageAvailable[0]
    renderFinished[0]
    inFlightFence[0]

Frame 1:
    imageAvailable[1]
    renderFinished[1]
    inFlightFence[1]

...

However, presentation-related semaphore reuse has additional subtleties.

A robust swapchain synchronization design should account for the fact that presentation completion is not necessarily represented by the normal graphics submission fence.

Do not assume:

vkWaitForFences(...)

automatically means a presentation semaphore is safe to reuse.

Frames In Flight
The renderer should eventually support multiple frames in flight.

A typical architecture should look like:

MAX_FRAMES_IN_FLIGHT = 2

FrameData
{
    VkSemaphore imageAvailable;
    VkSemaphore renderFinished;
    VkFence renderFence;

    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;

    // Future:
    descriptor allocator
    transient allocator
    deletion queue
}

The current single-frame implementation is acceptable for early development but should eventually evolve into this structure.

Swapchain Images
Swapchain images are owned by the swapchain.

The renderer must not destroy swapchain images manually.

The renderer may destroy associated image views.

Typical lifetime:

Create Swapchain
    ↓
Get Swapchain Images
    ↓
Create / retrieve Image Views
    ↓
Use images
    ↓
Wait for GPU/presentation usage to finish
    ↓
Destroy image views
    ↓
Destroy swapchain

When using vk-bootstrap, follow its ownership expectations carefully.

Swapchain Destruction
The swapchain must not be destroyed while any of its presentable images are still in use.

Before destroying the swapchain:

vkDeviceWaitIdle(device);

is acceptable during early development and during shutdown/recreation.

Later, a more sophisticated synchronization mechanism may be introduced.

The important invariant is:

No GPU or presentation operation may still reference the swapchain
when vkDestroySwapchainKHR is called.

Resource Destruction Order
Vulkan resources must be destroyed in reverse dependency order.

A simplified device-level destruction order is:

GPU work completed
    ↓
Graphics pipelines
    ↓
Pipeline layouts
    ↓
Render passes / dynamic rendering resources
    ↓
Shader modules
    ↓
Descriptor resources
    ↓
Frame synchronization objects
    ↓
Command buffers
    ↓
Command pools
    ↓
Swapchain image views
    ↓
Swapchain
    ↓
Logical device
    ↓
Surface
    ↓
Instance

The exact order depends on which resources exist.

The core rule is:

Destroy children before parents.

Examples:

Pipeline before pipeline layout
Command buffers before command pool
Image views before their images/swapchain
Swapchain before device
Device before instance-dependent device resources
Shutdown
Shutdown should first ensure that the GPU is finished:

vkDeviceWaitIdle(m_Device);

Then resources should be destroyed in dependency order.

Do not destroy:

Swapchain
Device
Instance

while work may still be executing against them.

Error Handling
Vulkan return values should be checked.

For functions returning VkResult, prefer:

VkResult result = ...;

if (result != VK_SUCCESS)
{
    ...
}

For functions that legitimately return:

VK_SUCCESS
VK_SUBOPTIMAL_KHR
VK_ERROR_OUT_OF_DATE_KHR

handle those cases explicitly.

Do not blindly treat every non-VK_SUCCESS value as fatal.

Validation Layers
Validation layers should remain enabled during development.

Validation messages should be considered important debugging information.

If validation reports:

current layout is VK_IMAGE_LAYOUT_UNDEFINED

do not suppress the message.

Find the missing state transition or incorrect state tracking.

If validation reports:

semaphore may still be in use by VkSwapchainKHR

treat this as a synchronization/lifetime bug.

Logging
The project uses a custom logging system with macros such as:

ORION_TRACE(...)
ORION_INFO(...)
ORION_WARN(...)
ORION_ERROR(...)

Use the existing logging system instead of introducing another logging library.

Vulkan validation messages are routed through the Vulkan debug callback.

Naming
Existing naming generally follows the project's C++ style.

Examples:

OVulkanContext
CreateInstance()
CreateSurface()
ChoosePhysicalDevice()
CreateDevice()
CreateSwapchain()
CreateCommandPool()
CreateCommandBuffer()
CreateSyncObjects()
RenderFrame()
Shutdown()

Maintain consistency with the existing naming conventions.

Do not rename large portions of the project merely for stylistic reasons.

Vulkan Context
OVulkanContext currently owns major Vulkan initialization and rendering state.

It currently manages concepts such as:

VkInstance
VkSurfaceKHR
VkPhysicalDevice
VkDevice
VkSwapchainKHR
VkQueue
VkCommandPool
VkCommandBuffer
VkSemaphore
VkFence

As the engine grows, this class may become too large.

Possible future separation:

OVulkanContext
OVulkanDevice
OVulkanSwapchain
OVulkanCommandContext
OVulkanFrameData
OVulkanAllocator
OVulkanPipeline
OVulkanDescriptorAllocator

Do not split these prematurely.

Refactor when ownership boundaries become clear.

Rendering Roadmap
The likely progression is:

Vulkan Initialization
        ↓
Swapchain
        ↓
Command Submission
        ↓
Clear Screen
        ↓
Triangle
        ↓
Vertex / Index Buffers
        ↓
Uniform Buffers
        ↓
Descriptors
        ↓
Textures
        ↓
Materials
        ↓
Meshes
        ↓
Camera
        ↓
Depth
        ↓
Lighting
        ↓
PBR
        ↓
Render Graph
        ↓
Advanced GPU-driven rendering

Each step should be stable before introducing unnecessary complexity.

Dynamic Rendering
Dynamic rendering is enabled at device creation.

The long-term renderer should prefer:

vkCmdBeginRendering(...)
vkCmdEndRendering(...)

over building large collections of fixed render passes where appropriate.

Do not maintain a render-pass abstraction merely because traditional Vulkan tutorials use it.

If a render pass is required by a specific subsystem, keep it isolated.

Render Graph
A render graph is a possible future architecture.

Do not introduce a render graph before the renderer has enough passes and resources to justify it.

A future render graph should potentially track:

Resource creation
Resource lifetime
Image layouts
Pipeline stages
Access masks
Pass dependencies
Barriers
Transient resources
Execution order
The render graph should generate synchronization rather than forcing every rendering pass to manually manage all barriers.

Memory Management
A dedicated GPU memory abstraction will eventually be required.

Potential future responsibilities:

Buffer allocation
Image allocation
Dedicated allocations
Suballocation
Staging buffers
Upload operations
Resource lifetime
Do not create a custom allocator before understanding the allocation patterns required by the renderer.

A library such as Vulkan Memory Allocator may eventually be considered.

Command Submission
The renderer should eventually centralize command submission.

Potential architecture:

BeginFrame()
    ↓
AcquireSwapchainImage()
    ↓
RecordCommands()
    ↓
Submit()
    ↓
Present()
    ↓
EndFrame()

The frame system should own synchronization details.

Rendering systems should ideally not need to manually understand semaphore ownership.

Current Swapchain Rendering Model
The current basic rendering path is approximately:

Wait for frame fence
        ↓
Reset fence
        ↓
Acquire swapchain image
        ↓
Reset command buffer
        ↓
Begin command buffer
        ↓
Transition image
        ↓
Clear image
        ↓
Transition image to PRESENT_SRC_KHR
        ↓
End command buffer
        ↓
vkQueueSubmit2
        ↓
vkQueuePresentKHR

This is intentionally simple.

It is a foundation for more advanced rendering.

Important Vulkan Invariants
AI-generated code must preserve these invariants.

Image Layout
If a command accesses an image, the image must be in a layout valid for that command.

Semaphore
A binary semaphore must not be signaled while it is still signaled or still in use.

Fence
A fence must not be reset unless the associated submission has completed or the program otherwise knows the fence can safely be reused.

Command Buffer
A command buffer must not be reset or recorded while it is still pending execution.

Command Pool
Command buffers must be freed before destroying their command pool.

Swapchain
A swapchain must not be destroyed while its images are still being used.

Device
Device-owned objects must be destroyed before destroying the device.

AI Development Rules
When modifying this project:

Read the existing implementation before proposing a rewrite.
Preserve existing architecture unless there is a concrete problem.
Prefer incremental changes.
Keep Vulkan validation enabled.
Explain synchronization changes.
Explain ownership changes.
Check every relevant VkResult.
Do not introduce deprecated Vulkan APIs unnecessarily.
Prefer Vulkan 1.3 functionality where supported.
Prefer Synchronization 2.
Prefer vkQueueSubmit2.
Do not silently change image layouts.
Do not silently change queue usage.
Do not silently change resource ownership.
Do not destroy resources in arbitrary order.
Do not hide synchronization behind unexplained abstractions.
Avoid premature optimization.
Avoid unnecessary third-party dependencies.
Keep changes focused.
Compile mentally against the existing types and ownership model before suggesting code.
When Suggesting Vulkan Code
Always consider:

Who owns this resource?
When is it created?
When is it first used?
Which queue uses it?
Which pipeline stage uses it?
Which access type is involved?
What image layout is required?
What synchronization makes it safe?
When can it be destroyed?

For image barriers specifically, consider:

oldLayout
newLayout
srcStageMask
srcAccessMask
dstStageMask
dstAccessMask
aspectMask
mip levels
array layers
queue family ownership

Do not use:

VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT
VK_ACCESS_2_MEMORY_READ_BIT
VK_ACCESS_2_MEMORY_WRITE_BIT

as a generic solution everywhere.

They are useful for debugging or simple early implementations, but production synchronization should eventually describe the actual dependency as precisely as practical.

Current Known Architectural Direction
The renderer is moving toward:

Modern Vulkan
    +
Synchronization 2
    +
vkQueueSubmit2
    +
Dynamic Rendering
    +
Explicit resource lifetime
    +
Frame-based synchronization
    +
Dedicated resource abstractions
    +
Eventually a render graph

The goal is a renderer that is explicit internally but pleasant to use from higher-level engine systems.

What Not To Do
Do not:

Disable validation to make errors disappear.
Add vkDeviceWaitIdle() after every operation as a permanent synchronization strategy.
Ignore swapchain synchronization errors.
Reuse binary semaphores without understanding their lifetime.
Assume vkQueuePresentKHR() means presentation has completed.
Destroy Vulkan objects in creation order.
Use VK_IMAGE_LAYOUT_UNDEFINED as a generic current layout.
Add barriers without understanding their dependency.
Use ALL_COMMANDS for every synchronization operation in the final renderer.
Add a render graph before the renderer needs one.
Build a huge renderer abstraction around one Vulkan call.
Copy an entire architecture from another engine without understanding why it exists.
Optimize before profiling.
Treat validation warnings as harmless.
Expected AI Response Style
When helping with Orion:

Be technically precise.
Prefer concrete Vulkan explanations.
Explain why a Vulkan rule exists.
Point out lifetime and synchronization bugs directly.
Show minimal fixes before suggesting architectural refactors.
Distinguish immediate fixes from long-term architecture.
Call out hidden assumptions.
Mention relevant Vulkan invariants.
Avoid unnecessary abstraction.
If code is incorrect, say exactly why.
If there are multiple valid approaches, explain the tradeoffs.

