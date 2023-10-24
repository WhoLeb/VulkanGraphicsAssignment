#include "LinesRenderSystem.h"

#include <array>
#include <stdexcept>

namespace assignment
{
	struct SimplePushConstantData
	{
		glm::mat4 modelMatrix{ 1.f };
		glm::mat4 normalMatrix{ 1.f };
	};

	LinesRenderSystem::LinesRenderSystem(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
		: device(device)
	{
		createPipelineLayout(globalSetLayout);
		createPipeline(renderPass);
	}

	LinesRenderSystem::~LinesRenderSystem()
	{
		vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
	}

	void LinesRenderSystem::renderLineObjects(FrameInfo& frameInfo, std::vector<GameObject>& gameObjects)
	{
		pipeline->bind(frameInfo.commandBuffer);

		vkCmdBindDescriptorSets(
			frameInfo.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineLayout,
			0, 1,
			&frameInfo.globalDescriptorSet,
			0, nullptr);

		for (auto& obj : gameObjects)
		{
			if (!obj.visible)
				continue;

			SimplePushConstantData push{};
			push.modelMatrix = obj.transform.mat4();
			push.normalMatrix = obj.transform.normalMatrix();

			vkCmdPushConstants(
				frameInfo.commandBuffer,
				pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(SimplePushConstantData),
				&push);

			obj.line->bind(frameInfo.commandBuffer);
			obj.line->draw(frameInfo.commandBuffer);
		}
	}

	void LinesRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
	{
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SimplePushConstantData);

		std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ globalSetLayout };

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = uint32_t(descriptorSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

		if (vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
			throw std::runtime_error("Failed to create pipeline layout");
	}

	void LinesRenderSystem::createPipeline(VkRenderPass renderPass)
	{
		PipelineConfigInfo pipelineConfig{};
		Pipeline::defaultPipelineConfigInfo(pipelineConfig);

		pipelineConfig.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;

		pipelineConfig.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		pipelineConfig.rasterizationInfo.depthClampEnable = VK_FALSE;
		pipelineConfig.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
		pipelineConfig.rasterizationInfo.polygonMode = VK_POLYGON_MODE_LINE;
		pipelineConfig.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
		pipelineConfig.rasterizationInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		pipelineConfig.rasterizationInfo.depthBiasEnable = VK_FALSE;
		pipelineConfig.rasterizationInfo.depthBiasConstantFactor = 0.0f;  // Optional
		pipelineConfig.rasterizationInfo.depthBiasClamp = 0.0f;           // Optional
		pipelineConfig.rasterizationInfo.depthBiasSlopeFactor = 0.0f;     // Optional
		pipelineConfig.rasterizationInfo.lineWidth = 2.f;

		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = pipelineLayout;
		pipeline = std::make_shared<Pipeline>(
			device,
			"shaders/splineVert.spv",
			"shaders/splineFrag.spv",
			pipelineConfig);
	}

}