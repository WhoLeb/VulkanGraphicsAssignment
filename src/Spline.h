#pragma once

#include "Device.h"
#include "Buffer.h"
#include "ImageTexture.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"

#include <memory>
#include <vector>

namespace assignment
{
	class Spline
	{
	public:
		struct Vertex {
			glm::vec3 position{};
			glm::vec3 color{};
			glm::vec3 normal{};
			glm::vec2 uv{};

			static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

			bool operator ==(const Vertex& other) const
			{
				return position == other.position &&
					color == other.color &&
					normal == other.normal;
			}
		};

		Spline(Device& device, const std::vector<Vertex>& builder);
		~Spline();

		NO_COPY(Spline);

	public:
		static std::unique_ptr<Spline> createSplineFromVector(Device& device, const std::vector<Vertex>& vertices);

		void bind(VkCommandBuffer commandBuffer);
		void draw(VkCommandBuffer commandBuffer);

		void createVertexBuffers(const std::vector<Vertex>& vertices);

	private:
		Device& device;

		std::unique_ptr<Buffer> vertexBuffer;
		uint32_t vertexCount;
	};

}
