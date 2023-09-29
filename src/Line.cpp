#include "Line.h"

#include "utils.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <cassert>
#include <cstring>

namespace std
{
	template<>
	struct hash<assignment::Line::Vertex> {
		size_t operator()(assignment::Line::Vertex const& vertex) const
		{
			size_t seed = 0;
			assignment::hashCombine(seed, vertex.position, vertex.color, vertex.normal);
			return seed;
		}
	};
}

namespace assignment
{
	Line::Line(Device& device, const std::vector<Vertex>& vertices)
		: device(device)
	{
		createVertexBuffers(vertices);
	}

	Line::~Line() {}

	std::unique_ptr<Line> Line::createLineFromVector(Device& device, const std::vector<Vertex>& vertices)
	{
		return std::make_unique<Line>(device, vertices);
	}

	void Line::bind(VkCommandBuffer commandBuffer)
	{
		VkBuffer buffers[] = { vertexBuffer->getBuffer() };
		VkDeviceSize offsets[] = { 0 };

		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
	}

	void Line::draw(VkCommandBuffer commandBuffer)
	{
		vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
	}

	void Line::createVertexBuffers(const std::vector<Vertex>& vertices)
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

	std::unique_ptr<Line> Line::calculateSplineWithCustomStep(Device& device, const std::vector<Vertex>& vertices, glm::vec3 P1, glm::vec3 Pn, const std::vector<float>& taus)
	{
		Eigen::MatrixXf _vertices = Eigen::MatrixXf::Zero(vertices.size(), 3);
		for (uint64_t i = 0; i < vertices.size(); i++)
		{
			const auto v = vertices[i].position;
			_vertices(i, 0) = v.x;
			_vertices(i, 1) = v.y;
			_vertices(i, 2) = v.z;
		}

		Eigen::Vector3f _P1 = { P1.x, P1.y, P1.z };
		Eigen::Vector3f _Pn = { Pn.x, Pn.y, Pn.z };

		std::vector<float> t = { 1.f };
		calculateTs(vertices, t);

		Eigen::MatrixXf M = Eigen::MatrixXf::Identity(vertices.size(), vertices.size());
		for (uint64_t i = 1; i < vertices.size() - 1; i++)
		{
			M(i, i - 1) = t[i+1];
			M(i, i) = 2 * (t[i] + t[i+1]);
			M(i, i + 1) = t[i];
		}

		Eigen::MatrixXf tangentVectors = Eigen::MatrixXf::Zero(vertices.size(), 3);
		
		auto invM = M.inverse();
		auto rMatrix = RMatrix(_vertices, t, _P1, _Pn);

		tangentVectors = invM * rMatrix;
		
		std::vector<Eigen::MatrixXf> newVertices;
		
		auto gMatrices = formGMatrices(_vertices, tangentVectors);
		auto weightMats = weightMatrices(t, taus);

		for (int i = 0; i < vertices.size() - 1; i++)
		{
			for (int j = 0; j < taus.size(); j++)
			{
				auto weightMat = weightMats[(i * taus.size()) + j];
				auto gMat = gMatrices[i];
				auto newVertex = weightMat * gMat;
				newVertices.push_back(newVertex);
			}
		}

		std::vector<Vertex> newVertexArray;
		auto start = vertices.begin();
		int j = 0, k = 0;
		for (int i = 0; i < newVertices.size() + vertices.size(); i++)
		{
			assignment::Line::Vertex v;
			if (i % (taus.size() + 1) == 0)
			{
				v.position = vertices[j].position;
				v.color = vertices[j].color;
				j++;
			}
			else
			{
				v.color = (vertices[j].color + vertices[j-1].color)/2.f;
				v.position = glm::vec3( newVertices[k](0, 0), newVertices[k](0, 1), newVertices[k](0, 2) );
				k++;
			}
			newVertexArray.push_back(v);
		}

		return std::make_unique<Line>(device, newVertexArray);
	}

	std::unique_ptr<Line> Line::calculateSplineEvenlySpaced(Device& device, const std::vector<Vertex>& vertices, glm::vec3 P1, glm::vec3 Pn, uint32_t n)
	{
		std::vector<float> taus;
		for (int i = 0; i < n; i++)
			taus.push_back(float(i + 1) / float(n+1));

		return calculateSplineWithCustomStep(device, vertices, P1, Pn, taus);
	}

