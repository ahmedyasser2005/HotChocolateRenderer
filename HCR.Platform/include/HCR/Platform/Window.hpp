#pragma once

#include "HCR/Core/Win32API.hpp"
#include <cstdint>
#include <optional>

namespace HCR
{
	class Window
	{
	public:
		Window( const Window& ) = delete;
		Window& operator=( const Window& ) = delete;
		Window( Window&& ) = delete;
		Window& operator=( Window&& ) = delete;

		static Window& Get() noexcept
		{
			static Window window{};
			return window;
		}

		void Start( const wchar_t* title, uint32_t width, uint32_t height ) noexcept;
		void Shutdown() const noexcept;

		[[nodiscard]] std::optional<int> ProcessMessages() noexcept;
		[[nodiscard]] HWND GetHandle() const noexcept { return m_hwnd; }
		[[nodiscard]] uint32_t GetWidth() const noexcept { return m_width; }
		[[nodiscard]] uint32_t GetHeight() const noexcept { return m_height; }

	private:
		Window() noexcept = default;
		~Window() noexcept = default;

		static LRESULT WindowProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam ) noexcept;

		HWND m_hwnd = nullptr;
		uint32_t m_width = 0u;
		uint32_t m_height = 0u;

	};

	inline Window& g_window = Window::Get();

} // namespace HCR
