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


}  // namespace HCR

#ifdef _DEBUG
	#define LINFO( message ) ::HCR::g_logger.LogMessage( ::HCR::LogLevel::Info, message )
	#define LWARN( message ) ::HCR::g_logger.LogMessage( ::HCR::LogLevel::Warn, message )
	#define LERROR( message ) ::HCR::g_logger.LogMessage( ::HCR::LogLevel::Error, message )
#else
	#define LINFO( message )
	#define LWARN( message )
	#define LERROR( message )
#endif
