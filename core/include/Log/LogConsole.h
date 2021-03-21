#ifndef DIGINEXT_CORE___LOG_LOG_CONSOLE_H
#define DIGINEXT_CORE___LOG_LOG_CONSOLE_H

#include "Log/Log.h"

#include <string>

namespace Diginext::Core::Log {
    class ConsoleLogger : public Logger {
    public:
        static Logger::pointer create(const std::string &name = "logger", bool enabled = true);

        explicit ConsoleLogger(const std::string &name, bool enabled = true);

        void LogTrace(const std::string &msg) override;
        void LogDebug(const std::string &msg) override;
        void LogInfo(const std::string &msg) override;
        void LogWarning(const std::string &msg) override;
        void LogError(const std::string &msg) override;
        void LogFatal(const std::string &msg) override;
    };
}// namespace Diginext::Core::Log

#endif
