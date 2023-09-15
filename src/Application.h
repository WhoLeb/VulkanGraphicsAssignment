#include "Device.h"
#include "Window.h"
#include "Pipeline.h"


namespace assignment
{
	class Application
	{
	public:
		const int WIDTH = 800;
		const int HEIGHT = 600;

	public:
		void run();

	private:
		Window window{ WIDTH, HEIGHT, "Application" };

		Device device{ window };
		Pipeline pipeline{ device,  "./shaders/vert.spv", "./shaders/frag.spv", Pipeline::defaultPipelineConfigInfo(WIDTH, HEIGHT) };
	};
}