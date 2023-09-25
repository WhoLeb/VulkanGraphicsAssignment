#include "Application.h"

#include "Camera.h"
#include "SimpleRenderSystem.h"
#include "KeyboardMovementController.h"

#include "glm/gtx/rotate_vector.hpp"

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
			camera.setOrthographicProjection(-aspect, -1, -1, aspect, 1, 30);
			//camera.setPerspecitveProjection(glm::radians(45.f), aspect, 0.1f, 10.f);

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
				renderer.endSwapChainRenderPass(commandBuffer);
				renderer.endFrame();
			}
		}

		vkDeviceWaitIdle(device.device());
	}

	void Application::loadGameObjects()
	{
		std::shared_ptr<Model> model = Model::createModelFromFile(device, "assets/meshes/my_cube.obj");

		auto gameObject = GameObject::createGameObject();
		gameObject.model = model;
		gameObject.transform.translation = { 0.3f, -0.3f, -0.3f };
		gameObject.transform.scale = glm::vec3(.3f);
		gameObject.transform.rotation = { 0.f, 0.f, 0.f };
		gameObjects.push_back(std::move(gameObject));

		model = Model::createModelFromFile(device, "assets/meshes/axis.obj");
		auto axis = GameObject::createGameObject();
		axis.model = model;
		axis.transform.translation = glm::vec3(0.f);
		axis.transform.scale = glm::vec3(1.f);
		axis.transform.rotation = { glm::radians(90.f), 0.f, 0.f };
		gameObjects.push_back(std::move(axis));
	}

}
