#include "Log.h"
#include "spdlog/sinks/stdout_color_sinks.h"

std::shared_ptr<spdlog::logger> Log::s_FluidSimulationLogger;

void Log::init() {
	spdlog::set_pattern("%^[%T] %n: %v%$");
	s_FluidSimulationLogger = spdlog::stdout_color_mt("FluidSimulation");
	s_FluidSimulationLogger->set_level(spdlog::level::trace);
}
