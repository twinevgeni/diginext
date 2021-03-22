#ifndef DIGINEXT_GTEST___BASE64_BASE64_TEST_H
#define DIGINEXT_GTEST___BASE64_BASE64_TEST_H

#include <Base64/Base64.h>

#include <string>

namespace Diginext::Core::GTest {
    using namespace std;

    struct TestStructString
    {
        string data;
        string base64;
    };

    TestStructString assertsString[] = {
            { "", "" },
            { "HELLO", "SEVMTE8=" },
            { "HELLO BASE64", "SEVMTE8gQkFTRTY0" },
            { "fgkmn gjk fdi jnjfnjewrnhjfj kkfeFSDKGGD65465434^^$#E$$", "ZmdrbW4gZ2prIGZkaSBqbmpmbmpld3JuaGpmaiBra2ZlRlNES0dHRDY1NDY1NDM0Xl4kI0UkJA==" },
    };

    TEST(Test_Base64, Test_Encode_String) {
        for (const auto& assert : assertsString)
        {
            ASSERT_EQ(assert.base64, Base64::Encode(assert.data));
        }
    }

    TEST(Test_Base64, Test_Devode_String) {
        for (const auto& assert : assertsString)
        {
            ASSERT_EQ(assert.data, Base64::Decode(assert.base64));
        }
    }
}

#endif
