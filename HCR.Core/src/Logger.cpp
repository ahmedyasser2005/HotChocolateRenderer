#include "pch.hpp"
#include "Core/Logger.hpp"

void HCR::Logger::LogMessage( LogLevel level, const char* message ) noexcept
{
	// \033[32m = Green, \033[33m = Yellow, \033[31m = Red, \033[0m = Reset

	switch( level )
	{
	case LogLevel::Info:
		std::println( "\033[32m[Client][INFO] {}\033[0m", message );
		break;
	case LogLevel::Warn:
		std::println( "\033[33m[Client][WARN] {}\033[0m", message );
		break;
	case LogLevel::Error:
		std::println( "\033[31m[Client][ERROR] {}\033[0m", message );
		break;
	}
}
