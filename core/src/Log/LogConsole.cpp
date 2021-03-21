#include "Log/LogConsole.h"

#include <memory>
#include <mutex>
#include <iostream>
#include <string>

void log_console(const std::string &loggerName, const std::string &type, const std::string &msg, std::mutex &sync) {
    std::lock_guard<std::mutex> guard(sync);
    std::cout << "--------------------------------------------" << std::endl;
    std::cout << loggerName << " : " << type << " : " << msg << std::endl;
}

namespace Diginext::Core::Log {

    namespace StrTypes {
        const std::string __TRACE = "trace";
        const std::string __DEBUG = "debug";
        const std::string __INFO = "info";
        const std::string __WARNING = "warning";
        const std::string __ERROR = "error";
        const std::string __FATAL = "fatal";
    }// namespace StrTypes

    std::mutex sync;

    Logger::pointer ConsoleLogger::create(const std::string& name, bool enabled)
    {
        return std::make_shared<ConsoleLogger>(name, enabled);
    }

    ConsoleLogger::ConsoleLogger(const std::string &name, bool enabled)
        : Logger(name, enabled) {
    }

    void ConsoleLogger::LogTrace(const std::string &msg) {
        if (this->Enabled()) {
            log_console(this->getName(), StrTypes::__TRACE, msg, sync);
        }
    }

    void ConsoleLogger::LogDebug(const std::string &msg) {
        if (this->Enabled()) {
            log_console(this->getName(), StrTypes::__DEBUG, msg, sync);
        }
    }

    void ConsoleLogger::LogInfo(const std::string &msg) {
        if (this->Enabled()) {
            log_console(this->getName(), StrTypes::__INFO, msg, sync);
        }
    }

    void ConsoleLogger::LogWarning(const std::string &msg) {
        if (this->Enabled()) {
            log_console(this->getName(), StrTypes::__WARNING, msg, sync);
        }
    }

    void ConsoleLogger::LogError(const std::string &msg) {
        if (this->Enabled()) {
            log_console(this->getName(), StrTypes::__ERROR, msg, sync);
        }
    }

    void ConsoleLogger::LogFatal(const std::string &msg) {
        if (this->Enabled()) {
            log_console(this->getName(), StrTypes::__FATAL, msg, sync);
        }
    }

}// namespace Diginext::Core::Log