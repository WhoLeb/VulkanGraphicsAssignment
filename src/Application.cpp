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
		glm::vec3 lightDirection = glm::normalize(glm::vec3{ -1.f, -1.f, -1.f });
	};

	Application::Application()
	{
		globalPool = DescriptorPool::Builder(device)
			.setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, SwapChain::MAX_FRAMES_IN_FLIGHT)
			.build();
		loadGameObjects();
		
		textureImage = std::make_unique<ImageTexture>(device, "assets/textures/viking_room.png");
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
				//.writeImage(1, &descriptor)
				.build(globalDescriptorSets[i]);
		}

		SimpleRenderSystem simpleRenderSystem(device, renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout());

		auto viewerObject = GameObject::createGameObject();
		KeyboardMovementController cameraController{};

		Camera camera{};
		camera.setViewTarget({3.f, -2.f, -3.f}, glm::vec3(0.f));

		auto currentTime = std::chrono::high_resolution_clock::now();

		auto startTime = std::chrono::high_resolution_clock::now();

		while (!window.shouldClose())
		{
			glfwPollEvents();

			auto newTime = std::chrono::high_resolution_clock::now();
			float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
			currentTime = newTime;

			cameraController.moveInPlaneXZ(window.getGLFWwindow(), frameTime, viewerObject);
			camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

			float aspect = renderer.getAspectRatio();
			//camera.setOrthographicProjection(-aspect, -1, -1, aspect, 1, 3);
			camera.setPerspecitveProjection(glm::radians(45.f), aspect, 0.1f, 10.f);

			GlobalUbo ubo{};
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

				// update
				ubo.projectionView = camera.getProjection() * camera.getView();
				//gameObjects[0].transform.rotation = gameObjects[0].transform.rotation + glm::vec3{ 0.f, glm::mod<float>(frameTime * glm::radians(10.f), 360.f), 0.f };
				ubo.lightDirection = glm::normalize(glm::rotateY(ubo.lightDirection, frameTime * glm::radians(10.f)));
				if (std::chrono::duration<float, std::chrono::seconds::period>(newTime - startTime).count() > 1.f)
				{
					std::cout << ubo.lightDirection.x << " " << ubo.lightDirection.y << " " << ubo.lightDirection.z << "\n";
					startTime = newTime;
				}
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
		gameObject.color = { .1f, .1f, .1f };
		gameObject.transform.translation = { 1.f, -0.2f, 1.f };
		gameObject.transform.scale = glm::vec3(1.f);
		gameObject.transform.rotation = { glm::radians(90.f), 0.f, 0.f };

		gameObjects.push_back(std::move(gameObject));

		model = Model::createModelFromFile(device, "assets/meshes/axis.obj");
		auto axis = GameObject::createGameObject();
		axis.model = model;
		axis.color = glm::vec3(0.f);
		axis.transform.translation = glm::vec3(0.f);
		axis.transform.scale = glm::vec3(1.f);
		axis.transform.rotation = glm::vec3(0.f);
		gameObjects.push_back(std::move(axis));
		
		model = Model::createModelFromFile(device, "assets/meshes/arrow.obj");
		auto arrow = GameObject::createGameObject();
		arrow.model = model;
		arrow.transform.translation = { 2.f, -2.f, 2.f };
		arrow.transform.scale = glm::vec3(0.3f);
		
		gameObjects.push_back(std::move(arrow));
	}

}
