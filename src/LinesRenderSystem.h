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
	class LinesRenderSystem
	{
	public:
		LinesRenderSystem(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~LinesRenderSystem();

		NO_COPY(LinesRenderSystem);

	public:
		void renderSplineObjects(FrameInfo& frameInfo, std::vector<GameObject>& gameObjects);

	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderPass);

	private:
		Device& device;

		std::shared_ptr<Pipeline> pipeline;
		VkPipelineLayout pipelineLayout;
	};
}
