#include "Application.h"

#include "Camera.h"
#include "SimpleRenderSystem.h"
#include "KeyboardMovementController.h"

#include <stdexcept>
#include <chrono>
#include <array>

namespace
{
	void sierpinski(
		std::vector<assignment::Model::Vertex>& vertices,
		int depth,
		glm::vec2 top,
		glm::vec2 left,
		glm::vec2 right
	)
	{
		if (depth <= 0)
		{
			vertices.push_back({ {top, 0.f}, glm::vec3(0.5f) });
			vertices.push_back({ {right, 0.f}, glm::vec3(0.5f) });
			vertices.push_back({ {left, 0.f}, glm::vec3(0.5f) });
			return;
		}
		assignment::Model::Vertex leftTop = { {(left + top) / 2.f, 0.f}, {glm::vec3(0.4f)}};
		assignment::Model::Vertex rightTop = { {(right + top) / 2.f, 0.f}, {glm::vec3(0.4f)}};
		assignment::Model::Vertex leftRight = { {(left + right) / 2.f, 0.f}, {glm::vec3(0.4f)}};
		sierpinski(vertices, depth - 1, left, leftRight.position, leftTop.position);
		sierpinski(vertices, depth - 1, leftRight.position, right, rightTop.position);
		sierpinski(vertices, depth - 1, leftTop.position, rightTop.position, top);
	}
}

namespace assignment
{
	Application::Application()
	{
		loadGameObjects();
	}

	Application::~Application() {}

	void Application::run()
	{
		SimpleRenderSystem simpleRenderSystem(device, renderer.getSwapChainRenderPass());

		auto viewerObject = GameObject::createGameObject();
		KeyboardMovementController cameraController{};

		Camera camera{};
		camera.setViewDirection(glm::vec3(0.f), glm::vec3(0.5f, 0.f, 1.f));

		auto currentTime = std::chrono::high_resolution_clock::now();

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

			if (auto commandBuffer = renderer.beginFrame())
			{
				renderer.beginSwapChainRenderPass(commandBuffer);
				simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects, camera);
				renderer.endSwapChainRenderPass(commandBuffer);
				renderer.endFrame();
			}
		}

		vkDeviceWaitIdle(device.device());
	}

	void Application::loadGameObjects()
	{
		std::shared_ptr<Model> model = Model::createModelFromFile(device, "assets/meshes/viking_room.obj");

		auto gameObject = GameObject::createGameObject();
		gameObject.model = model;
		gameObject.color = { .1f, .1f, .1f };
		gameObject.transform.translation = { 0.f, .0f, 1.5f };
		gameObject.transform.scale = glm::vec3(0.5f);
		gameObject.transform.rotation = { glm::radians(90.f), 0.f, 0.f };

		gameObjects.push_back(std::move(gameObject));
	}

}
