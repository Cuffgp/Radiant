
#include "VulkanPipeline.h"
#include "VulkanSwapchain.h"

#include <fstream>

namespace Radiant {


	static std::vector<char> readFile(const std::string& filename) 
	{
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open()) 
		{
			std::println("Failed to open file!");
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}

	VulkanPipeline::VulkanPipeline(const std::string filepath)
	{
		CreatePipelineLayout();
		CreatePipeline();
	}

	VulkanPipeline::~VulkanPipeline()
	{
		auto device = VulkanDevice::Get().Device();

		vkDestroyShaderModule(device, m_ShaderModule, nullptr);
		vkDestroyPipeline(device, m_Pipeline, nullptr);
		vkDestroyPipelineLayout(device, m_PipelineLayout, nullptr);
	}

	void VulkanPipeline::CreatePipelineLayout()
	{
		auto device = VulkanDevice::Get().Device();

		VkPipelineLayoutCreateInfo pipelineLayoutInfo
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.setLayoutCount = 0, // Optional
			.pSetLayouts = nullptr, // Optional
			.pushConstantRangeCount = 0, // Optional
			.pPushConstantRanges = nullptr // Optional
		};

		if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS)
		{
			std::println("Failed to create pipeline layout!");
		}
	}

	void VulkanPipeline::CreatePipeline()
	{
		auto device = VulkanDevice::Get().Device();

		// Load shader

		auto shaderCode = readFile("assets/simple_shader.spv");

		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = shaderCode.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());

		if (vkCreateShaderModule(device, &createInfo, nullptr, &m_ShaderModule) != VK_SUCCESS) 
		{
			std::println("Failed to create shader module!");
		}

		VkPipelineShaderStageCreateInfo vertShaderStageInfo
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_VERTEX_BIT,
			.module = m_ShaderModule,
			.pName = "vertMain"
		};

		VkPipelineShaderStageCreateInfo fragShaderStageInfo
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
			.module = m_ShaderModule,
			.pName = "fragMain"
		};

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		std::vector<VkDynamicState> dynamicStates =
		{
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};

		VkPipelineDynamicStateCreateInfo dynamicState
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
			.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
			.pDynamicStates = dynamicStates.data()
		};

		VkPipelineVertexInputStateCreateInfo vertexInputInfo
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
			.vertexBindingDescriptionCount = 0,
			.pVertexBindingDescriptions = nullptr,
			.vertexAttributeDescriptionCount = 0,
			.pVertexAttributeDescriptions = nullptr // Optional
		};

		// ---- End hard coded

		VkPipelineInputAssemblyStateCreateInfo inputAssembly
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
			.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			.primitiveRestartEnable = VK_FALSE
		};

		VkPipelineViewportStateCreateInfo viewportState
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
			.viewportCount = 1,
			.pViewports = nullptr,
			.scissorCount = 1,
			.pScissors = nullptr
		};

		VkPipelineRasterizationStateCreateInfo rasterizer
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			.depthClampEnable = VK_FALSE,
			.rasterizerDiscardEnable = VK_FALSE,
			.polygonMode = VK_POLYGON_MODE_FILL,
			.cullMode = VK_CULL_MODE_NONE,
			.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
			.depthBiasEnable = VK_FALSE,
			.depthBiasConstantFactor = 0.0f, // Optional
			.depthBiasClamp = 0.0f, // Optional
			.depthBiasSlopeFactor = 0.0f, // Optional
			.lineWidth = 1.0f
		};

		VkPipelineMultisampleStateCreateInfo multisampling
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
			.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
			.sampleShadingEnable = VK_FALSE,
			.minSampleShading = 1.0f, // Optional
			.pSampleMask = nullptr, // Optional
			.alphaToCoverageEnable = VK_FALSE, // Optional
			.alphaToOneEnable = VK_FALSE // Optional
		};

		VkPipelineDepthStencilStateCreateInfo depthStencil
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
			.depthTestEnable = VK_TRUE,
			.depthWriteEnable = VK_TRUE,
			.depthCompareOp = VK_COMPARE_OP_LESS,
			.depthBoundsTestEnable = VK_TRUE,
			.stencilTestEnable = VK_FALSE,
			.front = {},  // Optional
			.back = {},   // Optional
			.minDepthBounds = 0.0f,  // Optional
			.maxDepthBounds = 1.0f,  // Optional
		};

		VkPipelineColorBlendAttachmentState colorBlendAttachment
		{
			.blendEnable = VK_TRUE,
			.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
			.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
			.colorBlendOp = VK_BLEND_OP_ADD,
			.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
			.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
			.alphaBlendOp = VK_BLEND_OP_ADD,
			.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
		};

		VkPipelineColorBlendStateCreateInfo colorBlending
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			.logicOpEnable = VK_FALSE,
			.logicOp = VK_LOGIC_OP_COPY, // Optional
			.attachmentCount = 1,
			.pAttachments = &colorBlendAttachment,
			//.blendConstants[0] = 0.0f, // Optional
			//.blendConstants[1] = 0.0f, // Optional
			//.blendConstants[2] = 0.0f, // Optional
			//.blendConstants[3] = 0.0f // Optional
		};

		VkPipelineRenderingCreateInfo pipelineRenderingInfo
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
			.pNext = nullptr,
			.colorAttachmentCount = 1,
			.pColorAttachmentFormats = &VulkanSwapchain::s_SwapchainImageFormat,
			.depthAttachmentFormat = VulkanSwapchain::s_SwapchainDepthFormat,
			.stencilAttachmentFormat = VK_FORMAT_UNDEFINED
		};

		VkGraphicsPipelineCreateInfo pipelineInfo
		{
			.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			.pNext = &pipelineRenderingInfo,
			.stageCount = 2,
			.pStages = shaderStages,
			.pVertexInputState = &vertexInputInfo,
			.pInputAssemblyState = &inputAssembly,
			.pViewportState = &viewportState,
			.pRasterizationState = &rasterizer,
			.pMultisampleState = &multisampling,
			.pDepthStencilState = &depthStencil, // Optional
			.pColorBlendState = &colorBlending,
			.pDynamicState = &dynamicState,
			.layout = m_PipelineLayout,
			.renderPass = nullptr,
			.subpass = 0,
			.basePipelineHandle = VK_NULL_HANDLE, // Optional
			.basePipelineIndex = -1 // Optional
		};

		auto res = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_Pipeline);

		if (res != VK_SUCCESS)
		{
			std::print("Failed to create graphics pipeline!");
		}
	}

	void VulkanPipeline::Bind(VkCommandBuffer buffer)
	{
		vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);
	}
}