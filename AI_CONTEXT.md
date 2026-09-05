Orion Engine — AI Development Context
Project Overview
Orion Engine is a C++20 Vulkan game-engine project currently in the early renderer-foundation stage.

The project is NOT yet a complete game engine. The current milestone is:

Build a minimal modern Vulkan renderer capable of drawing the first triangle.

The immediate goal is:

Window
↓
Vulkan Instance
↓
Surface
↓
Physical Device
↓
Logical Device
↓
Swapchain
↓
Shader Modules
↓
Pipeline Layout
↓
Graphics Pipeline
↓
Command Buffer
↓
Dynamic Rendering
↓
vkCmdDraw()
↓
Present

Do not jump ahead into ECS, physics, scripting, editor UI, networking, animation, etc. until the basic renderer is stable.

Current Technology Stack
C++20
CMake
Vulkan
GLFW
vk-bootstrap
GLM
spdlog
Dependencies are currently stored as git submodules under vendor/.

Current Project Structure
Orion-Engine/
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
├── .gitmodules
├── AI_CONTEXT.md
└── README.md

Planned renderer files:

src/Engine/
├── OVulkanPipeline.h
├── OVulkanPipeline.cpp
├── ORenderer.h
└── ORenderer.cpp

Planned shader files:

assets/
└── shaders/
├── triangle.vert
├── triangle.frag
├── triangle.vert.spv
└── triangle.frag.spv

Existing Architecture
Current high-level ownership:

main
│
└── OApplication
│
├── GLFW
├── Window
├── OVulkanContext
│     ├── Vulkan Instance
│     ├── Surface
│     ├── Physical Device
│     ├── Logical Device
│     ├── Graphics Queue
│     └── Swapchain
│
└── OVulkanPipeline
├── Shader Modules
├── Pipeline Layout
└── Graphics Pipeline

Keep OVulkanContext focused on Vulkan context/device/swapchain-level responsibilities.

Do NOT turn OVulkanContext into a giant renderer class.

Current Vulkan Context
OVulkanContext currently handles:

Vulkan instance creation
Validation layers/debug callback
Surface creation
Physical device selection
Logical device creation
Graphics queue acquisition
Swapchain creation
Swapchain image acquisition
Swapchain image-view acquisition
Vulkan resource shutdown
The current initialization sequence is:

CreateInstance()
↓
CreateSurface()
↓
ChoosePhysicalDevice()
↓
CreateDevice()
↓
CreateSwapchain()
↓
CreateImageViews()

Note:

CreateImageViews() currently returns true without doing actual work, while CreateSwapchain() obtains image views using vk-bootstrap:

m_SwapchainImageViews = VKB_Swapchain.get_image_views().value();

This is currently functional but the naming/ownership should eventually be cleaned up.

vk-bootstrap's swapchain image views still need to be explicitly destroyed by Orion.

Vulkan Version
The current code requires Vulkan 1.4:

.require_api_version(1, 4)

and physical-device selection currently requires:

.set_minimum_version(1, 4)

This is intentionally retained for now.

Potential future change:

Required Vulkan:
1.3

Optional:
1.4 features

Do NOT change this during the first graphics-pipeline implementation unless necessary.

Swapchain
Current swapchain configuration requests:

Format:
VK_FORMAT_B8G8R8A8_UNORM

Color space:
VK_COLORSPACE_SRGB_NONLINEAR_KHR

Present mode preference:
VK_PRESENT_MODE_MAILBOX_KHR

The actual selected swapchain format must be queried from vk-bootstrap rather than assumed.

Expose:

VkFormat GetSwapchainImageFormat() const
{
return VKB_Swapchain.image_format;
}

Also expose:

VkExtent2D GetSwapchainExtent() const
{
return VKB_Swapchain.extent;
}

And:

const std::vector<VkImageView>& GetSwapchainImageViews() const
{
return m_SwapchainImageViews;
}

These will be needed by the renderer.

Immediate Milestone
Graphics Pipeline
The immediate task is implementing:

OVulkanPipeline

It should initially own:

VkDevice m_Device;

VkShaderModule m_VertexShader;
VkShaderModule m_FragmentShader;

VkPipelineLayout m_PipelineLayout;

VkPipeline m_Pipeline;

Public API:

class OVulkanPipeline
{
public:
OVulkanPipeline() = default;
~OVulkanPipeline() = default;

    bool Init(
        VkDevice device,
        VkFormat colorFormat
    );

