#include "OVulkanContext.h"
#include "GLFW/glfw3.h"
#include "OLog.h"
#include "VkBootstrap.h"
#include <vulkan/vulkan_core.h>

namespace Orion {
static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {
    switch (messageSeverity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            ORION_TRACE("[Vulkan] {}", pCallbackData->pMessage);
            break;

        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            ORION_INFO("[Vulkan] {}", pCallbackData->pMessage);
            break;

        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            ORION_WARN("[Vulkan] {}", pCallbackData->pMessage);
            break;

        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            ORION_ERROR("[Vulkan] {}", pCallbackData->pMessage);
            break;

        default:
            ORION_INFO("[Vulkan] {}", pCallbackData->pMessage);
            break;
    }

    return VK_FALSE;
}

bool OVulkanContext::Init(GLFWwindow* window) {
    m_Window = window;

    if (!CreateInstance()) return false;
    if (!CreateSurface()) return false;
    if (!ChoosePhysicalDevice()) return false;
    if (!CreateDevice()) return false;
    if (!CreateSwapchain()) return false;
    if (!CreateImageViews()) return false;

    if (!CreateCommandPool()) return false;
    if (!CreateCommandBuffer()) return false;
    if (!CreateSyncObjects()) return false;

    return true;
}
bool OVulkanContext::CreateInstance() {
    auto inst_ret = vkb::InstanceBuilder()
                        .set_app_name("Orion Engine")
                        .request_validation_layers()
                        .set_debug_callback(VulkanDebugCallback)
                        .require_api_version(1, 3)
                        .build();

    if (!inst_ret) {
        ORION_ERROR("Failed to create Vulkan Instance");
        return false;
    }

    VKB_Instance = inst_ret.value();

    m_Instance = VKB_Instance.instance;

    ORION_INFO("Vulkan Isntance Created Successfully");

    return true;
}

bool OVulkanContext::CreateSurface() {
    VkResult result = glfwCreateWindowSurface(m_Instance, m_Window, nullptr, &m_Surface);

    if (result != VK_SUCCESS) {
        ORION_ERROR("Failed to create VUlkan Surfaces: {}", static_cast<int>(result));

        return false;
    }

    ORION_INFO("Vulkan Surface Created");

    return true;
}

bool OVulkanContext::ChoosePhysicalDevice() {
    vkb::PhysicalDeviceSelector selector{ VKB_Instance };

	VkPhysicalDeviceVulkan13Features vk13Features{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
		.synchronization2 = VK_TRUE,

		.dynamicRendering = VK_TRUE,
	};

	selector.set_required_features_13(vk13Features);

    auto phys_dev_ret = selector.set_minimum_version(1, 3).set_surface(m_Surface).select();

    if (!phys_dev_ret) {
        ORION_ERROR("Failed to select Physical Device: {}", phys_dev_ret.error().message());
        return false;
    }

    VKB_PhysicalDevice = phys_dev_ret.value();

    m_PhysicalDevice = VKB_PhysicalDevice.physical_device;

    ORION_INFO("Physical Device Selected: {}", VKB_PhysicalDevice.properties.deviceName);

    return true;
}

bool OVulkanContext::CreateDevice() {
    vkb::DeviceBuilder device_builder{ VKB_PhysicalDevice };

    auto device_ret = device_builder.build();
    if (!device_ret) {
        ORION_ERROR("Failed to create Vulkan Logical Device: {}", device_ret.error().message());
        return false;
    }

    VKB_Device = device_ret.value();
    m_Device = VKB_Device.device;

    ORION_INFO("Created Logical Vulkan Device Successfully");

    auto graphics_queue_ret = VKB_Device.get_queue(vkb::QueueType::graphics);

    if (!graphics_queue_ret) {
        ORION_ERROR("Failed to get Graphics Queue: ", graphics_queue_ret.error().message());
        return false;
    }

    graphicsQueue = graphics_queue_ret.value();

    auto graphics_queue_index_ret = VKB_Device.get_queue_index(vkb::QueueType::graphics);

    if (!graphics_queue_index_ret) {
        ORION_ERROR("Failed to get graphics Queue Index: {}", graphics_queue_index_ret.error().message());
        return false;
    }

    graphicsQueueIndex = graphics_queue_index_ret.value();

    ORION_INFO("Choose Graphics Queue and Index Successfully");

    return true;
}

bool OVulkanContext::CreateSwapchain() {
    vkb::SwapchainBuilder swapchainBuilder{ VKB_Device };

	swapchainBuilder.add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT);

    int width, height;

    glfwGetWindowSize(m_Window, &width, &height);

    auto swap_ret = swapchainBuilder.set_desired_format({ VK_FORMAT_B8G8R8A8_UNORM, VK_COLORSPACE_SRGB_NONLINEAR_KHR })
                        .set_desired_present_mode(VK_PRESENT_MODE_MAILBOX_KHR)
                        .set_desired_extent(width, height)
                        .build();

