#include <gtest/gtest.h>

#include "TCP/TCP_Test.h"

int main(int argc, char** argv)
{
    Diginext::Core::TCP::GTest::init_tcp_test();
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}