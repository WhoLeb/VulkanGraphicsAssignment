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
	class SimpleRenderSystem
	{
	public:
		SimpleRenderSystem(Device& device, VkRenderPass renderPass);
		~SimpleRenderSystem();

		NO_COPY(SimpleRenderSystem);

	public:
		void renderGameObjects(FrameInfo& frameInfo, std::vector<GameObject>& gameObjects);

	private:
		void createPipelineLayout();
		void createPipeline(VkRenderPass renderPass);

	private:
		Device& device;

		std::unique_ptr<Pipeline> pipeline;
		VkPipelineLayout pipelineLayout;
	};
}
