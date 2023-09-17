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
		VkExtent2D getExtent() { return { uint32_t(width), uint32_t(height) }; }
		bool wasWindowResized() { return framebufferResized; }
		void resetWindowResizedFlag() { framebufferResized = false; }

		void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

	private:
		static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
		void initWindow();

	private:
		GLFWwindow* window;

		int width, height;
		bool framebufferResized = false;
		std::string windowName;
	};
}
