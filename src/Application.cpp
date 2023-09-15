#include "Application.h"


namespace assignment
{
	void Application::run()
	{
		while (!window.shouldClose())
		{
			glfwPollEvents();
		}
	}
}
