#include "OVulkanContext.h"
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

    return true;
}
bool OVulkanContext::CreateInstance() {
    auto inst_ret = vkb::InstanceBuilder()
                        .set_app_name("Orion Engine")
                        .request_validation_layers()
                        .set_debug_callback(VulkanDebugCallback)
                        .require_api_version(1, 4)
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

    auto phys_dev_ret = selector.set_minimum_version(1, 4).set_surface(m_Surface).select();

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

    if(!graphics_queue_index_ret){
        ORION_ERROR("Failed to get graphics Queue Index: {}", graphics_queue_index_ret.error().message());
        return false;
  }

    graphicsQueueIndex = graphics_queue_index_ret.value();

    ORION_INFO("Choose Graphics Queue and Index Successfully");

    return true;
}

void OVulkanContext::Shutdown() {

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
