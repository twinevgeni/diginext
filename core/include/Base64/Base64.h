// Кодировка utf-8

#ifndef DIGINEXT_CORE___BASE64_BASE64_H
#define DIGINEXT_CORE___BASE64_BASE64_H

#include <stdexcept>
#include <string>

namespace Diginext::Core {

    /**
     * \brief base64
     */
    class Base64 {
    public:
        static std::string Encode(const char *data, size_t dataLength);
        static void Decode(const std::string &base64, char **out, size_t *outLength);

        static std::string Encode(const unsigned char *data, size_t dataLength);
        static void Decode(const std::string &base64, unsigned char **out, size_t *outLength);

        /**
         * @brief base64 encode
         * @details encode string to base64 string
         * @param[in] data
         * @return base64 string
         */
        static std::string Encode(const std::string &data);

        /**
         * @brief base64 devode
         * @details decode base64 string
         * @param[in] base64
         * @return plain string
         */
        static std::string Decode(const std::string &base64);
    };

}// namespace Diginext::Core

#endif//PP_LIB_AES_BASE64_H