    if (!swap_ret) {
        ORION_ERROR("Failed to create Swapchain: {}", swap_ret.error().message());
        return false;
    }

    VKB_Swapchain = swap_ret.value();

    m_Swapchain = VKB_Swapchain.swapchain;
    ORION_INFO("Swapchain Created Successfully");

    m_SwapchainImages = VKB_Swapchain.get_images().value();
    m_SwapchainImageViews = VKB_Swapchain.get_image_views().value();

	m_SwapchainImageLayouts.resize(m_SwapchainImages.size(), VK_IMAGE_LAYOUT_UNDEFINED);

    return true;
}

bool OVulkanContext::CreateImageViews() { return true; }

bool OVulkanContext::CreateCommandPool() {
    VkCommandPoolCreateInfo poolInfo = { .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = graphicsQueueIndex };

    if (vkCreateCommandPool(m_Device, &poolInfo, nullptr, &m_CommandPool) != VK_SUCCESS) {
        ORION_ERROR("Failed to Create Command Pool");
        return false;
    }

    ORION_INFO("Created Command Pool Successfully");

    return true;
}

bool OVulkanContext::CreateCommandBuffer() {
    VkCommandBufferAllocateInfo allocInfo = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = m_CommandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1 };

    if (vkAllocateCommandBuffers(m_Device, &allocInfo, &m_CommandBuffer) != VK_SUCCESS) {
        ORION_ERROR("Failed to Create Command Buffer");
        return false;
    }

    ORION_INFO("Created Command Buffer Successfully");

    return true;
}

bool OVulkanContext::CreateSyncObjects() {
    VkSemaphoreCreateInfo semaphoreInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };

    VkFenceCreateInfo fenceInfo = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };

    if (vkCreateSemaphore(m_Device, &semaphoreInfo, nullptr, &m_ImageAvailableSemaphore) != VK_SUCCESS) {
        ORION_ERROR("Failed to Create image Available Semaphore");
        return false;
    }

    ORION_INFO("Created Image Available Semaphore Successfully");

	m_RenderFinishedSemaphores.resize(m_SwapchainImages.size());

	for(auto& semaphore: m_RenderFinishedSemaphores){
    if (vkCreateSemaphore(m_Device, &semaphoreInfo, nullptr, &semaphore) != VK_SUCCESS) {
        ORION_ERROR("Failed to Create Render Finished Semaphore");
        return false;
    }
	}

    ORION_INFO("Created Render Finished Semaphore Successfully");

    if (vkCreateFence(m_Device, &fenceInfo, nullptr, &m_InFlightFence) != VK_SUCCESS) {
        ORION_ERROR("Failed to create in flight fences");
        return false;
    }

    ORION_INFO("Created Sync Objects Successfully");

    return true;
}

void OVulkanContext::TransitionImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout) {
	VkImageMemoryBarrier2 imageBarrier {.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2};
    imageBarrier.pNext = nullptr;

    imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    imageBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
    imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    imageBarrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;

    imageBarrier.oldLayout = oldLayout;
    imageBarrier.newLayout = newLayout;

    imageBarrier.subresourceRange = {
		.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.baseMipLevel = 0,
		.levelCount = 1,
		.baseArrayLayer = 0,
		.layerCount = 1,
	};
    imageBarrier.image = image;

    VkDependencyInfo depInfo {};
    depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    depInfo.pNext = nullptr;

	// Nothing needs to be synchronized from an UNDEFINED layout.
    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED) {
        imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
        imageBarrier.srcAccessMask = VK_ACCESS_2_NONE;
    }

    // Presentation doesn't perform memory accesses that need
    // to be made visible through dstAccessMask.
    if (newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
        imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_NONE;
        imageBarrier.dstAccessMask = VK_ACCESS_2_NONE;
    }

    depInfo.imageMemoryBarrierCount = 1;
    depInfo.pImageMemoryBarriers = &imageBarrier;

    vkCmdPipelineBarrier2(commandBuffer, &depInfo);
}

