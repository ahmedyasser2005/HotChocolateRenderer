#pragma once

#include <chrono>

namespace HCR
{
	class Timer final
	{
	public:
		Timer() noexcept = default;

		// Resets the timer and returns elapsed time since the last Mark() call
		[[nodiscard]] float Mark() noexcept
		{
			const auto now = std::chrono::steady_clock::now();
			const std::chrono::duration<float> frameTime = now - m_lastTime;
			m_lastTime = now;
			return frameTime.count();
		}

		// Returns elapsed time since the last Mark() call WITHOUT resetting the timer
		[[nodiscard]] float Peek() const noexcept
		{
			const std::chrono::duration<float> elapsedTime = std::chrono::steady_clock::now() - m_lastTime;
			return elapsedTime.count();
		}

	private:
		std::chrono::steady_clock::time_point m_lastTime = std::chrono::steady_clock::now();
	};

}  // namespace HCR
