#pragma once

#include "Camera.h"
#include "Device.h"
#include "GameObject.h"
#include "Pipeline.h"
#include "FrameInfo.h"

#include <memory>
#include <vector>

namespace assignment
{
	class SplineRenderSystem
	{
	public:
		SplineRenderSystem(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~SplineRenderSystem();

		NO_COPY(SplineRenderSystem);

	public:
		void renderSplineObjects(FrameInfo& frameInfo, std::vector<GameObject>& gameObjects);

	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderPass);

	private:
		Device& device;

		std::unique_ptr<Pipeline> pipeline;
		VkPipelineLayout pipelineLayout;
	};
}
