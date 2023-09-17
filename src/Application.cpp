#include "Application.h"
#include "SimpleRenderSystem.h"

#include <stdexcept>
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
			vertices.push_back({ {top}, glm::vec3(0.5f) });
			vertices.push_back({ {right}, glm::vec3(0.5f) });
			vertices.push_back({ {left}, glm::vec3(0.5f) });
			return;
		}
		assignment::Model::Vertex leftTop = { {(left + top) / 2.f}, {glm::vec3(0.4f)}};
		assignment::Model::Vertex rightTop = { {(right + top) / 2.f}, {glm::vec3(0.4f)}};
		assignment::Model::Vertex leftRight = { {(left + right) / 2.f}, {glm::vec3(0.4f)}};
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
		while (!window.shouldClose())
		{
			glfwPollEvents();

			if (auto commandBuffer = renderer.beginFrame())
			{
				renderer.beginSwapChainRenderPass(commandBuffer);
				simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects);
				renderer.endSwapChainRenderPass(commandBuffer);
				renderer.endFrame();
			}
		}

		vkDeviceWaitIdle(device.device());
	}

	void Application::loadGameObjects()
	{
		std::vector<Model::Vertex> vertices = {
			{{0.f, -0.5f}, {0.f, 0.f, 0.f}},
			{{0.5f, 0.5f}, {0.f, 1.f, 0.f}},
			{{-0.5f, 0.5f}, {0.f, 0.f, 1.f}}
		};
		auto model = std::make_shared<Model>(device, vertices);

		auto triangle = GameObject::createGameObject();
		triangle.model = model;
		triangle.color = { .1f, .1f, .1f };
		triangle.transform2d.translation.x = .2f;

		gameObjects.push_back(std::move(triangle));
	}

}
