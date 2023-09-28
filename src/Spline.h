#pragma once

#include "Device.h"
#include "Buffer.h"
#include "ImageTexture.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"

#include <Eigen/Dense>

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

		Spline(Device& device) : device(device) {}
		Spline(Device& device, const std::vector<Vertex>& vertices);
		~Spline();

		NO_COPY(Spline);

	public:
		static std::unique_ptr<Spline> createSplineFromVector(Device& device, const std::vector<Vertex>& vertices);

		void bind(VkCommandBuffer commandBuffer);
		void draw(VkCommandBuffer commandBuffer);

		void createVertexBuffers(const std::vector<Vertex>& vertices);

		void calculateSplineWithCustomStep(std::vector<Vertex>& vertices, glm::vec3 P1, glm::vec3 Pn, const std::vector<float>& taus);
		void calculateSplineEvenlySpaced(std::vector<Vertex>& vertices, glm::vec3 P1, glm::vec3 Pn, uint32_t n);

	private:
		void calculateTs(const std::vector<assignment::Spline::Vertex>& vertices, std::vector<float>& t);
		std::vector<Eigen::MatrixXf> weightMatrices(const std::vector<float>& t, std::vector<float> taus);
		Eigen::MatrixXf RMatrix(const Eigen::MatrixXf& vertices, const std::vector<float>& t, const Eigen::Vector3f& P1, const Eigen::Vector3f& Pn);
		std::vector<Eigen::MatrixXf> formGMatrices(Eigen::MatrixXf& vertices, Eigen::MatrixXf& tangentVectors);

	private:
		Device& device;

		std::unique_ptr<Buffer> vertexBuffer;
		uint32_t vertexCount;
	};

}
