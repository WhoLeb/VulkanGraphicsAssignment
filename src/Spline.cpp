#include "Spline.h"

#include "utils.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <cassert>
#include <cstring>

namespace std
{
	template<>
	struct hash<assignment::Spline::Vertex> {
		size_t operator()(assignment::Spline::Vertex const& vertex) const
		{
			size_t seed = 0;
			assignment::hashCombine(seed, vertex.position, vertex.color, vertex.normal);
			return seed;
		}
	};
}

namespace assignment
{
	Spline::Spline(Device& device, const std::vector<Vertex>& vertices)
		: device(device)
	{
		createVertexBuffers(vertices);
	}

	Spline::~Spline() {}

	std::unique_ptr<Spline> Spline::createSplineFromVector(Device& device, const std::vector<Vertex>& vertices)
	{
		return std::make_unique<Spline>(device, vertices);
	}

	void Spline::bind(VkCommandBuffer commandBuffer)
	{
		VkBuffer buffers[] = { vertexBuffer->getBuffer() };
		VkDeviceSize offsets[] = { 0 };

		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
	}

	void Spline::draw(VkCommandBuffer commandBuffer)
	{
		vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
	}

	void Spline::createVertexBuffers(const std::vector<Vertex>& vertices)
	{
		vertexCount = uint32_t(vertices.size());

		VkDeviceSize bufferSize = sizeof(Vertex) * vertexCount;
		uint32_t vertexSize = sizeof(vertices[0]);

		Buffer stagingBuffer(
			device,
			vertexSize,
			vertexCount,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		
		stagingBuffer.map();
		stagingBuffer.writeToBuffer((void*)vertices.data());

		vertexBuffer = std::make_unique<Buffer>(
			device,
			vertexSize,
			vertexCount,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		device.copyBuffer(stagingBuffer.getBuffer(), vertexBuffer->getBuffer(), bufferSize);
	}

	std::vector<VkVertexInputBindingDescription> Spline::Vertex::getBindingDescriptions()
	{
		std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
		bindingDescriptions[0].binding = 0;
		bindingDescriptions[0].stride = sizeof(Vertex);
		bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescriptions;
	}

	std::vector<VkVertexInputAttributeDescription> Spline::Vertex::getAttributeDescriptions()
	{
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions(3);

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].offset = offsetof(Vertex, position);
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].offset = offsetof(Vertex, color);
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].offset = offsetof(Vertex, normal);
		attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT; 

		return attributeDescriptions;
	}
}
