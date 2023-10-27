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

	std::unique_ptr<Model> Model::createSmoothSurfaceFromVector(Device& device, const std::vector<Vertex>& vertices, uint32_t rows, uint32_t cols)
	{
		Builder builder{};
		builder.vertices = vertices;
		
		for (int i = 0; i < rows - 1; i++)
		{
			for (int j = 0; j < cols - 1; j++)
			{
				builder.indices.push_back(i * cols + j);
				builder.indices.push_back(i * cols + j + 1);
				builder.indices.push_back((i + 1) * cols + j);
				builder.indices.push_back(i * cols + j + 1);
				builder.indices.push_back((i + 1) * cols + j + 1);
				builder.indices.push_back((i + 1) * cols + j);
			}
		}

		uint32_t triangleCount = builder.indices.size()/3;

		for (int i = 0; i < triangleCount; i++)
		{
			uint32_t triangleIndex = i * 3;
			glm::vec3 vec1 = builder.vertices[builder.indices[triangleIndex]].position;
			glm::vec3 vec2 = builder.vertices[builder.indices[triangleIndex + 1]].position;
			glm::vec3 vec3 = builder.vertices[builder.indices[triangleIndex + 2]].position;

			glm::vec3 side1 = vec2 - vec1;
			glm::vec3 side2 = vec3 - vec1;
			glm::vec3 triangleNormal = glm::cross(side1, side2);

			builder.vertices[builder.indices[triangleIndex]].normal += triangleNormal;
			builder.vertices[builder.indices[triangleIndex + 1]].normal += triangleNormal;
			builder.vertices[builder.indices[triangleIndex + 2]].normal += triangleNormal;
		}
		for (auto& v : builder.vertices)
			v.normal = glm::normalize(v.normal);

		return std::make_unique<Model>(device, builder);
	}

	std::unique_ptr<Model> Model::createFlatSurfaceFromVector(Device& device, const std::vector<Vertex>& vertices, uint32_t rows, uint32_t cols)
	{
		Builder builder{};
		
		std::vector<uint32_t> indices;
		for (int i = 0; i < rows - 1; i++)
		{
			for (int j = 0; j < cols - 1; j++)
			{
				indices.push_back(i * cols + j);
				indices.push_back(i * cols + j + 1);
				indices.push_back((i + 1) * cols + j);
				indices.push_back(i * cols + j + 1);
				indices.push_back((i + 1) * cols + j + 1);
				indices.push_back((i + 1) * cols + j);
			}
		}

		for (uint32_t i : indices)
			builder.vertices.push_back(vertices[i]);

		uint32_t triangleCount = builder.vertices.size()/3;

		for (int i = 0; i < triangleCount; i++)
		{
			uint32_t triangleIndex = i * 3;
			glm::vec3 vec1 = vertices[indices[triangleIndex]].position;
			glm::vec3 vec2 = vertices[indices[triangleIndex + 1]].position;
			glm::vec3 vec3 = vertices[indices[triangleIndex + 2]].position;

			glm::vec3 side1 = vec2 - vec1;
			glm::vec3 side2 = vec3 - vec1;
			glm::vec3 triangleNormal = glm::cross(side1, side2);

			builder.vertices[triangleIndex].normal = triangleNormal;
			builder.vertices[triangleIndex + 1].normal = triangleNormal;
			builder.vertices[triangleIndex + 2].normal = triangleNormal;
		}
		for (auto& v : builder.vertices)
			v.normal = glm::normalize(v.normal);

		return std::make_unique<Model>(device, builder);

	}

	std::vector<Model::Vertex> Model::calculateSplineSurface(int degreeU, int degreeV, std::vector<float>& knotsU, std::vector<float>& knotsV, std::vector<Vertex>& controlPoints)
	{
		std::vector<Vertex> resultVector;
		for (float u = 0.f; u < knotsU.at(knotsU.size() - 1); u += 0.25f)
		{
			for (float v = 0.f; v < knotsV.at(knotsV.size() - 1); v += 0.25f)
			{
				resultVector.push_back(calculateSpline(u, v, degreeU, degreeV, knotsU, knotsV, controlPoints));
			}
		}
		return resultVector;
	}

	Model::Vertex Model::calculateSpline(
		float u,
		float v,
		int degreeU,
		int degreeV,
		std::vector<float>& knotsU,
		std::vector<float>& knotsV,
		std::vector<Vertex>& controlPoints
	)
	{
		Vertex result;
		int numRows = controlPoints.size() / (degreeU + 1) + 1;
		int numCols = controlPoints.size() / numRows;

		for (int i = 0; i < numRows; i++) {
			for (int j = 0; j < numCols; j++) {
				double N_i = basisFunction(i, degreeU, u, knotsU);
				double N_j = basisFunction(j, degreeV, v, knotsV);
				//double N_ip1 = basisFunction(i + 1, degreeU, u, knotsU);
				//double N_jp1 = basisFunction(j + 1, degreeV, v, knotsV);

				int controlPointIndex = i * numCols + j;

				result.position.x += N_i * N_j * controlPoints[controlPointIndex].position.x;
				result.position.y += N_i * N_j * controlPoints[controlPointIndex].position.y;
				result.position.z += N_i * N_j * controlPoints[controlPointIndex].position.z;

				//controlPointIndex = (i + 1) * numCols + (j + 1);

				//result.position.x += N_ip1 * N_jp1 * controlPoints[controlPointIndex].position.x;
				//result.position.y += N_ip1 * N_jp1 * controlPoints[controlPointIndex].position.y;
				//result.position.z += N_ip1 * N_jp1 * controlPoints[controlPointIndex].position.z;
			}
		}

		return result;
	}

	double Model::basisFunction(int i, int p, float u, const std::vector<float>& knots)
	{
		if (p == 0)
		{
			if (u >= knots[i] && u < knots[i + 1])
				return 1.f;
			else
				return 0.f;
		}
		float firstFractionTop = (u - knots[i]) * basisFunction(i, p - 1, u, knots);
		float firstFractionBottom = (knots[i + p] - knots[i]);
		float firstFraction = firstFractionBottom == 0 ? 0 : firstFractionTop / firstFractionBottom;
		float secondFractionTop = (knots[i + p + 1] - u) * basisFunction(i + 1, p - 1, u, knots);
		float secondFractionBottom = (knots[i + p + 1] - knots[i + 1]);
		float secondFraction = secondFractionBottom == 0 ? 0 : secondFractionTop / secondFractionBottom;

		return firstFraction + secondFraction;
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
