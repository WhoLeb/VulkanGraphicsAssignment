#pragma once

#include "Device.h"

#include <vector>
#include <string>


namespace assignment
{
	struct PipelineConfigInfo
	{
		VkViewport								viewport;
		VkRect2D								scisor;
		VkPipelineInputAssemblyStateCreateInfo	inputAssemblyInfo;
		VkPipelineRasterizationStateCreateInfo	rasterizationInfo;
		VkPipelineMultisampleStateCreateInfo	multisampleInfo;
		VkPipelineColorBlendAttachmentState		colorBlendAttachment;
		VkPipelineColorBlendStateCreateInfo		colorBlendInfo;
		VkPipelineDepthStencilStateCreateInfo	depthStencilInfo;
		//std::vector<VkDynamicState>				dynamicStates;
		//VkPipelineDynamicStateCreateInfo		dynamicStateInfo;
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
		static PipelineConfigInfo defaultPipelineConfigInfo(uint32_t width, uint32_t height);

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
