#include "pch.hpp"
#include "Application.hpp"

int main( [[maybe_unused]] int argc, [[maybe_unused]] char** argv )
{
	HCR::Application app{ L"Hot Chocolate Renderer", 1280u, 720u };
	return app.Run();
}
