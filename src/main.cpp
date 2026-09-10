#include <OrcaEngine/Core/Application.hpp>
#include <iostream>

int main() 
{
	try 
	{
		Application app;
		app.Initialize();
		app.Run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}