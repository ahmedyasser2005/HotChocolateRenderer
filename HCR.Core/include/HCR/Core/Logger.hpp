#pragma once

#include <cstdint>

namespace HCR
{
	enum class LogLevel : uint8_t
	{
		Info,
		Warn,
		Error,
	};

	class Logger final
	{
	public:
		Logger( const Logger& ) = delete;
		Logger& operator=( const Logger& ) = delete;
		Logger( Logger&& ) = delete;
		Logger& operator=( Logger&& ) = delete;

		static Logger& Get() noexcept
		{
			static Logger logger{};
			return logger;
		}

		void LogMessage( LogLevel level, const char* message ) noexcept;

	private:
		Logger() noexcept = default;
		~Logger() noexcept = default;

	};

	inline Logger& g_logger = Logger::Get();

#ifdef _DEBUG
#define LINFO(message) g_logger.LogMessage( LogLevel::Info, message )
#define LWARN(message) g_logger.LogMessage( LogLevel::Warn, message )
#define LERROR(message) g_logger.LogMessage( LogLevel::Error, message )
#else
#define LINFO(message) 
#define LWARN(message) 
#define LERROR(message) 
#endif

} // namespace HCR
