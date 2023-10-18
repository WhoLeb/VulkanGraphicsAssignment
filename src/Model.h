#pragma once

#include "GraphicsPrimitive.h"

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
	};
};

