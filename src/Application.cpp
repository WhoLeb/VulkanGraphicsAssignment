#include "Application.h"

#include "Camera.h"
#include "SimpleRenderSystem.h"
#include "LinesRenderSystem.h"
#include "Line.h"
#include "KeyboardMovementController.h"
#include "Model.h"

#include "glm/gtx/rotate_vector.hpp"
#include <Eigen/Dense>

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

#include <stdexcept>
#include <chrono>
#include <array>
#include <iostream>
#include <format>

namespace assignment
{
	struct GlobalUbo {
		glm::mat4 projectionMatrix;
		glm::mat4 viewMatrix;
		glm::vec3 lightDirection = glm::normalize(glm::vec3{ 1.f, -1.f, -1.f });
	};

	Application::Application()
	{
		globalPool = DescriptorPool::Builder(device)
			.setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100)
			.build();
		loadGameObjects();
		
		textureImage = std::make_unique<ImageTexture>(device, "assets/textures/white.png");
	}

	Application::~Application() {}

	void Application::run()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		initImGui();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

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
		
		int vertexCount = 5;
		std::vector<Line::Vertex> splineVertices(5);

		splineVertices[0].position = { 0.f,  0.f, 5.f };
		splineVertices[1].position = { 1.f,  1.f, -3.f };
		splineVertices[2].position = { 2.f, -1.f, 2.f };
		splineVertices[3].position = { 3.f,  0.f, 0.f };
		splineVertices[4].position = { 4.f,  -2.f, 3.f };

		std::shared_ptr<Line> spline = Line::createLineFromVector(device, splineVertices);
		auto gameObject = GameObject::createGameObject();
		gameObject.line = spline;
		gameObject.transform.scale = glm::vec3(0.3f);
		lineObjects.push_back(std::move(gameObject));

		spline = Line::calculateSplineEvenlySpaced(device, splineVertices, glm::vec3(1.f), glm::vec3(1.f), 20);
		gameObject = GameObject::createGameObject();
		gameObject.line = spline;
		gameObject.transform.scale = glm::vec3(0.3f);
		lineObjects.push_back(std::move(gameObject));

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
			camera.setPerspecitveProjection(glm::radians(45.f), aspect, 0.1f, 100.f);

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
				ubo.projectionMatrix = camera.getProjection();
				ubo.viewMatrix = camera.getView();
				ubo.lightDirection = glm::vec3(1.f, -1.f, -1.f);

				uboBuffers[frameIndex]->writeToBuffer(&ubo);
				uboBuffers[frameIndex]->flush(); 

				ImGui_ImplVulkan_NewFrame();
				ImGui_ImplGlfw_NewFrame();
				ImGui::NewFrame();

				//ImGui::ShowDemoWindow();
				ImGui::Begin("Frame time");
				ImGui::Text(std::to_string(frameTime).c_str());
				ImGui::End();
				
				ImGui::Begin("Spline vertex controls");
				if (ImGui::InputInt("Vertex count", &vertexCount))
				{
					if (vertexCount < 2)
					{
						ImGui::Text("Wrong size, setting it to 2");
						vertexCount = 2;
					}
					splineVertices.resize(vertexCount);
				}

				for (int i = 0; i < vertexCount; i++)
					ImGui::InputFloat3(std::format("Vertex {} position", i).c_str(), (float*)&splineVertices[i].position);

				ImGui::End();
				ImGui::Render();

				for (auto& v : splineVertices)
					v.color = { 1.f, 0.f, 0.f };
				spline = Line::createLineFromVector(device, splineVertices);
				lineObjects[3].line = spline;

				for (auto& v : splineVertices)
					v.color = { 0.f, 1.f, 0.f };
				spline = Line::calculateSplineEvenlySpaced(device, splineVertices, glm::vec3(1.f), glm::vec3(1.f), 20);
				lineObjects[4].line = spline;

				// render
				renderer.beginSwapChainRenderPass(commandBuffer);
				simpleRenderSystem.renderGameObjects(frameInfo, gameObjects);
				linesRenderSystem.renderSplineObjects(frameInfo, lineObjects);

				ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
				renderer.endSwapChainRenderPass(commandBuffer);
				renderer.endFrame();
			}
		}
		vkDeviceWaitIdle(device.device());

		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void Application::loadGameObjects()
	{
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


	void Application::initImGui()
	{
		IMGUI_CHECKVERSION();

		imguiPool = DescriptorPool::Builder(device)
			.addPoolSize(VK_DESCRIPTOR_TYPE_SAMPLER, SwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, SwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, SwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, SwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, SwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, SwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, SwapChain::MAX_FRAMES_IN_FLIGHT)
			.setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
			.build();

		ImGui_ImplGlfw_InitForVulkan(window.getGLFWwindow(), true);
		ImGui_ImplVulkan_InitInfo initInfo{};
		initInfo.Instance = device.getInstance();
		initInfo.PhysicalDevice = device.getPhysicalDevice();
		initInfo.Device = device.device();
		initInfo.QueueFamily = device.findPhysicalQueueFamilies().graphicsFamily;
		initInfo.Queue = device.graphicsQueue();
		initInfo.DescriptorPool = imguiPool->getDescriptorPool();
		initInfo.MinImageCount = SwapChain::MAX_FRAMES_IN_FLIGHT;
		initInfo.ImageCount = SwapChain::MAX_FRAMES_IN_FLIGHT;
		initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		ImGui_ImplVulkan_Init(&initInfo, renderer.getSwapChainRenderPass());

		VkCommandBuffer commandBuffer = device.beginSingleTimeCommands();
		ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
		device.endSingleTimeCommands(commandBuffer);
		vkDeviceWaitIdle(device.device());
		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}

}
