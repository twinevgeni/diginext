#ifndef DIGINEXT_CORE___LOG_LOG_H
#define DIGINEXT_CORE___LOG_LOG_H

#ifndef BOOST_BIND_GLOBAL_PLACEHOLDERS
#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#endif

#include <memory>
#include <string>

namespace Diginext::Core::Log {

    /**
     * \brief Класс логера
     */
    class Logger {
    private:
        std::string name;
        bool enabled;

    public:
        typedef std::shared_ptr<Logger> pointer;

        explicit Logger(const std::string &name, bool enabled = true);
        virtual ~Logger() = default;

        std::string getName() const;

        virtual bool Enabled() const;
        virtual void SetEnabled(bool enabled = true);
        virtual void Enable();
        virtual void Disable();

        virtual void LogTrace(const std::string &msg) = 0;
        virtual void LogDebug(const std::string &msg) = 0;
        virtual void LogInfo(const std::string &msg) = 0;
        virtual void LogWarning(const std::string &msg) = 0;
        virtual void LogError(const std::string &msg) = 0;
        virtual void LogFatal(const std::string &msg) = 0;
    };
}// namespace Diginext::Core::Log

#endif
