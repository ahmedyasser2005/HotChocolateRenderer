#include "pch.hpp"
#include "Platform/Input.hpp"

[[nodiscard]] bool HCR::Input::IsKeyPressed( uint8_t key ) const noexcept
{
	return m_keys[key];
}

[[nodiscard]] bool HCR::Input::IsKeyTriggered( uint8_t key ) const noexcept
{
	return m_keys[key] && !m_prevKeys[key];
}

[[nodiscard]] bool HCR::Input::IsMouseDown( MouseButton button ) const noexcept
{
	return m_mouseButtons[button];
}

[[nodiscard]] bool HCR::Input::IsMouseTriggered( MouseButton button ) const noexcept
{
	return m_mouseButtons[button] && !m_prevMouseButtons[button];
}

void HCR::Input::Reset() noexcept
{
	m_prevKeys = m_keys;
	m_prevMouseButtons = m_mouseButtons;
	m_mouseDeltaX = 0;
	m_mouseDeltaY = 0;
	m_wheelDelta = 0;
}

void HCR::Input::OnKeyDown( uint8_t key ) noexcept
{
	m_keys[key] = true;
}

void HCR::Input::OnKeyUp( uint8_t key ) noexcept
{
	m_keys[key] = false;
}

void HCR::Input::OnMouseMove( int32_t x, int32_t y ) noexcept
{
	m_mouseDeltaX = x - m_mouseX;
	m_mouseDeltaY = y - m_mouseY;
	m_mouseX = x;
	m_mouseY = y;
}

void HCR::Input::OnMouseDown( MouseButton button ) noexcept
{
	m_mouseButtons.set( button );
}

void HCR::Input::OnMouseUp( MouseButton button ) noexcept
{
	m_mouseButtons.reset( button );
}

void HCR::Input::OnMouseWheel( int16_t delta ) noexcept
{
	m_wheelDelta += delta;
}
