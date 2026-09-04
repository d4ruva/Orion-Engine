#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "VkBootstrap.h"
#include <vulkan/vulkan_core.h>


namespace Orion {
class OVulkanContext {

  public:
    OVulkanContext() = default;
    ~OVulkanContext() = default;

    bool Init(GLFWwindow *window);
    void Shutdown();

    VkInstance GetInstance() const {return m_Instance;}
    VkSurfaceKHR GetSurface() const {return m_Surface;}
    
    VkPhysicalDevice GetPhysicalDevice() const {return m_PhysicalDevice; }
    VkDevice GetDevice() const {return m_Device;}

  private:
    bool CreateInstance();
    bool CreateSurface();
    bool ChoosePhysicalDevice();
    bool CreateDevice();

  private:
    GLFWwindow *m_Window = nullptr;

    vkb::Instance VKB_Instance;
    vkb::PhysicalDevice VKB_PhysicalDevice;
    vkb::Device VKB_Device;

    VkInstance m_Instance = VK_NULL_HANDLE;
    VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
    
    VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
    VkDevice m_Device = VK_NULL_HANDLE;
    
    VkQueue graphicsQueue = VK_NULL_HANDLE;
    uint32_t graphicsQueueIndex = 0;
};
} // namespace Orion
