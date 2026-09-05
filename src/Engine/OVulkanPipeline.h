#pragma once

#include "OVulkanContext.h"

#include <memory>
#include <vulkan/vulkan_core.h>

#include <vector>

namespace Orion {
class OVulkanPipeline {

	public:
	OVulkanPipeline() = default;
	~OVulkanPipeline() = default;

	bool Init(VkDevice device, VkFormat colorFormat, OVulkanContext& vulkanContext);
	void Shutdown();

	VkPipeline GetPipeline() const {return m_Pipeline;};
	VkPipelineLayout GetPipelineLayout() const {return m_PipelineLayout;};

private:
	bool CreateShaderModules();
	bool CreatePipelineLayout();
	bool CreateGraphicsPipeline(VkFormat colorFormat);

	VkShaderModule CreateShaderModule(const std::vector<char>& code);

private:
    OVulkanContext* m_VulkanContext = nullptr;

	VkDevice m_Device = VK_NULL_HANDLE;

	VkPipeline m_Pipeline = VK_NULL_HANDLE;
	VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;

	VkShaderModule m_VertexShader = VK_NULL_HANDLE;
	VkShaderModule m_FragmentShader = VK_NULL_HANDLE;

};
} // namespace Orion
