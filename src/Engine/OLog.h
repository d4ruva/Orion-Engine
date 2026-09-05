#pragma once

#include <memory>
#include <spdlog/logger.h>

namespace Orion {
class OLog {
    public:
    static void Init();

    static std::shared_ptr<spdlog::logger>& GetLogger() { return s_Logger; }

    private:
    static std::shared_ptr<spdlog::logger> s_Logger;
};
} // namespace Orion

#define ORION_TRACE(...) ::Orion::OLog::GetLogger()->trace(__VA_ARGS__)
#define ORION_INFO(...) ::Orion::OLog::GetLogger()->info(__VA_ARGS__)
#define ORION_WARN(...) ::Orion::OLog::GetLogger()->warn(__VA_ARGS__)
#define ORION_ERROR(...) ::Orion::OLog::GetLogger()->error(__VA_ARGS__)
#define ORION_CRITICAL(...) ::Orion::OLog::GetLogger()->critical(__VA_ARGS__)
