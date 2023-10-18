#pragma once

#include "GraphicsPrimitive.h"

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
	class Line : public GraphicsPrimitive
	{
	public:
		Line(Device& device, const std::vector<Vertex>& vertices);
		~Line();

		NO_COPY(Line);

	public:
		static std::unique_ptr<Line> createLineFromVector(Device& device, const std::vector<Vertex>& vertices);

		static std::unique_ptr<Line> calculateCubicSplineWithCustomStep(Device& device, const std::vector<Vertex>& vertices, glm::vec3 P1, glm::vec3 Pn, const std::vector<float>& taus);
		static std::unique_ptr<Line> calculateCubicSplineEvenlySpaced(Device& device, const std::vector<Vertex>& vertices, glm::vec3 P1, glm::vec3 Pn, uint32_t n);

		static std::unique_ptr<Line> calculateBSplineWithCustomStep(Device& device, const std::vector<Vertex>& vertices, const std::vector<float>& knots);
		static std::unique_ptr<Line> calculateBSplineEvenlySpaced(Device& device, const std::vector<Vertex>& vertices, uint32_t n);

	private:
		static void calculateTs(const std::vector<Vertex>& vertices, std::vector<float>& t);
		static std::vector<Eigen::MatrixXf> weightMatrices(const std::vector<float>& t, std::vector<float> taus);
		static Eigen::MatrixXf RMatrix(const Eigen::MatrixXf& vertices, const std::vector<float>& t, const Eigen::Vector3f& P1, const Eigen::Vector3f& Pn);
		static std::vector<Eigen::MatrixXf> formGMatrices(Eigen::MatrixXf& vertices, Eigen::MatrixXf& tangentVectors);
	};

}
