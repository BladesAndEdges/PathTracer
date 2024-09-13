#include<imgui.h>

#include <Windows.h>

#include "Application.h"

// --------------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE, HINSTANCE, PSTR, int)
{
	Application app;
	app.Run();

	return 0;
}