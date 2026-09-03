#include "GLFW/glfw3.h"
#include "VkBootstrap.h"
#include <vulkan/vulkan_core.h>

#include "OLog.h"

namespace Orion {
class OVulkanContext {

  public:
    OVulkanContext() = default;
    ~OVulkanContext() = default;

    bool Init(GLFWwindow *window);
    void Shutdown();

  private:
    bool CreateInstance();

  private:
    GLFWwindow *m_Window = nullptr;

    vkb::Instance VKB_Instance;
    VkInstance m_Instance = VK_NULL_HANDLE;
};
} // namespace Orion
