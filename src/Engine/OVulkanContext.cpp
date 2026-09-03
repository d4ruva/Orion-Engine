#include "OVulkanContext.h"
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

    return CreateInstance();
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

void OVulkanContext::Shutdown() {
    if (m_Instance != VK_NULL_HANDLE) {
        vkb::destroy_instance(VKB_Instance);

        m_Instance = nullptr;
    }
}
} // namespace Orion
