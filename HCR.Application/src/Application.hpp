#pragma once

#include <cstdint>

namespace HCR
{
	class Application final
	{
	public:
		Application( const wchar_t* title, uint32_t width, uint32_t height );
		~Application();
		Application( const Application& ) = delete;
		Application& operator=( const Application& ) = delete;
		Application( Application&& ) = delete;
		Application& operator=( Application&& ) = delete;

		[[nodiscard]] int Run() noexcept;

	private:
		void Update( const float dt ) noexcept;
		void Render() noexcept;
	};

}  // namespace HCR
