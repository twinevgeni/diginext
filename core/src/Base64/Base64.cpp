#include "Base64/Base64.h"

namespace Diginext::Core
{
    std::string Base64::Encode(const char* data, size_t dataLength)
    {
        static constexpr char sEncodingTable[] = {
                'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                'w', 'x', 'y', 'z', '0', '1', '2', '3',
                '4', '5', '6', '7', '8', '9', '+', '/'
        };

        size_t in_len = dataLength;
        size_t out_len = 4 * ((in_len + 2) / 3);
        std::string ret(out_len, '\0');
        size_t i;
        char* p = const_cast<char*>(ret.c_str());

        for (i = 0; i < in_len - 2; i += 3)
        {
            *p++ = sEncodingTable[(data[i] >> 2) & 0x3F];
            *p++ = sEncodingTable[((data[i] & 0x3) << 4) | ((int)(data[i + 1] & 0xF0) >> 4)];
            *p++ = sEncodingTable[((data[i + 1] & 0xF) << 2) | ((int)(data[i + 2] & 0xC0) >> 6)];
            *p++ = sEncodingTable[data[i + 2] & 0x3F];
        }
        if (i < in_len)
        {
            *p++ = sEncodingTable[(data[i] >> 2) & 0x3F];
            if (i == (in_len - 1))
            {
                *p++ = sEncodingTable[((data[i] & 0x3) << 4)];
                *p++ = '=';
            }
            else
            {
                *p++ = sEncodingTable[((data[i] & 0x3) << 4) | ((int)(data[i + 1] & 0xF0) >> 4)];
                *p++ = sEncodingTable[((data[i + 1] & 0xF) << 2)];
            }
            *p++ = '=';
        }

        return ret;
    }

    void Base64::Decode(const std::string& base64, char** out, size_t* outLength)
    {
        static constexpr unsigned char kDecodingTable[] = {
                64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
                64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
                64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
                52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
                64, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
                15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
                64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
                41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,
                64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
                64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
                64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
                64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
                64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
                64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
                64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
                64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
        };

        size_t in_len = base64.size();

        if (in_len % 4 != 0)
            throw std::runtime_error("Input data size is not a multiple of 4");

        try
        {
            size_t out_len = in_len / 4 * 3;
            if (base64[in_len - 1] == '=')
                out_len--;
            if (base64[in_len - 2] == '=')
                out_len--;

            *out = new char[out_len];
            *outLength = out_len;

            for (size_t i = 0, j = 0; i < in_len;)
            {
                uint32_t a = base64[i] == '=' ? 0 & i++ : kDecodingTable[static_cast<int>(base64[i++])];
                uint32_t b = base64[i] == '=' ? 0 & i++ : kDecodingTable[static_cast<int>(base64[i++])];
                uint32_t c = base64[i] == '=' ? 0 & i++ : kDecodingTable[static_cast<int>(base64[i++])];
                uint32_t d = base64[i] == '=' ? 0 & i++ : kDecodingTable[static_cast<int>(base64[i++])];

                uint32_t triple = (a << 3 * 6) + (b << 2 * 6) + (c << 1 * 6) + (d << 0 * 6);

                if (j < out_len)
                    (*out)[j++] = (triple >> 2 * 8) & 0xFF;
                if (j < out_len)
                    (*out)[j++] = (triple >> 1 * 8) & 0xFF;
                if (j < out_len)
                    (*out)[j++] = (triple >> 0 * 8) & 0xFF;
            }
        }
        catch (...)
        {
            throw std::runtime_error("decode error");
        }
    }

    std::string Base64::Encode(const unsigned char* data, size_t dataLength)
    {
        return Encode((char*)data, dataLength);
    }

    void Base64::Decode(const std::string& base64, unsigned char** out, size_t* outLength)
    {
        Decode(base64, (char**)out, outLength);
    }

    std::string Base64::Encode(const std::string &data)
    {
        return Encode(data.c_str(), data.size());
    }

    std::string Base64::Decode(const std::string& base64)
    {
        char* outBin;
        size_t outLength;

        Decode(base64, &outBin, &outLength);
        std::string out = std::string(outBin, outLength);
        delete[] outBin;

        return out;
    }
}  // namespace PP_Base64