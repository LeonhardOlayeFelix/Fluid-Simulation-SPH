#pragma once

#include<spdlog/spdlog.h>
#include <memory.h>

class Log
{
private:
	static std::shared_ptr<spdlog::logger> s_FluidSimulationLogger;

public:
	static void init();

	inline static std::shared_ptr<spdlog::logger>& Logger(){ return s_FluidSimulationLogger; };

};

#ifdef _DEBUG
	#define SPHERROR(...)    ::Log::Logger()->error(__VA_ARGS__)
	#define SPHWARN(...)     ::Log::Logger()->warn(__VA_ARGS__)
	#define SPHINFO(...)     ::Log::Logger()->info(__VA_ARGS__)
	#define SPHTRACE(...)    ::Log::Logger()->trace(__VA_ARGS__)
	#define SPHCRITICAL(...) ::Log::Logger()->critical(__VA_ARGS__)
#else
	#define SPHERROR(...)
	#define SPHWARN(...)
	#define SPHINFO(...)
	#define SPHTRACE(...)
	#define SPHCRITICAL(...)
#endif

