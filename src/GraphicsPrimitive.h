#pragma once

#include "Device.h"
#include "Buffer.h"
#include "ImageTexture.h"

#include "utils.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <memory>
#include <vector>

namespace assignment
{
	class GraphicsPrimitive
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
					normal == other.normal &&
					uv == other.uv;
			}
		};

		GraphicsPrimitive(Device& device, const std::vector<Vertex>& vertices, const std::vector<uint32_t>* indices = nullptr);

	public:
		void bind(VkCommandBuffer commandBuffer);
		void draw(VkCommandBuffer commandBuffer);

		virtual void createVertexBuffers(const std::vector<Vertex>& vertices);
		virtual void createIndexBuffers(const std::vector<uint32_t>& indices);

	protected:
		Device& device;

		std::unique_ptr<Buffer> vertexBuffer;
		uint32_t vertexCount;

		bool hasIndexBuffer = false;
		std::unique_ptr<Buffer> indexBuffer;
		uint32_t indexCount;
	};
}

namespace std
{
	template<>
	struct hash<assignment::GraphicsPrimitive::Vertex> {
		size_t operator()(assignment::GraphicsPrimitive::Vertex const& vertex) const
		{
			size_t seed = 0;
			assignment::hashCombine(
				seed,
				vertex.position,
				vertex.color,
				vertex.normal,
				vertex.uv
			);
			return seed;
		}
	};
}

