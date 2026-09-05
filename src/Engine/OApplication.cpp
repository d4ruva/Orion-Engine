#include "OApplication.h"
#include "GLFW/glfw3.h"
#include "OLog.h"
#include "OVulkanContext.h"
#include "OVulkanPipeline.h"
#include <memory>

namespace Orion {

OApplication::OApplication() { Init(); }

OApplication::~OApplication() { Shutdown(); }

bool OApplication::Init() {
    if (!glfwInit()) {
        ORION_ERROR("Failed to Initialize GLFW\n");
        return false;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_Window = glfwCreateWindow(m_Width, m_Height, "Untitled Window", nullptr, nullptr);

    if (!m_Window) {
        ORION_ERROR("Failed to create a GLFW window");
        glfwTerminate();
        return false;
    }

    m_VulkanContext = std::make_unique<OVulkanContext>();

    if (!m_VulkanContext->Init(m_Window)) {
        ORION_ERROR("Failed to Initialize Vulkan\n");
        return false;
    }

	m_Pipeline = std::make_unique<OVulkanPipeline>();

	if(!m_Pipeline->Init(m_VulkanContext->GetDevice(), m_VulkanContext->GetSwapchainImageFormat(), *m_VulkanContext))
	{
		ORION_ERROR("Failed To create grpahics pipeline");
		return false;

	}

    return true;
}

void OApplication::Run() {
    while (!glfwWindowShouldClose(m_Window)) {
        glfwPollEvents();

		m_VulkanContext->RenderFrame();
    }
}

void OApplication::Shutdown() {

	if(m_Pipeline)
	{
		m_Pipeline->Shutdown();
		m_Pipeline.reset();
	}

    if (m_VulkanContext) {
        m_VulkanContext->Shutdown();
        m_VulkanContext.reset();
    }
    if (m_Window) {
        glfwDestroyWindow(m_Window);
        m_Window = nullptr;
    }

    glfwTerminate();
}
}; // namespace Orion
