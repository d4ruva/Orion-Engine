#include "OLog.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace Orion
{

std::shared_ptr<spdlog::logger> OLog::s_Logger = nullptr;

void OLog::Init()
{
    spdlog::set_pattern("[%T] [%^%l%$] %v");

    s_Logger = spdlog::stdout_color_mt("ORION");
    s_Logger->set_level(spdlog::level::trace);
}

}
