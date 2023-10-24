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
		
		int vertexCount = 39;
		std::vector<Line::Vertex> splineVertices(vertexCount);

		splineVertices[0].position = glm::vec3(0.098, 0.062, 0.f);
		splineVertices[1].position = glm::vec3(0.352, 0.073, 0.f);
		splineVertices[2].position = glm::vec3(0.422, 0.136, 0.f);
		splineVertices[3].position = glm::vec3(0.371, 0.085, 0.f);
		splineVertices[4].position = glm::vec3(0.449, 0.140, 0.f);
		splineVertices[5].position = glm::vec3(0.352, 0.187, 0.f);
		splineVertices[6].position = glm::vec3(0.379, 0.202, 0.f);
		splineVertices[7].position = glm::vec3(0.398, 0.202, 0.f);
		splineVertices[8].position = glm::vec3(0.266, 0.198, 0.f);
		splineVertices[9].position = glm::vec3(0.318, 0.345, 0.f);
		splineVertices[10].position = glm::vec3(0.402, 0.359, 0.f);
		splineVertices[11].position = glm::vec3(0.361, 0.425, 0.f);
		splineVertices[12].position = glm::vec3(0.371, 0.521, 0.f);
		splineVertices[13].position = glm::vec3(0.410, 0.491, 0.f);
		splineVertices[14].position = glm::vec3(0.410, 0.357, 0.f);
		splineVertices[15].position = glm::vec3(0.502, 0.482, 0.f);
		splineVertices[16].position = glm::vec3(0.529, 0.435, 0.f);
		splineVertices[17].position = glm::vec3(0.426, 0.343, 0.f);
		splineVertices[18].position = glm::vec3(0.449, 0.343, 0.f);
		splineVertices[19].position = glm::vec3(0.504, 0.335, 0.f);
		splineVertices[20].position = glm::vec3(0.664, 0.355, 0.f);
		splineVertices[21].position = glm::vec3(0.748, 0.208, 0.f);
		splineVertices[22].position = glm::vec3(0.738, 0.277, 0.f);
		splineVertices[23].position = glm::vec3(0.787, 0.308, 0.f);
		splineVertices[24].position = glm::vec3(0.748, 0.183, 0.f);
		splineVertices[25].position = glm::vec3(0.623, 0.081, 0.f);
		splineVertices[26].position = glm::vec3(0.557, 0.099, 0.f);
		splineVertices[27].position = glm::vec3(0.648, 0.116, 0.f);
		splineVertices[28].position = glm::vec3(0.598, 0.116, 0.f);
		splineVertices[29].position = glm::vec3(0.566, 0.195, 0.f);
		splineVertices[30].position = glm::vec3(0.584, 0.228, 0.f);
		splineVertices[31].position = glm::vec3(0.508, 0.083, 0.f);
		splineVertices[32].position = glm::vec3(0.457, 0.140, 0.f);
		splineVertices[33].position = glm::vec3(0.508, 0.130, 0.f);
		splineVertices[34].position = glm::vec3(0.625, 0.071, 0.f);
		splineVertices[35].position = glm::vec3(0.818, 0.093, 0.f);
		splineVertices[36].position = glm::vec3(0.951, 0.066, 0.f);
		splineVertices[37].position = glm::vec3(0.547, 0.081, 0.f);
		splineVertices[38].position = glm::vec3(0.098, 0.062, 0.f);
		for (auto& v : splineVertices)
			v.position.y *= -1;

		std::shared_ptr<Line> spline = Line::createLineFromVector(device, splineVertices);
		auto gameObject = GameObject::createGameObject("SplineBase");
		gameObject.line = spline;
		gameObject.transform.scale = glm::vec3(0.7f);
		lineObjects.push_back(std::move(gameObject));

		spline = Line::calculateCubicSplineEvenlySpaced(device, splineVertices, glm::vec3(1.f), glm::vec3(1.f), 20);
		gameObject = GameObject::createGameObject("CubicSpline");
		gameObject.line = spline;
		gameObject.transform.scale = glm::vec3(0.7f);
		lineObjects.push_back(std::move(gameObject));

		int BSplineSubdivisions = 100;
		int BSplineDegree = 4;
		splineVertices[0].color = { 1.f, 0.f, 1.f };
		spline = Line::calculateBSplineOpened(device, splineVertices, BSplineDegree, BSplineSubdivisions);
		gameObject = GameObject::createGameObject("B-Spline");
		gameObject.line = spline;
		gameObject.transform.scale = glm::vec3(0.7f);
		lineObjects.push_back(std::move(gameObject));
		bool rebuildSpline = true;

		std::vector<Line::Vertex> pointV(2);
		std::shared_ptr<Line> point = Line::createLineFromVector(device, pointV);


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
					rebuildSpline = true;
				}

				if (ImGui::CollapsingHeader("Vertex positions"))
				{
					for (int i = 0; i < vertexCount; i++)
					{
						if (ImGui::DragFloat3(
							std::format("Vertex {} position", i).c_str(),
							(float*)&splineVertices[i].position,
							0.01f))
						{
							rebuildSpline = true;
						}
					}
				}

				if (ImGui::InputInt("B-Spline subdivisions", &BSplineSubdivisions)) rebuildSpline = true;
				if (ImGui::InputInt("B-Spline degree", &BSplineDegree, 1, 3)) rebuildSpline = true;

				bool ph1, ph2, ph3;
				if (ImGui::Checkbox("Show base line", &ph1))
				{
					for (uint32_t i = 0; i < lineObjects.size(); i++)
						if (lineObjects[i].getName() == "SplineBase")
						{
							lineObjects[i].changeVisibility();
							break;
						}
				};
				if (ImGui::Checkbox("Show Cubic Spline", &ph2))
				{
					for (uint32_t i = 0; i < lineObjects.size(); i++)
						if (lineObjects[i].getName() == "CubicSpline")
						{
							lineObjects[i].changeVisibility();
							break;
						}
				};
				if (ImGui::Checkbox("Show B-Spline", &ph3))
				{
					for (uint32_t i = 0; i < lineObjects.size(); i++)
						if (lineObjects[i].getName() == "B-Spline")
						{
							lineObjects[i].changeVisibility();
							break;
						}
				};
				
				ImGui::End();
				ImGui::Render();

				if (rebuildSpline)
				{
					for (auto& v : splineVertices)
						v.color = { 1.f, 0.f, 0.f };
					spline = Line::createLineFromVector(device, splineVertices);
					lineObjects[3].line = spline;

					for (auto& v : splineVertices)
						v.color = { 0.f, 1.f, 0.f };
					spline = Line::calculateCubicSplineEvenlySpaced(device, splineVertices, glm::vec3(1.f), glm::vec3(1.f), 20);
					lineObjects[4].line = spline;

					for (auto& v : splineVertices)
						v.color = { 1.f, 0.f, 1.f };
					spline = Line::calculateBSplineOpened(device, splineVertices, BSplineDegree, BSplineSubdivisions);
					lineObjects[5].line = spline;
					rebuildSpline = false;
				}

				// render
				renderer.beginSwapChainRenderPass(commandBuffer);
				simpleRenderSystem.renderGameObjects(frameInfo, gameObjects);
				linesRenderSystem.renderLineObjects(frameInfo, lineObjects);

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
		//std::shared_ptr<Model> cube = Model::createModelFromFile(device, "./assets/meshes/cube.obj");
		//auto cubeObject = GameObject::createGameObject();
		//cubeObject.model = cube;
		//cubeObject.transform.scale = glm::vec3(0.005f);
		//cubeObject.transform.translation = { .6f, -.1f, 0.f };
		//gameObjects.push_back(std::move(cubeObject));


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
			.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, SwapChain::MAX_FRAMES_IN_FLIGHT)
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
