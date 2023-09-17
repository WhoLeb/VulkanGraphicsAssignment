#pragma once

#include "Device.h"
#include "GameObject.h"
#include "Window.h"
#include "Renderer.h"

#include <memory>
#include <vector>

namespace assignment
{
	class Application
	{
	public:
		const int WIDTH = 800;
		const int HEIGHT = 600;

	public:
		Application();
		~Application();

		NO_COPY(Application);

	public:
		void run();

	private:
		void loadGameObjects();

	private:
		Window window{ WIDTH, HEIGHT, "Application" };
		Device device{ window };
		Renderer renderer{ window, device };

		std::vector<GameObject> gameObjects;
	};
}