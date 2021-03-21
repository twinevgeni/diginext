#include <Log/LogConsole.h>

using namespace Diginext::Core::Log;

int main() {
    Logger::pointer logger = ConsoleLogger::create("test");
    logger->LogTrace("Test Trace");
    logger->LogDebug("Test Debug");
    logger->LogInfo("Test Info");
    logger->LogWarning("Test Warning");
    logger->LogError("Test Error");
    logger->LogFatal("Test Fatal");

    return 0;
}
