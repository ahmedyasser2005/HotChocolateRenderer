#include "pch.hpp"
#include "Application.hpp"
#include "HCR/Core/Logger.hpp"
#include "HCR/Core/Timer.hpp"
#include "HCR/Graphics/Graphics.hpp"
#include "HCR/Platform/Input.hpp"
#include "HCR/Platform/Window.hpp"

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

#pragma region Testing CUDA kernel
///////////////////////////////////
#pragma comment( lib, "cudart.lib" )
#include "HCR/CUDA/kernel.cuh"

static void TestCUDAGradientKernel()
{
	constexpr uint32_t width = 1920;
	constexpr uint32_t height = 1080;
	uint8_t* pixels = (uint8_t*)malloc( width * height * static_cast<size_t>( 4 ) );

	HCR::make_gradient( &pixels, width, height );

	LINFO( "Saving image..." );

	{
		std::ofstream file( "gradient.ppm", std::ios::binary );
		if( !file )
		{
			assert( file );
			return;
		}

		file << "P6\n"
			 << width << ' ' << height << "\n255\n";

		for( size_t i = 0; i < width * height; ++i )
		{
			file.put( static_cast<char>( pixels[i * 4] ) );      // R
			file.put( static_cast<char>( pixels[i * 4 + 1] ) );  // G
			file.put( static_cast<char>( pixels[i * 4 + 2] ) );  // B
		}
	}

	LINFO( "Saved Image!" );
}
///////////////////////////////////
#pragma endregion

int HCR::Application::Run() noexcept
{
	LINFO( "Running application..." );

#pragma region Testing CUDA kernel
	///////////////////////////////////
	TestCUDAGradientKernel();
	///////////////////////////////////
#pragma endregion

	std::optional<int> exitCode = std::nullopt;
	Timer timer{};

	while( !( exitCode = g_window.ProcessMessages() ).has_value() )
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
