#include<imgui.h>

#include <Windows.h>

#include <cstdlib>
#include <ctime>

#include "Application.h"

// --------------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE, HINSTANCE, PSTR, int)
{
	std::srand((int)std::time(nullptr));

	Application app;
	app.Run();

	return 0;
}