#pragma once

#include "Buffer.h"
#include "Device.h"

#include <string>

namespace assignment
{
	class ImageTexture
	{
	public:
		ImageTexture(Device& device, std::string filepath);
		~ImageTexture();

		NO_COPY(ImageTexture);

	public:
		void setImageInfo(
			uint32_t width,
			uint32_t height,
			VkFormat format,
			VkImageTiling tiling,
			VkImageUsageFlags usage);

		VkDescriptorImageInfo getImageDescriptor();

	private:
		void createTextureImage();
		void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
		void createTextureImageView();
		VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
		void createTextureSampler();

		bool hasStencilComponent(VkFormat format);

	private:
		Device& device;

		std::string filepath;

		VkImageCreateInfo imageInfo{};

		VkImage textureImage;
		VkDeviceMemory textureImageMemory;
		VkImageView textureImageView;

		VkSampler textureSampler;
	};
}
