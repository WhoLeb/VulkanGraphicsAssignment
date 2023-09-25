#pragma once

#include "Device.h"

#include <vector>
#include <string>


namespace assignment
{
	struct PipelineConfigInfo
	{
		NO_COPY(PipelineConfigInfo);
		PipelineConfigInfo() = default;

		VkPipelineViewportStateCreateInfo		viewportInfo;
		VkPipelineInputAssemblyStateCreateInfo	inputAssemblyInfo;
		VkPipelineRasterizationStateCreateInfo	rasterizationInfo; 
		VkPipelineMultisampleStateCreateInfo	multisampleInfo;
		VkPipelineColorBlendAttachmentState		colorBlendAttachment;
		VkPipelineColorBlendStateCreateInfo		colorBlendInfo;
		VkPipelineDepthStencilStateCreateInfo	depthStencilInfo;
		std::vector<VkDynamicState>				dynamicStateEnables;
		VkPipelineDynamicStateCreateInfo		dynamicStateInfo;
		VkPipelineLayout						pipelineLayout = nullptr;
		VkRenderPass							renderPass = nullptr;
		uint32_t								subpass = 0;
	};

	class Pipeline
	{
	public:
		NO_COPY(Pipeline);

		Pipeline(
			Device& device,
			const std::string& vertFilepath,
			const std::string& fragFilepath,
			const PipelineConfigInfo& configInfo);
		~Pipeline();

	public:
		void bind(VkCommandBuffer commandBuffer);
		static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo);

	private:
		static std::vector<char> readFile(const std::string& filepath);

		void createGraphicsPipeline(
			Device& device,
			const std::string& vertFilepath,
			const std::string& fragFilepath,
			const PipelineConfigInfo& configInfo);

		void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

	private:
		Device& m_device;
		VkPipeline graphicsPipeline;
		VkShaderModule vertShaderModule;
		VkShaderModule fragShaderModule;
	};
}
