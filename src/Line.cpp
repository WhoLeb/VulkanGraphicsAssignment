#include "Line.h"

#include "utils.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <cassert>
#include <cstring>

namespace assignment
{
	Line::Line(Device& device, const std::vector<Vertex>& vertices)
		: GraphicsPrimitive(device, vertices)
	{}

	Line::~Line() {}

	std::unique_ptr<Line> Line::createLineFromVector(Device& device, const std::vector<Vertex>& vertices)
	{
		return std::make_unique<Line>(device, vertices);
	}

	std::unique_ptr<Line> Line::calculateCubicSplineWithCustomStep(Device& device, const std::vector<Vertex>& vertices, glm::vec3 P1, glm::vec3 Pn, const std::vector<float>& taus)
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
				Eigen::MatrixXf newVertex = weightMat * gMat;
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

	std::unique_ptr<Line> Line::calculateCubicSplineEvenlySpaced(Device& device, const std::vector<Vertex>& vertices, glm::vec3 P1, glm::vec3 Pn, uint32_t n)
	{
		std::vector<float> taus;
		for (uint32_t i = 0; i < n; i++)
			taus.push_back(float(i + 1) / float(n+1));

		return calculateCubicSplineWithCustomStep(device, vertices, P1, Pn, taus);
	}

	std::unique_ptr<Line> Line::calculateBSplineUnordered(Device& device, const std::vector<Vertex>& vertices, uint32_t degree, const std::vector<float>& knots, uint32_t subdivisions)
	{
		assert(knots.size() >= vertices.size() + degree + 1 && "Not enough knots");

		std::vector<Vertex> newVertexVector = calculateBSpline(vertices, degree-1, knots, subdivisions);
		return std::make_unique<Line>(device, newVertexVector);
	}

	std::unique_ptr<Line> Line::calculateBSplineOpened(Device& device, const std::vector<Vertex>& vertices, uint32_t degree, uint32_t subdivisions)
	{
		std::vector<float> ts;
		for (uint32_t i = 1; i <= degree; i++)
			ts.push_back(0.f);
		for (uint32_t i = degree + 1; i <= vertices.size(); i++)
			ts.push_back(i - degree);
		for (uint32_t i = vertices.size() + 1; i <= vertices.size() + degree; i++)
			ts.push_back(vertices.size() - degree + 1);

		std::vector<Vertex> newVertexVector = calculateBSpline(vertices, degree-1, ts, subdivisions);
		return std::make_unique<Line>(device, newVertexVector);
	}

	std::unique_ptr<Line> Line::calculateBSplineEvenlySpaced(Device& device, const std::vector<Vertex>& vertices, uint32_t degree, uint32_t n, uint32_t subdivisions)
	{
		std::vector<float> ts;
		for (int i = 0; i < vertices.size() + degree + 1; i++)
			ts.push_back(static_cast<float>(i));
		std::vector<Vertex> newVertexVector = calculateBSpline(vertices, degree-1, ts, subdivisions);
		return std::make_unique<Line>(device, newVertexVector);
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
				weightMatrix (0, 0) = float(2 * glm::pow(tau, 3) - 3 * glm::pow(tau, 2) + 1);
				weightMatrix (0, 1) = float(- 2 * glm::pow(tau, 3) + 3 * glm::pow(tau, 2));
				weightMatrix (0, 2) = float(tau * (glm::pow(tau, 2) - 2 * tau + 1) * t[i + 1]);
				weightMatrix (0, 3) = float(tau * (glm::pow(tau, 2) - tau) * t[i + 1]);
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

		const uint32_t n = uint32_t(vertices.rows());

		for (uint32_t k = 0; k < n-1; k++)
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

	float Line::basisFunction(uint32_t i, uint32_t k, const std::vector<float>& ts, float t)
	{
		if (k == 0)
		{
			if (t >= ts[i] && t < ts[i + 1])
				return 1;
			else
				return 0;
		}
		float firstFractionTop = (t - ts[i]);
		float firstFractionBottom = (ts[i + k] - ts[i]);
		float firstFraction = firstFractionBottom == 0 ? 0 : firstFractionTop / firstFractionBottom;
		float secondFractionTop = (ts[i + k + 1] - t);
		float secondFractionBottom = (ts[i + k + 1] - ts[i + 1]);
		float secondFraction = secondFractionBottom == 0 ? 0 : secondFractionTop / secondFractionBottom;

		return firstFraction * basisFunction(i, k - 1, ts, t) + secondFraction * basisFunction(i + 1, k - 1, ts, t);
	}

	std::vector<Line::Vertex> Line::calculateBSpline(const std::vector<Vertex>& vertices, uint32_t degree, const std::vector<float>& ts, uint32_t subdivisions)
	{
		std::vector<Vertex> newVertexVector;
		for (uint32_t i = 0; i < subdivisions; i++)
		{
			Vertex p;
			p.position = { 0.f, 0.f, 0.f };
			p.color = vertices[0].color;

			for (uint32_t j = 0; j < vertices.size(); j++)
				p.position += vertices[j].position * basisFunction(j, degree, ts, float(vertices.size() - degree) * float(i) / float(subdivisions));
			newVertexVector.emplace_back(p);
		}
		newVertexVector.push_back(*(vertices.end()-1));

		return newVertexVector;
	}

}
