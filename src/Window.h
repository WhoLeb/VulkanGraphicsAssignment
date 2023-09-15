#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

#include "BaseClassDefines.h"

namespace assignment
{
	class Window
	{
	public:
		NO_COPY(Window);

		Window(int w, int h, std::string name);
		~Window();

		bool shouldClose() { return glfwWindowShouldClose(window); }
		void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

	private:
		void initWindow();

	private:
		GLFWwindow* window;

		int width, height;
		std::string windowName;
	};
}
