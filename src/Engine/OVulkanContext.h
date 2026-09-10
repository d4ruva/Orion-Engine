#pragma once


#include <vector>
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "VkBootstrap.h"
#include <vulkan/vulkan_core.h>


namespace Orion {
class OVulkanPipeline;
class OVulkanContext {

    public:
    OVulkanContext() = default;
    ~OVulkanContext() = default;

	bool RecreateSwapchain();

    bool Init(GLFWwindow* window);
    void Shutdown();

    VkInstance GetInstance() const { return m_Instance; }
    VkSurfaceKHR GetSurface() const { return m_Surface; }

    VkPhysicalDevice GetPhysicalDevice() const { return m_PhysicalDevice; }
    VkDevice GetDevice() const { return m_Device; }

    VkFormat GetSwapchainImageFormat() const { return VKB_Swapchain.image_format; }
    VkExtent2D GetSwaphcainImageExtent() const { return VKB_Swapchain.extent; }
    const std::vector<VkImageView>& GetSwaphcainImageViews() const { return m_SwapchainImageViews; }

    private:
    bool CreateInstance();
    bool CreateSurface();
    bool ChoosePhysicalDevice();
    bool CreateDevice();
    bool CreateSwapchain();
    bool CreateImageViews();

    bool CreateCommandPool();
    bool CreateCommandBuffer();
    bool CreateSyncObjects();
    bool CreateSwapchainSyncObjects();

    void TransitionImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);

	void DestroySwaphchainResources();

    public:
    void RenderFrame(const OVulkanPipeline& pipeline);

    private:
    GLFWwindow* m_Window = nullptr;

    vkb::Instance VKB_Instance;
    vkb::PhysicalDevice VKB_PhysicalDevice;
    vkb::Device VKB_Device;
    vkb::Swapchain VKB_Swapchain;

    VkInstance m_Instance = VK_NULL_HANDLE;
    VkSurfaceKHR m_Surface = VK_NULL_HANDLE;

    VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
    VkDevice m_Device = VK_NULL_HANDLE;

    VkQueue graphicsQueue = VK_NULL_HANDLE;
    uint32_t graphicsQueueIndex = 0;

    VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> m_SwapchainImages;
    std::vector<VkImageView> m_SwapchainImageViews;

	std::vector<VkImageLayout> m_SwapchainImageLayouts;

    VkCommandPool m_CommandPool = VK_NULL_HANDLE;
    VkCommandBuffer m_CommandBuffer = VK_NULL_HANDLE;

    VkSemaphore m_ImageAvailableSemaphore = VK_NULL_HANDLE;
	std::vector<VkSemaphore> m_RenderFinishedSemaphores;

    VkFence m_InFlightFence = VK_NULL_HANDLE;
};
} // namespace Orion
