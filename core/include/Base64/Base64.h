// Кодировка utf-8

#ifndef DIGINEXT_CORE___BASE64_BASE64_H
#define DIGINEXT_CORE___BASE64_BASE64_H

#include <stdexcept>
#include <string>

namespace Diginext::Core {
    class Base64 {
    public:
        static std::string Encode(const char *data, size_t dataLength);
        static void Decode(const std::string &base64, char **out, size_t *outLength);

        static std::string Encode(const unsigned char *data, size_t dataLength);
        static void Decode(const std::string &base64, unsigned char **out, size_t *outLength);

        static std::string Encode(const std::string &data);
        static std::string Decode(const std::string &base64);
    };

}// namespace Diginext::Core

#endif//PP_LIB_AES_BASE64_H
