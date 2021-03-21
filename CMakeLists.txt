cmake_minimum_required(VERSION 3.15)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.12" CACHE STRING "Minimum OS X deployment version")
project(diginext)

set(CMAKE_CXX_STANDARD 17)

if (MSVC)  #use utf-8 in msvc
    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
endif ()

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(MACOS TRUE)
endif ()

add_subdirectory(core)
add_subdirectory(test.LogConsole)
#add_subdirectory(server)
#add_subdirectory(client)