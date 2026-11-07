#include "pch.hpp"
#include "Application.hpp"
#include "HCR/Platform/Window.hpp"
#include "HCR/Platform/Input.hpp"
#include "HCR/Core/Timer.hpp"
#include "HCR/Core/Logger.hpp"
#include "HCR/Graphics/Graphics.hpp"

HCR::Application::Application( const wchar_t* title, uint32_t width, uint32_t height )
{
	LINFO( "Starting Application..." );

	g_window.Start( title, width, height );
	g_gfx.Start( g_window.GetHandle(), width, height );

	LINFO( "Application started." );
}

HCR::Application::~Application()
{
	LINFO( "Shutting Application..." );

	g_gfx.Shutdown();
	g_window.Shutdown();

	LINFO( "Application shut." );
}

int HCR::Application::Run() noexcept
{
	LINFO( "Running application..." );

	std::optional<int> exitCode = std::nullopt;
	Timer timer{};

	while( !(exitCode = g_window.ProcessMessages()).has_value() )
	{
		const float dt = std::min( timer.Mark(), 0.1f );
		Update( dt );
		Render();
	}

	LINFO( "Stopped application." );
	return exitCode.value();
}

void HCR::Application::Update( [[maybe_unused]] const float dt ) noexcept
{

	g_input.Reset();
}

void HCR::Application::Render() noexcept
{
	g_gfx.Render();
}