void OVulkanContext::RenderFrame() {
    // Wait for previous frame to finish.
    vkWaitForFences(m_Device, 1, &m_InFlightFence, VK_TRUE, UINT64_MAX);

    vkResetFences(m_Device, 1, &m_InFlightFence);


    // Get the next swapchain image.
    uint32_t imageIndex = 0;

    VkResult result =
        vkAcquireNextImageKHR(m_Device, m_Swapchain, UINT64_MAX, m_ImageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        ORION_ERROR("Failed to acquire swapchain image: {}", static_cast<int>(result));

        return;
    }

	VkSemaphore renderFinishedSemaphore = m_RenderFinishedSemaphores[imageIndex];


    // Reset command buffer.
    vkResetCommandBuffer(m_CommandBuffer, 0);


    // Begin recording.
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    result = vkBeginCommandBuffer(m_CommandBuffer, &beginInfo);

    if (result != VK_SUCCESS) {
        ORION_ERROR("Failed to begin command buffer");
        return;
    }


    VkImage swapchainImage = m_SwapchainImages[imageIndex];


	VkImageLayout oldLayout = m_SwapchainImageLayouts[imageIndex];
    // PRESENT -> TRANSFER_DST
    TransitionImage(m_CommandBuffer, swapchainImage, oldLayout, VK_IMAGE_LAYOUT_GENERAL);

	m_SwapchainImageLayouts[imageIndex] = VK_IMAGE_LAYOUT_GENERAL;

    // Blue!
    VkClearColorValue clearColor{};

    clearColor.float32[0] = 1.0f;
    clearColor.float32[1] = 0.0f;
    clearColor.float32[2] = 0.0f;
    clearColor.float32[3] = 1.0f;


    VkImageSubresourceRange range{};

    range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    range.baseMipLevel = 0;
    range.levelCount = 1;

    range.baseArrayLayer = 0;
    range.layerCount = 1;


	vkCmdClearColorImage(m_CommandBuffer, swapchainImage, VK_IMAGE_LAYOUT_GENERAL, &clearColor, 1, &range);

    TransitionImage(m_CommandBuffer, swapchainImage, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

	m_SwapchainImageLayouts[imageIndex] = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;


    // TRANSFER_DST -> PRESENT


    result = vkEndCommandBuffer(m_CommandBuffer);

    if (result != VK_SUCCESS) {
        ORION_ERROR("Failed to end command buffer");
        return;
    }


    // Submit
	//
	VkSemaphoreSubmitInfo waitSemaphoreInfo{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
		.semaphore = m_ImageAvailableSemaphore,
		.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
	};

	VkSemaphoreSubmitInfo signalSemaphoreInfo{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
		.semaphore = renderFinishedSemaphore,
		.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
	};

	VkCommandBufferSubmitInfo cmdSubmitInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
		.commandBuffer = m_CommandBuffer
	};

	VkSubmitInfo2 submitInfo = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,

		.waitSemaphoreInfoCount = 1,
		.pWaitSemaphoreInfos = &waitSemaphoreInfo,

		.commandBufferInfoCount = 1,
		.pCommandBufferInfos = &cmdSubmitInfo,

		.signalSemaphoreInfoCount = 1,
		.pSignalSemaphoreInfos = &signalSemaphoreInfo,
	};

	result = vkQueueSubmit2(graphicsQueue, 1, &submitInfo, m_InFlightFence);

    if (result != VK_SUCCESS) {
        ORION_ERROR("Failed to Submit Command Buffer");
        return;
    }



    // Present.
    VkPresentInfoKHR presentInfo{};

    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;

    presentInfo.pWaitSemaphores = &renderFinishedSemaphore;

    presentInfo.swapchainCount = 1;

    presentInfo.pSwapchains = &m_Swapchain;

    presentInfo.pImageIndices = &imageIndex;


    result = vkQueuePresentKHR(graphicsQueue, &presentInfo);

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        ORION_ERROR("Failed to present swapchain image: {}", static_cast<int>(result));
    }
}

void OVulkanContext::Shutdown() {

	vkDeviceWaitIdle(m_Device);

	if(m_ImageAvailableSemaphore != VK_NULL_HANDLE)
	{
		vkDestroySemaphore(m_Device, m_ImageAvailableSemaphore, nullptr);
	}

	if(!m_RenderFinishedSemaphores.empty()){
		for(auto& semaphore: m_RenderFinishedSemaphores)
		{
			vkDestroySemaphore(m_Device, semaphore, nullptr);
		}
	}

	if(m_InFlightFence != VK_NULL_HANDLE)
	{
		vkDestroyFence(m_Device, m_InFlightFence, nullptr);
	}

	if(m_CommandBuffer != VK_NULL_HANDLE)
	{
		vkFreeCommandBuffers(m_Device, m_CommandPool, 1, &m_CommandBuffer);
	}

	if(m_CommandPool != VK_NULL_HANDLE)
	{
		vkDestroyCommandPool(m_Device, m_CommandPool, nullptr);
	}

    if (!m_SwapchainImageViews.empty()) {
        for (auto imageView : m_SwapchainImageViews) {
            vkDestroyImageView(m_Device, imageView, nullptr);
        }
    }

    if (m_Swapchain != VK_NULL_HANDLE) {
        vkb::destroy_swapchain(VKB_Swapchain);
    }

    if (m_Device != VK_NULL_HANDLE) {
        vkb::destroy_device(VKB_Device);
    }

    if (m_Surface != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
    }
    if (m_Instance != VK_NULL_HANDLE) {
        vkb::destroy_instance(VKB_Instance);

        m_Instance = nullptr;
    }
}
} // namespace Orion
