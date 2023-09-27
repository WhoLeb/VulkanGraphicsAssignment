#include "Application.h"

#include "Camera.h"
#include "SimpleRenderSystem.h"
#include "SplineRenderSystem.h"
#include "Spline.h"
#include "KeyboardMovementController.h"
#include "Model.h"

#include "glm/gtx/rotate_vector.hpp"
#include <Eigen/Dense>

#include <stdexcept>
#include <chrono>
#include <array>
#include <iostream>

namespace special
{
	void calculateTs(const std::vector<assignment::Spline::Vertex>& vertices, std::vector<float>& t);
	Eigen::Matrix4f splineSegmentMatrix(const std::vector<float>& t, uint64_t k);
	Eigen::MatrixXf RMatrix(const Eigen::MatrixXf& vertices, const std::vector<float>& t, const Eigen::Vector3f& P1, const Eigen::Vector3f& Pn);
	std::vector<Eigen::MatrixXf> weightMatrices(const std::vector<float>& t, std::vector<float> taus);
	std::vector<Eigen::MatrixXf> formGMatrices(Eigen::MatrixXf& vertices, Eigen::MatrixXf& tangentVectors);

	void spline(std::vector<assignment::Spline::Vertex>& vertices, glm::vec3 P1, glm::vec3 Pn, const std::vector<float>& taus)
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

		std::vector<assignment::Spline::Vertex> newVertexArray;
		auto start = vertices.begin();
		int j = 0, k = 0;
		for (int i = 0; i < newVertices.size() + vertices.size(); i++)
		{
			assignment::Spline::Vertex v;
			if (i % (taus.size() + 1) == 0)
			{
				v.position = vertices[j].position;
				j++;
			}
			else
			{
				v.position = glm::vec3( newVertices[k](0, 0), newVertices[k](0, 1), newVertices[k](0, 2) );
				k++;
			}
			v.color = glm::vec3(1.f);
			newVertexArray.push_back(v);
		}

		vertices = newVertexArray;
	}

	Eigen::Matrix4f splineSegmentMatrix(const std::vector<float>& t, uint64_t k)
	{
		Eigen::Matrix4f returnMatrix = Eigen::Matrix4f::Identity();
		auto t_k1 = t[k + 1];
		returnMatrix(0, 2) = -3.f / (t_k1 * t_k1);
		returnMatrix(1, 2) = -2.f / (t_k1);
		returnMatrix(2, 2) =  3.f / (t_k1 * t_k1);
		returnMatrix(3, 2) = -1.f / (t_k1);
		returnMatrix(0, 3) =  2.f / (t_k1 * t_k1 * t_k1);
		returnMatrix(1, 3) =  1.f / (t_k1 * t_k1);
		returnMatrix(2, 3) = -2.f / (t_k1 * t_k1 * t_k1);
		returnMatrix(3, 3) =  1.f / (t_k1 * t_k1);

		return returnMatrix;
	}

	std::vector<Eigen::MatrixXf> weightMatrices(const std::vector<float>& t, std::vector<float> taus)
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

	void calculateTs(const std::vector<assignment::Spline::Vertex>& vertices, std::vector<float>& t)
	{
		for (int i = 0; i < vertices.size() - 1; i++)
			t.push_back(glm::distance(vertices[i + 1].position, vertices[i].position));
	}

	Eigen::MatrixXf RMatrix(const Eigen::MatrixXf& vertices, const std::vector<float>& t, const Eigen::Vector3f& P1, const Eigen::Vector3f& Pn)
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

	std::vector<Eigen::MatrixXf> formGMatrices(Eigen::MatrixXf& vertices, Eigen::MatrixXf& tangentVectors)
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
}

namespace assignment
{
	struct GlobalUbo {
		glm::mat4 projectionView{ 1.f };
		glm::vec3 lightDirection = glm::normalize(glm::vec3{ 1.f, -1.f, -1.f });
	};

	Application::Application()
	{
		globalPool = DescriptorPool::Builder(device)
			.setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, SwapChain::MAX_FRAMES_IN_FLIGHT)
			.build();
		loadGameObjects();
		
