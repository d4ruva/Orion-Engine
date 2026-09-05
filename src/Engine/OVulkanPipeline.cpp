#include "OVulkanPipeline.h"

#include "OLog.h"

#include <cstddef>
#include <fstream>
#include <vulkan/vulkan_core.h>

namespace Orion {
static std::vector<char> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        ORION_ERROR("Failed to Open shader file: {}", filename);
        return {};
    }

    const size_t fileSize = static_cast<size_t>(file.tellg());

    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}


bool OVulkanPipeline::Init(VkDevice device, VkFormat colorFormat, OVulkanContext& vulkanContext) {
	m_VulkanContext = &vulkanContext;
    m_Device = device;

    if (!CreateShaderModules()) {
        return false;
    }
    if (!CreatePipelineLayout()) {
        Shutdown();
        return false;
    }
    if (!CreateGraphicsPipeline(colorFormat)) {
        Shutdown();
        return false;
    }

    ORION_INFO("Graphics Pipeline Created Successfully");

    return true;
}

bool OVulkanPipeline::CreateShaderModules() {
    auto vertCode = readFile("assets/shaders/triangle.vert.spv");
    auto fragCode = readFile("assets/shaders/triangle.frag.spv");

    if (vertCode.empty() || fragCode.empty()) {
        ORION_ERROR("Failed to load Binary Shader");
		return false;
    }

	m_VertexShader = CreateShaderModule(vertCode);
	m_FragmentShader = CreateShaderModule(fragCode);

	if(m_VertexShader == VK_NULL_HANDLE || m_FragmentShader == VK_NULL_HANDLE)
	{
		ORION_ERROR("Failed to create Shader modules");
		return false;
	}

	return true;

}
bool OVulkanPipeline::CreatePipelineLayout() {
    VkPipelineLayoutCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 0,
        .pSetLayouts = nullptr,

        .pushConstantRangeCount = 0,
        .pPushConstantRanges = nullptr,
    };

    if (vkCreatePipelineLayout(m_Device, &createInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS) {
        ORION_ERROR("Failed to create pipeline layout");
        return false;
    }
    return true;
}
bool OVulkanPipeline::CreateGraphicsPipeline(VkFormat colorFormat) {
    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = m_VertexShader,
        .pName = "main",
    };

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = m_FragmentShader,
        .pName = "main",
    };

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
        .pDynamicStates = dynamicStates.data()
    };

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 0,
        .pVertexBindingDescriptions = nullptr,
        .vertexAttributeDescriptionCount = 0,
        .pVertexAttributeDescriptions = nullptr,
    };

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE,
    };

	VkViewport viewPort{
		.x = 0.0f,
		.y = 0.0f,
		.width = (float) m_VulkanContext->GetSwaphcainImageExtent().width,
		.height= (float) m_VulkanContext->GetSwaphcainImageExtent().height,
		.minDepth = 0.0f,
		.maxDepth = 1.0f,
	};

	VkRect2D scissor{
		.offset = {0, 0},
		.extent = m_VulkanContext->GetSwaphcainImageExtent(),
	};

    VkPipelineViewportStateCreateInfo viewportState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
		.pViewports = &viewPort,
        .scissorCount = 1,
		.pScissors = &scissor,
    };

	VkPipelineRasterizationStateCreateInfo rasterizer{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.depthClampEnable = VK_FALSE,
		.rasterizerDiscardEnable = VK_FALSE,
		.polygonMode = VK_POLYGON_MODE_FILL,
		.cullMode = VK_CULL_MODE_BACK_BIT,
		.frontFace = VK_FRONT_FACE_CLOCKWISE,
		.depthBiasEnable = VK_FALSE
	};
	rasterizer.lineWidth = 1.0f;

	VkPipelineMultisampleStateCreateInfo multisampling = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
		.sampleShadingEnable = VK_FALSE,
	};

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {
		.blendEnable = VK_FALSE
	};

		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

	VkPipelineColorBlendStateCreateInfo colorBlending{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.logicOpEnable = VK_FALSE,
		.logicOp = VK_LOGIC_OP_COPY,
		.attachmentCount = 1,
		.pAttachments = &colorBlendAttachment,
	};


	VkPipelineRenderingCreateInfo renderingInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
		.colorAttachmentCount = 1,
		.pColorAttachmentFormats = &colorFormat
	};

	VkGraphicsPipelineCreateInfo pipelineInfo = {
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.pNext = &renderingInfo,
		.stageCount = 2,
		.pStages = shaderStages,
		.pVertexInputState = &vertexInputInfo,
		.pInputAssemblyState = &inputAssemblyState,
		.pViewportState = &viewportState,
		.pRasterizationState = &rasterizer,
		.pMultisampleState = &multisampling,
		.pColorBlendState = &colorBlending,
		.pDynamicState = &dynamicState,
		.layout = m_PipelineLayout,
		.basePipelineHandle = VK_NULL_HANDLE
	};


	if(vkCreateGraphicsPipelines(m_Device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_Pipeline) != VK_SUCCESS)
	{
		ORION_ERROR("Failed to create Grraphics Pipeline");
		return false;
	}

	return true;

}

VkShaderModule OVulkanPipeline::CreateShaderModule(const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = code.size(),
        .pCode = reinterpret_cast<const uint32_t*>(code.data())
    };

    VkShaderModule shaderModule = VK_NULL_HANDLE;

    if (vkCreateShaderModule(m_Device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        ORION_ERROR("Failed to create shader module");
        return VK_NULL_HANDLE;
    }

    return shaderModule;
}

void OVulkanPipeline::Shutdown()
	{

	vkDeviceWaitIdle(m_Device);

	if(m_Pipeline)
	{
		vkDestroyPipeline(m_Device, m_Pipeline, nullptr);
	}

	if(m_PipelineLayout)
	{
		vkDestroyPipelineLayout(m_Device, m_PipelineLayout, nullptr);
	}

	if(m_VertexShader)
	{
		vkDestroyShaderModule(m_Device, m_VertexShader, nullptr);
	}

	if(m_FragmentShader)
	{
		vkDestroyShaderModule(m_Device, m_FragmentShader, nullptr);
	}
}
} // namespace Orion``
