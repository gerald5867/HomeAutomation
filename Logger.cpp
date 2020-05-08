#include "Logger.h"
#include <vector>
#include "spdlog/include/spdlog/spdlog.h"
#include "spdlog/include/spdlog/sinks/sink.h"
#include "spdlog/include/spdlog/sinks/stdout_sinks.h"
#include "spdlog/include/spdlog/sinks/rotating_file_sink.h"


utils::Logger& utils::Logger::Instance() {
	static Logger inst;
	return inst;
}


utils::Logger::Logger() {
	std::vector<spdlog::sink_ptr> sinks;
	sinks.reserve(2);
	sinks.emplace_back(std::make_shared<spdlog::sinks::stdout_sink_st>());
	sinks.emplace_back(std::make_shared<spdlog::sinks::rotating_file_sink_st>("HomeAutomationLog", 1024 * 1024 * 50, 3));
	m_pLogger = std::make_shared<spdlog::logger>("homeAutomationLogger", std::begin(sinks), std::end(sinks));
	m_pLogger->set_level(spdlog::level::trace);
	spdlog::register_logger(m_pLogger);
}