	void Line::calculateTs(const std::vector<Vertex>& vertices, std::vector<float>& t)
	{
		for (int i = 0; i < vertices.size() - 1; i++)
			t.push_back(glm::distance(vertices[i + 1].position, vertices[i].position));
	}

	std::vector<Eigen::MatrixXf> Line::weightMatrices(const std::vector<float>& t, std::vector<float> taus)
	{
		std::vector<Eigen::MatrixXf> weightMatrices;
		for (int i = 0; i < t.size() - 1; i++)
		{
			for (auto tau : taus)
			{
				Eigen::MatrixXf weightMatrix = Eigen::MatrixXf::Zero(1, 4);
				weightMatrix (0, 0) = 2 * glm::pow(tau, 3) - 3 * glm::pow(tau, 2) + 1;
				weightMatrix (0, 1) = -2 * glm::pow(tau, 3) + 3 * glm::pow(tau, 2);
				weightMatrix (0, 2) = tau * (glm::pow(tau, 2) - 2 * tau + 1) * t[i + 1];
				weightMatrix (0, 3) = tau * (glm::pow(tau, 2) - tau) * t[i + 1];
				weightMatrices.push_back(std::move(weightMatrix));
			}
		}

		return weightMatrices;
	}

	Eigen::MatrixXf Line::RMatrix(const Eigen::MatrixXf& vertices, const std::vector<float>& t, const Eigen::Vector3f& P1, const Eigen::Vector3f& Pn)
	{
		const uint64_t n = vertices.rows();
		Eigen::MatrixXf vectors = Eigen::MatrixXf::Zero(n, 3);

		vectors(0, 0) = P1.x();
		vectors(0, 1) = P1.y();
		vectors(0, 2) = P1.z();

		for (uint64_t i = 1; i < n - 1; i++)
		{
			vectors(i, 0) = (3.f / (t[i] * t[i + 1])) *
				(float(glm::pow(t[i], 2)) * (vertices(i + 1, 0) - vertices(i, 0)) +
				 float(glm::pow(t[i + 1], 2)) * (vertices(i, 0) - vertices(i - 1, 0)));
			vectors(i, 1) = (3.f / (t[i] * t[i + 1])) *
				(float(glm::pow(t[i], 2)) * (vertices(i + 1, 1) - vertices(i, 1)) +
				 float(glm::pow(t[i + 1], 2)) * (vertices(i, 1) - vertices(i - 1, 1)));
			vectors(i, 2) = (3.f / (t[i] * t[i + 1])) *
				(float(glm::pow(t[i], 2)) * (vertices(i + 1, 2) - vertices(i, 2)) +
				 float(glm::pow(t[i + 1], 2)) * (vertices(i, 2) - vertices(i - 1, 2)));
		}

		vectors(n - 1, 0) = Pn.x();
		vectors(n - 1, 1) = Pn.y();
		vectors(n - 1, 2) = Pn.z();

		return vectors;
	}

	std::vector<Eigen::MatrixXf> Line::formGMatrices(Eigen::MatrixXf& vertices, Eigen::MatrixXf& tangentVectors)
	{
		std::vector<Eigen::MatrixXf> mats;

		const int n = vertices.rows();

		for (int k = 0; k < n-1; k++)
		{
			Eigen::MatrixXf mat = Eigen::MatrixXf::Zero(4, 3);
			mat.row(0) = vertices.row(k);
			mat.row(1) = vertices.row(k+1);
			mat.row(2) = tangentVectors.row(k);
			mat.row(3) = tangentVectors.row(k+1);
			mats.push_back(std::move(mat));
		}
		return mats;
	}

	std::vector<VkVertexInputBindingDescription> Line::Vertex::getBindingDescriptions()
	{
		std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
		bindingDescriptions[0].binding = 0;
		bindingDescriptions[0].stride = sizeof(Vertex);
		bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescriptions;
	}

	std::vector<VkVertexInputAttributeDescription> Line::Vertex::getAttributeDescriptions()
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
