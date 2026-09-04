#pragma once

#include "GLFW/glfw3.h"
#include <memory>


namespace Orion {
class OVulkanContext;

class OApplication {
    public:
    OApplication();
    ~OApplication();
    void Run();

    private:
    bool Init();
    void Shutdown();

    private:
    GLFWwindow* m_Window = nullptr;

    uint32_t m_Width = 1280;
    uint32_t m_Height = 720;

    std::unique_ptr<OVulkanContext> m_VulkanContext;
};
} // namespace Orion
