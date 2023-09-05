#pragma once

#include "Core/App.h"

int main(int argc, char **argv) {
	Pivot::CreateApp(std::span(argv, argc))->Run();
	
	return EXIT_SUCCESS;
}
