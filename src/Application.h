#pragma once

#include "Device.h"
#include "Descriptors.h"
#include "GameObject.h"
#include "Renderer.h"
#include "Window.h"

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
		Window window{ WIDTH, HEIGHT, "Что-нибудь доброе" };
		Device device{ window };
		Renderer renderer{ window, device };

		std::unique_ptr<DescriptorPool> globalPool{};
		std::vector<GameObject> gameObjects;
		std::vector<GameObject> splineObjects;

		std::unique_ptr<ImageTexture> textureImage;
	};
}