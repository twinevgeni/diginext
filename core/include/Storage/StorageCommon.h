#ifndef DIGINEXT_CORE___STORAGE_STORAGE_COMMON_H
#define DIGINEXT_CORE___STORAGE_STORAGE_COMMON_H

#include <string>

namespace Diginext::Core::Storage {
    namespace JSON {
        namespace KEY {
            const std::string REQUEST = "request";
            const std::string KEY = "key";
            const std::string VALUE = "value";
            const std::string STATUS = "status";
            const std::string DESCRIPTION = "description";
        }

        namespace VALUE {
            const std::string STATUS_OK = "ok";
            const std::string STATUS_ERROR = "error";
            const std::string REQUEST_READ = "read";
            const std::string REQUEST_WRITE = "write";
        }
    }
}

#endif
