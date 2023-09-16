#pragma once

#include "Device.h"

#include <vulkan/vulkan.h>

#include <string>
#include <vector>

namespace assignment
{

	class SwapChain
	{
	public:
		static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;
		
	public:
		SwapChain(Device& r_device, VkExtent2D windowExtent);
		~SwapChain();

		NO_COPY(SwapChain);

	public:
		VkFramebuffer getFrameBuffer(int index) { return swapChainFramebuffers[index]; }
		VkRenderPass getRenderPass() { return renderPass; }
		VkImageView getImageView(int index) { return swapChainImageViews[index]; }
		uint32_t imageCount() { return swapChainImages.size(); }
		VkFormat getSwapChainImageFormat() { return swapChainImageFormat; }
		VkExtent2D getSwapChainExtent() { return swapChainExtent; }
		uint32_t width() { return swapChainExtent.width; }
		uint32_t height() { return swapChainExtent.height; }

		float extentAspectRatio() { return float(swapChainExtent.width) / float(swapChainExtent.height); }
		VkFormat findDepthFormat();

		VkResult acquireNextImage(uint32_t* index);
		VkResult submitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex);

	private:
		void createSwapChain();
		void createImageViews();
		void createDepthResourses();
		void createRenderPass();
		void createFramebuffers();
		void createSyncObjects();

		VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	private:
		VkFormat swapChainImageFormat;
		VkExtent2D swapChainExtent;

		std::vector<VkFramebuffer> swapChainFramebuffers;
		VkRenderPass renderPass;

		std::vector<VkImage> depthImages;
		std::vector<VkDeviceMemory> depthImageMemories;
		std::vector<VkImageView> depthImageViews;
		std::vector<VkImage> swapChainImages;
		std::vector<VkImageView> swapChainImageViews;

		Device& m_device;
		VkExtent2D windowExtent;

		VkSwapchainKHR swapChain;

		std::vector<VkSemaphore> imageAvailableSemaphores;
		std::vector<VkSemaphore> renderFinishedSemaphores;
		std::vector<VkFence> inFlightFences;
		std::vector<VkFence> imagesInFlight;
		uint32_t currentFrame = 0;
	};

}
