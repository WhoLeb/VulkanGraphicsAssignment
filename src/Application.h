#pragma once

#include "Device.h"
#include "Window.h"
#include "Pipeline.h"
#include "SwapChain.h"

#include <memory>
#include <vector>

namespace assignment
{
	class Application
	{
	public:
		const int WIDTH = 800;
		const int HEIGHT = 600;

	public:
		Application();
		~Application();

		NO_COPY(Application);

	public:
		void run();

	private:
		void createPipelineLayout();
		void createPipeline();
		void createCommandBuffers();
		void drawFrame();

	private:
		Window window{ WIDTH, HEIGHT, "Application" };
		Device device{ window };
		SwapChain swapChain{ device, window.getExtent() };

		std::unique_ptr<Pipeline> pipeline;
		VkPipelineLayout pipelineLayout;
		std::vector<VkCommandBuffer> commandBuffers;
	};
}