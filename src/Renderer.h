#pragma once

#include "Device.h"
#include "Window.h"
#include "SwapChain.h"
#include "Model.h"

#include <memory>
#include <vector>
#include <cassert>

namespace assignment
{
	class Renderer
	{
	public:
		Renderer(Window& window, Device& device);
		~Renderer();

		NO_COPY(Renderer);

	public:
		VkRenderPass getSwapChainRenderPass() const { return swapChain->getRenderPass(); }
		bool isFrameInProgress() const { return isFrameStarted; }

		VkCommandBuffer getCurrentCommandBuffer() const {
			assert(isFrameStarted && "Cannot get command buffer when frame not in progress");
			return commandBuffers[currentFrameIndex]; 
		}

		VkCommandBuffer beginFrame();
		void endFrame();
		void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
		void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

		int getFrameIndex() const
		{
			assert(isFrameStarted && "Cannot get frame index when frame not in progress");
			return currentFrameIndex;
		}

	private:
		void createCommandBuffers();
		void freeCommandBuffers();
		void recreateSwapChain();

	private:
		Window& window;
		Device& device;
		std::unique_ptr<SwapChain> swapChain;
		std::vector<VkCommandBuffer> commandBuffers;

		uint32_t currentImageIndex;
		int currentFrameIndex;
		bool isFrameStarted = false;
	};
}
