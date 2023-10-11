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

		VkImage textureImage;
		VkDeviceMemory textureImageMemory;
		VkImageView textureImageView;
		VkSampler textureSampler;

	};
}
