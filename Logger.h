#pragma once
#include <memory>
#include "spdlog/include/spdlog/spdlog.h"

namespace utils {
class Logger final {
public:
	static Logger& Instance();
	std::shared_ptr<spdlog::logger>& Get() { return m_pLogger; }
	void Flush() { m_pLogger->flush(); }
private:
	Logger();
	std::shared_ptr<spdlog::logger> m_pLogger = nullptr;
};
}

#define LOG_TRACE(...) utils::Logger::Instance().Get()->trace(__VA_ARGS__)
#define LOG_DEBUG(...) utils::Logger::Instance().Get()->debug(__VA_ARGS__)
#define LOG_INFO(...) utils::Logger::Instance().Get()->info(__VA_ARGS__)
#define LOG_WARNING(...) utils::Logger::Instance().Get()->warn(__VA_ARGS__)
#define LOG_ERROR(...) utils::Logger::Instance().Get()->error(__VA_ARGS__)
#define LOG_CRITICAL(...) utils::Logger::Instance().Get()->critical(__VA_ARGS__)