    void Shutdown();

    VkPipeline GetPipeline() const;
    VkPipelineLayout GetPipelineLayout() const;

private:
bool CreateShaderModules();
bool CreatePipelineLayout();
bool CreateGraphicsPipeline(VkFormat colorFormat);

    VkShaderModule CreateShaderModule(
        const std::vector<char>& code
    );
};

Shader Strategy
For the first renderer milestone, use a hard-coded triangle.

Do NOT introduce vertex buffers yet.

The vertex shader uses gl_VertexIndex.

triangle.vert
#version 450

vec2 positions[3] = vec2[](
vec2( 0.0, -0.5),
vec2( 0.5,  0.5),
vec2(-0.5,  0.5)
);

vec3 colors[3] = vec3[](
vec3(1.0, 0.0, 0.0),
vec3(0.0, 1.0, 0.0),
vec3(0.0, 0.0, 1.0)
);

layout(location = 0) out vec3 fragColor;

void main()
{
gl_Position = vec4(
positions[gl_VertexIndex],
0.0,
1.0
);

    fragColor = colors[gl_VertexIndex];
}

triangle.frag
#version 450

layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

void main()
{
outColor = vec4(fragColor, 1.0);
}

Compile to SPIR-V:

glslc assets/shaders/triangle.vert \
-o assets/shaders/triangle.vert.spv

glslc assets/shaders/triangle.frag \
-o assets/shaders/triangle.frag.spv

Initially it is acceptable to commit the generated .spv files.

Later CMake should automate shader compilation.

Graphics Pipeline Configuration
The first graphics pipeline should use:

Shader stages
Vertex:
triangle.vert.spv

Fragment:
triangle.frag.spv

Vertex input
No vertex buffers yet:

vertexBindingDescriptionCount = 0;
vertexAttributeDescriptionCount = 0;

Input assembly
VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST

Rasterization
polygonMode = VK_POLYGON_MODE_FILL
cullMode = VK_CULL_MODE_NONE
frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE
lineWidth = 1.0f

Disable depth/stencil for the first triangle.

Multisampling
VK_SAMPLE_COUNT_1_BIT

Color blending
Initially disabled:

blendEnable = VK_FALSE;

Color write mask:

VK_COLOR_COMPONENT_R_BIT |
VK_COLOR_COMPONENT_G_BIT |
VK_COLOR_COMPONENT_B_BIT |
VK_COLOR_COMPONENT_A_BIT

Dynamic State
Viewport and scissor should be dynamic:

VK_DYNAMIC_STATE_VIEWPORT
VK_DYNAMIC_STATE_SCISSOR

This avoids coupling the pipeline to a specific viewport size.

During command recording:

vkCmdSetViewport(...)
vkCmdSetScissor(...)

Dynamic Rendering
Use modern Vulkan dynamic rendering.

Do NOT introduce VkRenderPass or VkFramebuffer for the first renderer.

Use:

VkPipelineRenderingCreateInfo

with:

colorAttachmentCount = 1;
pColorAttachmentFormats = &colorFormat;
depthAttachmentFormat = VK_FORMAT_UNDEFINED;
stencilAttachmentFormat = VK_FORMAT_UNDEFINED;

The graphics pipeline's pNext should point to the VkPipelineRenderingCreateInfo.

The color format must be the actual swapchain format obtained from:

VKB_Swapchain.image_format

Pipeline Layout
The first pipeline has no descriptors or push constants.

Therefore the initial pipeline layout is empty:

VkPipelineLayoutCreateInfo layoutInfo{};
layoutInfo.sType =
VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

layoutInfo.setLayoutCount = 0;
layoutInfo.pushConstantRangeCount = 0;

Later this will evolve toward:

Pipeline Layout
├── Frame descriptor set
├── Material descriptor set
├── Object descriptor set
└── Push constants

Do not implement those yet.

Pipeline Lifetime
Destruction order must be:

VkPipeline
↓
VkPipelineLayout
↓
Vertex Shader Module
↓
Fragment Shader Module

The graphics pipeline must be destroyed BEFORE VkDevice.

Therefore application shutdown should be:

OVulkanPipeline::Shutdown()
↓
OVulkanPipeline destroyed
↓
OVulkanContext::Shutdown()
↓
VkDevice destroyed

Never destroy the Vulkan device while a pipeline still references it.

Application Ownership
OApplication should own:

