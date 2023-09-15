#include "Application.h"

#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main()
{
	assignment::Application app;

	try
	{
		app.run();
	}
	catch (std::exception& err)
	{
		std::cerr << err.what() << "\n";
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;

}