		textureImage = std::make_unique<ImageTexture>(device, "assets/textures/white.png");
	}

	Application::~Application() {}

	void Application::run()
	{
		std::vector<std::unique_ptr<Buffer>> uboBuffers(SwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < uboBuffers.size(); i++)
		{
			uboBuffers[i] = std::make_unique<Buffer>(
				device,
				sizeof(GlobalUbo),
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

			uboBuffers[i]->map();
		}

		auto globalSetLayout = DescriptorSetLayout::Builder(device)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
			.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.build();

		std::vector<VkDescriptorSet> globalDescriptorSets(SwapChain::MAX_FRAMES_IN_FLIGHT);
		auto descriptor = textureImage->getImageDescriptor();
		for (int i = 0; i < globalDescriptorSets.size(); i++)
		{
			auto bufferInfo = uboBuffers[i]->descriptorInfo();
			DescriptorWriter(*globalSetLayout, *globalPool)
				.writeBuffer(0, &bufferInfo)
				.writeImage(1, &descriptor)
				.build(globalDescriptorSets[i]);
		}

		SimpleRenderSystem simpleRenderSystem(device, renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout());
		SplineRenderSystem splineRenderSystem(device, renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout());

		auto viewerObject = GameObject::createGameObject(); 
		KeyboardMovementController cameraController{};

		Camera camera{};
		camera.setViewTarget({3.f, -2.f, -3.f}, glm::vec3(0.f));

		auto currentTime = std::chrono::high_resolution_clock::now();

		auto startTime = std::chrono::high_resolution_clock::now();
		
		int lightingOption = 0;
		glm::vec3 lightingOptions[] = { glm::vec3(-1), glm::vec3(1.f, -1.f, -1.f) };

		int dimetricOption = 0;
		std::vector<float> dimetricOptions = { 0, 1.f / 4.f, 3.f / 8.f, 0.5f, 5.f / 8.f, 3.f / 4.f, 0.8165f, 1.f };

		int angleOption = 0;
		std::vector<std::pair<float, float>> angleOptions =
		{
			{ -1, 1 }, 
			{ -1, -1 }, 
			{ 1, 1 }, 
			{ 1, -1 } 
		};

		bool boundCamera = false;

		auto lastKeystroke = std::chrono::high_resolution_clock::now();

		while (!window.shouldClose())
		{
			glfwPollEvents();

			auto newTime = std::chrono::high_resolution_clock::now();
			float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
			currentTime = newTime;

			cameraController.moveInPlaneXZ(window.getGLFWwindow(), frameTime, viewerObject);
			camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

			float aspect = renderer.getAspectRatio();
			//camera.setOrthographicProjection(-aspect, -1, -1, aspect, 1, 30);
			camera.setPerspecitveProjection(glm::radians(45.f), aspect, 0.1f, 10.f);

			GlobalUbo ubo{};
			{
				auto keysAvailable = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastKeystroke).count() > .15f;
				if (keysAvailable &&
					glfwGetKey(window.getGLFWwindow(), GLFW_KEY_SPACE) == GLFW_PRESS)
				{
					lightingOption++;
					lightingOption %= 2;
					lastKeystroke = std::chrono::high_resolution_clock::now();
				}
				if (keysAvailable &&
					glfwGetKey(window.getGLFWwindow(), GLFW_KEY_L) == GLFW_PRESS)
				{
					dimetricOption++;
					dimetricOption %= dimetricOptions.size();
					lastKeystroke = std::chrono::high_resolution_clock::now();
					boundCamera = true;
				}
				if (keysAvailable &&
					glfwGetKey(window.getGLFWwindow(), GLFW_KEY_J) == GLFW_PRESS)
				{
					dimetricOption--;
					dimetricOption %= dimetricOptions.size();
					lastKeystroke = std::chrono::high_resolution_clock::now();
					boundCamera = true;
				}
				if (keysAvailable &&
					glfwGetKey(window.getGLFWwindow(), GLFW_KEY_I) == GLFW_PRESS)
				{
					angleOption++;
					angleOption %= angleOptions.size();
					lastKeystroke = std::chrono::high_resolution_clock::now();
					boundCamera = true;
				}
				if (keysAvailable &&
					glfwGetKey(window.getGLFWwindow(), GLFW_KEY_K) == GLFW_PRESS)
				{
					angleOption--;
					angleOption %= angleOptions.size();
					lastKeystroke = std::chrono::high_resolution_clock::now();
					boundCamera = true;
				}
				if (keysAvailable &&
					glfwGetKey(window.getGLFWwindow(), GLFW_KEY_O) == GLFW_PRESS)
				{
					viewerObject.transform.translation = { 0.f, 0.f, 0.f };
					viewerObject.transform.rotation = { 0.f, 0.f, 0.f };
					lastKeystroke = std::chrono::high_resolution_clock::now();
					boundCamera = true;
				}
				if (keysAvailable &&
					glfwGetKey(window.getGLFWwindow(), GLFW_KEY_U) == GLFW_PRESS)
				{
					dimetricOption = dimetricOptions.size() - 2;
					lastKeystroke = std::chrono::high_resolution_clock::now();
					boundCamera = true;
				}

			}
			if (auto commandBuffer = renderer.beginFrame())
			{
				int frameIndex = renderer.getFrameIndex();
				FrameInfo frameInfo{
					frameIndex,
					frameTime,
					commandBuffer,
					camera,
					globalDescriptorSets[frameIndex]
				};

				// Установлении проекции камеры, направления света
				ubo.projectionView = camera.getProjection() * camera.getView();
				ubo.lightDirection = lightingOptions[lightingOption];

				// Изменение поворота
				if (boundCamera)
				{
					viewerObject.transform.rotation.x =
						angleOptions[angleOption].first *
						glm::asin(dimetricOptions[dimetricOption] / glm::sqrt(2));
					viewerObject.transform.rotation.y =
						angleOptions[angleOption].second *
						glm::asin(dimetricOptions[dimetricOption] / glm::sqrt(2 - glm::pow(dimetricOptions[dimetricOption], 2)));
					boundCamera = false;
				}
				//gameObjects[0].transform.rotation.y += glm::radians(15.f) * frameTime;

				uboBuffers[frameIndex]->writeToBuffer(&ubo);
				uboBuffers[frameIndex]->flush(); 

				// render
				renderer.beginSwapChainRenderPass(commandBuffer);
				simpleRenderSystem.renderGameObjects(frameInfo, gameObjects);
				splineRenderSystem.renderSplineObjects(frameInfo, splineObjects);
				renderer.endSwapChainRenderPass(commandBuffer);
				renderer.endFrame();
			}
		}

		vkDeviceWaitIdle(device.device());
	}

	void Application::loadGameObjects()
	{
		std::shared_ptr<Model> model = Model::createModelFromFile(device, "assets/meshes/my_cube.obj");


		std::vector<Spline::Vertex> vertices(5);
		for (auto& v : vertices)
		{
			v.color = glm::vec3(1.f);
		}
		vertices[0].position = { 0.f,  0.f, 0.f };
		vertices[1].position = { 1.f,  1.f, 0.f };
		vertices[2].position = { 2.f, -1.f, 0.f };
		vertices[3].position = { 3.f,  0.f, 0.f };
		vertices[4].position = { 4.f,  -2.f, 3.f };

		std::vector<float> taus;
		for (int i = 0, n = 10; i < n; i++)
			taus.push_back(float(i + 1) / float(n+1));

		special::spline(vertices, { 1.f, 1.f, 1.f }, { 1.f, 1.f, 0.f }, taus);

		std::shared_ptr<Spline> spline = Spline::createSplineFromVector(device, vertices);
		auto gameObject = GameObject::createGameObject();
		gameObject.spline = spline;
		gameObject.transform.translation = { 0.f, 0.f, 0.f };
		gameObject.transform.scale = glm::vec3(0.3f);
		gameObject.transform.rotation = { 0.f, 0.f, 0.f };
		splineObjects.push_back(std::move(gameObject));

		Spline::Vertex v1, v2;
		v1.position = { -1000.f, 0.f, 0.f };
		v1.color = { 1.f, 0.f, 0.f };
		v2.position = { 1000.f, 0.f, 0.f };
		v2.color = { 1.f, 0.f, 0.f };
		std::vector<Spline::Vertex> axisLine = { v1, v2 };

		auto axis = GameObject::createGameObject();
		axis.spline = Spline::createSplineFromVector(device, axisLine);
		axis.transform.translation = glm::vec3(0.f);
		axis.transform.scale = glm::vec3(1.f);
		axis.transform.rotation = { 0.f, 0.f, 0.f };
		splineObjects.push_back(std::move(axis));

		v1.position = { 0.f,  -1000.f, 0.f };
		v1.color = { 0.f, 1.f, 0.f };
		v2.position = { 0.f,  1000.f, 0.f };
		v2.color = { 0.f, 1.f, 0.f };
		axisLine = { v1, v2 };

		axis = GameObject::createGameObject();
		axis.spline = Spline::createSplineFromVector(device, axisLine);
		axis.transform.translation = glm::vec3(0.f);
		axis.transform.scale = glm::vec3(1.f);
		axis.transform.rotation = { 0.f, 0.f, 0.f };
		splineObjects.push_back(std::move(axis));

		v1.position = { 0.f, 0.f,  -1000.f };
		v1.color = { 0.f, 0.f, 1.f };
		v2.position = { 0.f, 0.f, 1000.f };
		v2.color = { 0.f, 0.f, 1.f };
		axisLine = { v1, v2 };

		axis = GameObject::createGameObject();
		axis.spline = Spline::createSplineFromVector(device, axisLine);
		axis.transform.translation = glm::vec3(0.f);
		axis.transform.scale = glm::vec3(1.f);
		axis.transform.rotation = { 0.f, 0.f, 0.f };
		splineObjects.push_back(std::move(axis));
	}

}
