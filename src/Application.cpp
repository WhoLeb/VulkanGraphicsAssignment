#include "Application.h"

#include "Camera.h"
#include "SimpleRenderSystem.h"
#include "LinesRenderSystem.h"
#include "Line.h"
#include "KeyboardMovementController.h"
#include "Model.h"

#include "glm/gtx/rotate_vector.hpp"
#include <Eigen/Dense>

#include <stdexcept>
#include <chrono>
#include <array>
#include <iostream>

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
		LinesRenderSystem linesRenderSystem(device, renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout());

		auto viewerObject = GameObject::createGameObject(); 
		viewerObject.transform.translation = { 0.f, 0.f, -1.f };
		KeyboardMovementController cameraController{};

		Camera camera{};
		camera.setViewTarget({0.f, 0.f, -1.f}, {0.f, 0.f, 0.f});

		auto currentTime = std::chrono::high_resolution_clock::now();

		auto startTime = std::chrono::high_resolution_clock::now();
		
		auto lastKeystroke = std::chrono::high_resolution_clock::now();

		uint32_t selectedVertex = 0;

		{
			auto keysAvailable = std::chrono::duration<float, std::chrono::seconds::period>(currentTime > lastKeystroke).count() > 0.1f;
			if (keysAvailable && glfwGetKey(window.getGLFWwindow(), GLFW_KEY_U) == GLFW_PRESS)
			{
			}
		}

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
				GlobalUbo ubo{};
				ubo.projectionView = camera.getProjection() * camera.getView();
				ubo.lightDirection = glm::vec3(1.f, -1.f, -1.f);

				uboBuffers[frameIndex]->writeToBuffer(&ubo);
				uboBuffers[frameIndex]->flush(); 

				// render
				renderer.beginSwapChainRenderPass(commandBuffer);
				simpleRenderSystem.renderGameObjects(frameInfo, gameObjects);
				linesRenderSystem.renderSplineObjects(frameInfo, lineObjects);
				renderer.endSwapChainRenderPass(commandBuffer);
				renderer.endFrame();
			}
		}

		vkDeviceWaitIdle(device.device());
	}

	void Application::loadGameObjects()
	{
		std::shared_ptr<Model> model = Model::createModelFromFile(device, "assets/meshes/my_cube.obj");

		std::vector<Line::Vertex> vertices(5);
		for (auto& v : vertices)
			v.color = { 1.f, 0.f, 0.f };
		vertices[0].position = { 0.f,  0.f, 5.f };
		vertices[1].position = { 1.f,  1.f, -3.f };
		vertices[2].position = { 2.f, -1.f, 2.f };
		vertices[3].position = { 3.f,  0.f, 0.f };
		vertices[4].position = { 4.f,  -2.f, 3.f };

		std::shared_ptr<Line> spline = Line::createLineFromVector(device, vertices);
		auto gameObject = GameObject::createGameObject();
		gameObject.line = spline;
		gameObject.transform.translation = { 0.f, 0.f, 0.f };
		gameObject.transform.scale = glm::vec3(0.3f);
		gameObject.transform.rotation = { 0.f, 0.f, 0.f };
		lineObjects.push_back(std::move(gameObject));

		for (auto& v : vertices)
			v.color = { 0.f, 1.f, 0.f };

		spline = Line::calculateSplineEvenlySpaced(device, vertices, glm::vec3(1.f), glm::vec3(1.f), 20);
		gameObject = GameObject::createGameObject();
		gameObject.line = spline;
		gameObject.transform.translation = { 0.f, 0.f, 0.f };
		gameObject.transform.scale = glm::vec3(0.3f);
		gameObject.transform.rotation = { 0.f, 0.f, 0.f };
		lineObjects.push_back(std::move(gameObject));

		Line::Vertex v1, v2;
		v1.position = { -1000.f, 0.f, 0.f };
		v1.color = { 1.f, 0.f, 0.f };
		v2.position = { 1000.f, 0.f, 0.f };
		v2.color = { 1.f, 0.f, 0.f };
		std::vector<Line::Vertex> axisLine = { v1, v2 };

		auto axis = GameObject::createGameObject();
		axis.line = Line::createLineFromVector(device, axisLine);
		axis.transform.translation = glm::vec3(0.f);
		axis.transform.scale = glm::vec3(1.f);
		axis.transform.rotation = { 0.f, 0.f, 0.f };
		lineObjects.push_back(std::move(axis));

		v1.position = { 0.f,  -1000.f, 0.f };
		v1.color = { 0.f, 1.f, 0.f };
		v2.position = { 0.f,  1000.f, 0.f };
		v2.color = { 0.f, 1.f, 0.f };
		axisLine = { v1, v2 };

		axis = GameObject::createGameObject();
		axis.line = Line::createLineFromVector(device, axisLine);
		axis.transform.translation = glm::vec3(0.f);
		axis.transform.scale = glm::vec3(1.f);
		axis.transform.rotation = { 0.f, 0.f, 0.f };
		lineObjects.push_back(std::move(axis));

		v1.position = { 0.f, 0.f,  -1000.f };
		v1.color = { 0.f, 0.f, 1.f };
		v2.position = { 0.f, 0.f, 1000.f };
		v2.color = { 0.f, 0.f, 1.f };
		axisLine = { v1, v2 };

		axis = GameObject::createGameObject();
		axis.line = Line::createLineFromVector(device, axisLine);
		axis.transform.translation = glm::vec3(0.f);
		axis.transform.scale = glm::vec3(1.f);
		axis.transform.rotation = { 0.f, 0.f, 0.f };
		lineObjects.push_back(std::move(axis));
	}

}
