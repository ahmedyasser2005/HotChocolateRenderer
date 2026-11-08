#pragma once

#include <bitset>
#include <cstdint>

namespace HCR
{
	enum MouseButton : uint8_t
	{
		Left = 0u,
		Right,
		Middle,
		Count,  // Number of mouse buttons
	};

	class Input final
	{
		friend class Window;

	public:
		Input( const Input& ) = delete;
		Input& operator=( const Input& ) = delete;
		Input( Input&& ) = delete;
		Input& operator=( Input&& ) = delete;

		static Input& Get()
		{
			static Input input{};
			return input;
		}

		[[nodiscard]] bool IsKeyPressed( uint8_t key ) const noexcept;
		[[nodiscard]] bool IsKeyTriggered( uint8_t key ) const noexcept;

		[[nodiscard]] bool IsMouseDown( MouseButton button ) const noexcept;
		[[nodiscard]] bool IsMouseTriggered( MouseButton button ) const noexcept;

		[[nodiscard]] int32_t GetMouseX() const noexcept { return m_mouseX; }
		[[nodiscard]] int32_t GetMouseY() const noexcept { return m_mouseY; }
		[[nodiscard]] int32_t GetMouseDeltaX() const noexcept { return m_mouseDeltaX; }
		[[nodiscard]] int32_t GetMouseDeltaY() const noexcept { return m_mouseDeltaY; }
		[[nodiscard]] int16_t GetWheelDelta() const noexcept { return m_wheelDelta; }

		void Reset() noexcept;

	private:
		Input() noexcept = default;
		~Input() noexcept = default;

		void OnKeyDown( uint8_t key ) noexcept;
		void OnKeyUp( uint8_t key ) noexcept;
		void OnMouseMove( int32_t x, int32_t y ) noexcept;
		void OnMouseDown( MouseButton button ) noexcept;
		void OnMouseUp( MouseButton button ) noexcept;
		void OnMouseWheel( int16_t delta ) noexcept;

	private:
		std::bitset<256> m_keys{}, m_prevKeys{};
		std::bitset<MouseButton::Count> m_mouseButtons{}, m_prevMouseButtons{};
		int32_t m_mouseX = 0, m_mouseY = 0, m_mouseDeltaX = 0, m_mouseDeltaY = 0;
		int16_t m_wheelDelta = 0;
	};

	inline Input& g_input = Input::Get();

}  // namespace HCR