std::unique_ptr<OVulkanContext> m_VulkanContext;
std::unique_ptr<OVulkanPipeline> m_VulkanPipeline;

Initialization order:

GLFW
↓
Window
↓
OVulkanContext
↓
OVulkanPipeline

Pipeline initialization:

m_VulkanPipeline =
std::make_unique<OVulkanPipeline>();

if (!m_VulkanPipeline->Init(
m_VulkanContext->GetDevice(),
m_VulkanContext->GetSwapchainImageFormat()))
{
ORION_ERROR(
"Failed to Initialize Graphics Pipeline"
);

    return false;
}

Shutdown:

if (m_VulkanPipeline) {
m_VulkanPipeline->Shutdown();
m_VulkanPipeline.reset();
}

if (m_VulkanContext) {
m_VulkanContext->Shutdown();
m_VulkanContext.reset();
}

Shader Loading
Current first implementation can load SPIR-V at runtime using a helper:

std::vector<char> ReadBinaryFile(
const std::string& filename
);

The shader paths can initially be:

assets/shaders/triangle.vert.spv
assets/shaders/triangle.frag.spv

This is acceptable for the first milestone.

However, relative paths are fragile.

Eventually shader paths should be resolved relative to an asset root or executable/project root.

Do not over-engineer this yet.

Current Renderer Roadmap
Implement in this order.

Phase 1 — Graphics Pipeline
[ ] Add OVulkanPipeline.h
[ ] Add OVulkanPipeline.cpp
[ ] Add shader loading
[ ] Add triangle.vert
[ ] Add triangle.frag
[ ] Compile shaders to SPIR-V
[ ] Create shader modules
[ ] Create pipeline layout
[ ] Create graphics pipeline
[ ] Destroy pipeline correctly

Phase 2 — Frame Infrastructure
[ ] Command pool
[ ] Command buffers
[ ] Fences
[ ] Image-available semaphores
[ ] Render-finished semaphores
[ ] Per-frame resources

Recommended initial frame count:

constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

Phase 3 — Actual Rendering
Command sequence:

Acquire swapchain image
↓
Begin command buffer
↓
Transition image:
PRESENT_SRC_KHR
↓
COLOR_ATTACHMENT_OPTIMAL
↓
vkCmdBeginRendering()
↓
vkCmdBindPipeline()
↓
vkCmdSetViewport()
↓
vkCmdSetScissor()
↓
vkCmdDraw(3, 1, 0, 0)
↓
vkCmdEndRendering()
↓
Transition image:
COLOR_ATTACHMENT_OPTIMAL
↓
PRESENT_SRC_KHR
↓
End command buffer
↓
Submit
↓
Present

Phase 4 — Swapchain Recreation
Eventually:

Window resize
↓
Wait for non-zero framebuffer dimensions
↓
Wait for device idle
↓
Destroy swapchain-dependent resources
↓
Recreate swapchain
↓
Recreate image views
↓
Recreate graphics pipeline if format changed

Current window is non-resizable, so this can wait until after the first triangle.

Phase 5 — Real Geometry
After the hard-coded triangle works:

[ ] Vertex buffer
[ ] Index buffer
[ ] Vertex structure
[ ] Vertex input bindings
[ ] Vertex attributes
[ ] Mesh abstraction

Phase 6 — GPU Resources
[ ] Buffer abstraction
[ ] Image abstraction
[ ] ImageView abstraction
[ ] Sampler
[ ] Texture
[ ] GPU memory allocator

Phase 7 — Renderer Architecture
Eventually move toward:

ORenderer
│
├── OVulkanContext
├── OVulkanPipeline
├── Frame resources
├── Command pools
├── Synchronization
├── Resource management
└── Rendering interface

Longer-term architecture:

Orion Engine
│
├── Core
│   ├── Application
│   ├── Log
│   ├── Assert
│   ├── Time
│   └── Types
│
├── Platform
│   └── Window
│
├── Renderer
│   ├── Renderer
│   ├── RenderDevice
│   ├── Pipeline
│   ├── Buffer
│   ├── Image
│   ├── Texture
│   ├── Shader
│   └── CommandBuffer
│
├── Vulkan
│   ├── VulkanInstance
│   ├── VulkanDevice
│   ├── VulkanSwapchain
│   ├── VulkanPipeline
│   ├── VulkanBuffer
│   └── VulkanImage
│
├── Scene
├── Assets
├── Input
└── Math

Do NOT implement this entire architecture now.

