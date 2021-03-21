#include "Log/Log.h"

namespace Diginext::Core::Log {

    Logger::Logger(const std::string &name, bool enabled) {
        this->name = name;
        this->enabled = enabled;
    }

    std::string Logger::getName() const
    {
        return this->name;
    }

    bool Logger::Enabled() const
    {
        return this->enabled;
    }

    void Logger::SetEnabled(bool enabled)
    {
        if (enabled)
        {
            this->Enable();
        }
        else
        {
            this->Disable();
        }
    }

    void Logger::Enable()
    {
        this->enabled = true;
    }

    void Logger::Disable()
    {
        this->enabled = false;
    }

}// namespace Diginext::Core::Log