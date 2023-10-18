#include "Model.h"

#include "utils.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <cassert>
#include <cstring>

namespace assignment
{
	Model::Model(Device& device, const Builder& builder)
		: GraphicsPrimitive(device, builder.vertices, &builder.indices)
	{}

	Model::~Model() {}

	std::unique_ptr<Model> Model::createModelFromFile(Device& device, const std::string& filepath)
	{
		Builder builder{};
		builder.loadModel(filepath);

		return std::make_unique<Model>(device, builder);
	}

	std::unique_ptr<Model> Model::createModelFromVector(Device& device, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
	{
		Builder builder{};
		builder.vertices = vertices;
		builder.indices = indices;

		return std::make_unique<Model>(device, builder);
	}

	void Model::Builder::loadModel(const std::string& filename)
	{
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename.c_str()))
			throw std::runtime_error(warn + err);

		vertices.clear();
		indices.clear();

		std::unordered_map<Vertex, uint32_t> uniqueVertices{}; 
		for (const auto& shape : shapes)
		{
			for (const auto& index : shape.mesh.indices)
			{
				Vertex vertex{};

				if (index.vertex_index >= 0)
				{
					vertex.position =
					{
						attrib.vertices[3 * index.vertex_index + 0],
						attrib.vertices[3 * index.vertex_index + 1],
						attrib.vertices[3 * index.vertex_index + 2]
					};

					vertex.color =
					{
						attrib.colors[3 * index.vertex_index + 0],
						attrib.colors[3 * index.vertex_index + 1],
						attrib.colors[3 * index.vertex_index + 2]
					};
				}
				if (index.normal_index >= 0)
				{
					vertex.normal =
					{
						attrib.normals[3 * index.normal_index + 0],
						attrib.normals[3 * index.normal_index + 1],
						attrib.normals[3 * index.normal_index + 2]
					};
				}
				if (index.texcoord_index >= 0)
				{
					vertex.uv =
					{
						attrib.texcoords[2 * index.texcoord_index + 0],
						1.f - attrib.texcoords[2 * index.texcoord_index + 1]
					};
				}

				if (uniqueVertices.count(vertex) == 0)
				{
					uniqueVertices[vertex] = uint32_t(vertices.size());
					vertices.push_back(vertex);
				}
				indices.push_back(uniqueVertices[vertex]);
			}
		}
	}
}
