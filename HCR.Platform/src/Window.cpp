#include "pch.hpp"
#include "Platform/Window.hpp"
#include "Platform/Input.hpp"
#include "HCR/Core/Logger.hpp"

namespace
{
	static constexpr auto g_className = L"WindowClass";
	static const auto g_hInstance = GetModuleHandleW( nullptr );
}

//extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

LRESULT HCR::Window::WindowProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam ) noexcept
{
	//if( ImGui_ImplWin32_WndProcHandler( hwnd, msg, wParam, lParam ) )
	//	return true;

	switch( msg )
	{
	case WM_KEYDOWN:	 g_input.OnKeyDown( static_cast<uint8_t>(wParam) ); break;
	case WM_KEYUP:		 g_input.OnKeyUp( static_cast<uint8_t>(wParam) ); break;

	case WM_SYSKEYDOWN:	 g_input.OnKeyDown( static_cast<uint8_t>(wParam) );
		return DefWindowProcW( hwnd, msg, wParam, lParam );
	case WM_SYSKEYUP:    g_input.OnKeyUp( static_cast<uint8_t>(wParam) );
		return DefWindowProcW( hwnd, msg, wParam, lParam );

	case WM_MOUSEMOVE:	 g_input.OnMouseMove( GET_X_LPARAM( lParam ), GET_Y_LPARAM( lParam ) ); break;
	case WM_LBUTTONDOWN: g_input.OnMouseDown( MouseButton::Left ); break;
	case WM_LBUTTONUP:   g_input.OnMouseUp( MouseButton::Left ); break;
	case WM_RBUTTONDOWN: g_input.OnMouseDown( MouseButton::Right ); break;
	case WM_RBUTTONUP:   g_input.OnMouseUp( MouseButton::Right ); break;
	case WM_MBUTTONDOWN: g_input.OnMouseDown( MouseButton::Middle ); break;
	case WM_MBUTTONUP:   g_input.OnMouseUp( MouseButton::Middle ); break;
	case WM_MOUSEWHEEL:  g_input.OnMouseWheel( GET_WHEEL_DELTA_WPARAM( wParam ) ); break;

	case WM_DESTROY:     PostQuitMessage( 0 ); break;

	default:
	return DefWindowProcW( hwnd, msg, wParam, lParam );
	}
	return 0;
}

void HCR::Window::Start( const wchar_t* title, uint32_t width, uint32_t height ) noexcept
{
	LINFO( "Starting Window..." );

	m_width = width;
	m_height = height;

	WNDCLASSEX wc = {
		.cbSize = sizeof( WNDCLASSEX ),
		.style = CS_HREDRAW | CS_VREDRAW,
		.lpfnWndProc = WindowProc,
		.cbClsExtra = 0,
		.cbWndExtra = 0,
		.hInstance = g_hInstance,
		.hIcon = nullptr,
		.hCursor = LoadCursorW( nullptr, IDC_ARROW ),
		.hbrBackground = nullptr,
		.lpszMenuName = nullptr,
		.lpszClassName = g_className,
		.hIconSm = nullptr,
	};

	RegisterClassExW( &wc );

	RECT rc = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };
	AdjustWindowRect( &rc, WS_OVERLAPPEDWINDOW, FALSE );

	m_hwnd = CreateWindowExW( 0u, // WS_EX_TOPMOST, // Just to prevent the console messing up my initial mouse pos
							  g_className, title,
							  WS_OVERLAPPEDWINDOW,
							  CW_USEDEFAULT, CW_USEDEFAULT,
							  rc.right - rc.left,
							  rc.bottom - rc.top,
							  nullptr,
							  nullptr,
							  g_hInstance,
							  this );

	assert( m_hwnd );

	ShowWindow( m_hwnd, SW_SHOW );
	UpdateWindow( m_hwnd );

	LINFO( "Window started." );
}

void HCR::Window::Shutdown() const noexcept
{
	LINFO( "Shutting Window..." );

	if( m_hwnd )
	{
		DestroyWindow( m_hwnd );
	}
	UnregisterClassW( g_className, g_hInstance );

	LINFO( "Window shut." );
}

std::optional<int> HCR::Window::ProcessMessages() noexcept
{
	MSG msg;
	while( PeekMessageW( &msg, nullptr, 0u, 0u, PM_REMOVE ) )
	{
		if( msg.message == WM_QUIT )
		{
			return static_cast<int>(msg.wParam);
		}

		TranslateMessage( &msg );
		DispatchMessageW( &msg );
	}

	return std::nullopt;
}
