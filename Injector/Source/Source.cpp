#include "Application/CApplication.h"
#include <iostream>

// If anyone is seeing this then I hope you enjoy my none commented code :)

int main(int argc, char** argv) {
	for (size_t i = 0; i < argc; ++i) {
		if (argv[i] == "-console") {
			AllocConsole();
			FILE* f;
			freopen_s(&f, "CONOUT$", "w", stdout);
		}
	}

	CApplication Application("Injector", 950, 600);

	Application.InitializePlatform();
	Application.Initialize();
	Application.InitializeOverlay("file:///injector.html");

	spdlog::info("Finished Initialization");

	Application.Start();

	return 0;
}