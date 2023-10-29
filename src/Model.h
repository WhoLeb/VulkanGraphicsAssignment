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
	class Model : public GraphicsPrimitive
	{
	public:
		struct Builder
		{
			std::vector<Vertex> vertices{};
			std::vector<uint32_t> indices{};

			void loadModel(const std::string& filename);
		};

		Model(Device& device, const Model::Builder& builder);
		~Model();

		NO_COPY(Model);

	public:
		static std::unique_ptr<Model> createModelFromFile(Device& device, const std::string& filepath);
		static std::unique_ptr<Model> createModelFromVector(Device& device, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

		static std::unique_ptr<Model> createSmoothSurfaceFromVector(Device& device, const std::vector<Vertex>& vertices, uint32_t rows, uint32_t cols);
		static std::unique_ptr<Model> createFlatSurfaceFromVector(Device& device, const std::vector<Vertex>& vertices, uint32_t rows, uint32_t cols);

		static std::vector<Vertex> calculateSplineSurface(
			int degreeU,
			int degreeV,
			std::vector<float>& knotsU,
			std::vector<float>& knotsV,
			std::vector<Vertex>& controlPoints,
			uint32_t subdivisions = 10
		);

		static std::vector<float> calculateKnots(uint32_t degree, int size);

	public:
		static Vertex calculateSpline(
			float u,
			float v,
			int degreeU,
			int degreeV,
			std::vector<float>& knotsU,
			std::vector<float>& knotsV,
			std::vector<Vertex>& controlPoints
		);
		static float basisFunction(int i, int p, float u, const std::vector<float>& knots);

	};
};