Grow toward it incrementally.

Important Current Issues / Technical Debt
1. CMake minimum version
   Current CMake requires a very recent CMake version.

Review whether:

cmake_minimum_required(VERSION 4.4)

is actually necessary.

A lower supported version would improve portability.

Do not change blindly; verify dependency requirements first.

2. C++ standard discrepancy
   README currently describes C++17+, but CMake requires:

CMAKE_CXX_STANDARD 20

The actual project requirement should eventually be documented consistently as C++20.

3. GLOB_RECURSE
   Current source discovery uses:

file(GLOB_RECURSE ...)

If retained, prefer:

file(GLOB_RECURSE ORION_SOURCES
CONFIGURE_DEPENDS
...
)

Eventually explicit source lists or per-directory CMake files may be preferable.

4. .gitmodules
   There is an unusual/possibly stale submodule entry under:

build/vendor/spdlog

build/ should normally contain generated build artifacts and should not contain project dependencies.

Investigate and remove the stale entry if confirmed unnecessary.

5. Initialization error handling
   Current OApplication construction calls Init() internally.

This means initialization failure does not currently propagate cleanly through object construction.

Current conceptual flow:

OApplication()
↓
Init()
↓
failure
↓
object still exists

Eventually change toward one of:

factory/result-based initialization

or:

OApplication::OApplication()
{
if (!Init())
throw std::runtime_error(...);
}

Do not prioritize this over getting the first triangle rendered.

6. Vulkan handles
   Use:

VK_NULL_HANDLE

for Vulkan handle invalidation rather than:

nullptr

Example:

m_Instance = VK_NULL_HANDLE;

Design Principles
Keep these principles while developing Orion.

1. Don't abstract Vulkan too early
   First understand the Vulkan concepts directly.

Then wrap repeated patterns.

2. Keep ownership obvious
   Every Vulkan resource should have one clear owner.

3. Destroy in reverse dependency order
   Example:

Pipeline
↓
Pipeline Layout
↓
Device

4. Prefer RAII eventually
   The current explicit Init() / Shutdown() style is acceptable during the early learning phase.

As the engine grows, consider RAII wrappers.

5. Keep OVulkanContext small
   It should not become:

everything Vulkan

6. Don't build features ahead of the renderer
   The next meaningful milestone is the triangle.

7. Use validation layers aggressively
   Any Vulkan validation error should be treated as important.

Do not suppress validation errors just to get the triangle running.

Immediate Task For Next Development Session
Continue from:

Implementing OVulkanPipeline and creating the first Vulkan graphics pipeline.

First verify:

[ ] OVulkanContext exposes VkDevice
[ ] OVulkanContext exposes swapchain format
[ ] OVulkanContext exposes swapchain extent
[ ] OVulkanContext exposes swapchain image views
[ ] OVulkanPipeline exists
[ ] Triangle shaders exist
[ ] SPIR-V shaders compile
[ ] Shader modules are created
[ ] Pipeline layout is created
[ ] Graphics pipeline is created
[ ] Pipeline is destroyed before VkDevice

At this stage, do not expect anything to appear on screen yet.

The next major task after pipeline creation is:

Build command pools, command buffers, synchronization, dynamic rendering, and finally issue vkCmdDraw(3, 1, 0, 0).

Expected first visual milestone:

┌─────────────────────────────┐
│                             │
│            🔺               │
│         RGB TRIANGLE        │
│                             │
└─────────────────────────────┘

The first triangle should be implemented without a vertex buffer using gl_VertexIndex.

After the triangle works, introduce vertex/index buffers and begin building the actual renderer abstraction.

Current Mental Model
The most important architecture to preserve is:

                 OApplication
                      │
                      ▼
              OVulkanContext
                      │
          ┌───────────┴───────────┐
          │                       │
       VkDevice               Swapchain
          │                       │
          └───────────┬───────────┘
                      │
                      ▼
              OVulkanPipeline
                      │
             ┌────────┴────────┐
             │                 │
        Vertex Shader     Fragment Shader
             │                 │
             └────────┬────────┘
                      │
                 VkPipeline
                      │
                      ▼
                ORenderer
                      │
          ┌───────────┼───────────┐
          │           │           │
      Commands      Frames    Synchronization
          │           │           │
          └───────────┼───────────┘
                      ▼
               Dynamic Rendering
                      │
                      ▼
                 vkCmdDraw()
                      │
                      ▼
                   Present