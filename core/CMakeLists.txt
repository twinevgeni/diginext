cmake_minimum_required(VERSION 3.15)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.12" CACHE STRING "Minimum OS X deployment version")
project(diginext.core)

set(CMAKE_CXX_STANDARD 17)

add_library(${PROJECT_NAME} STATIC)

SET(CORE_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_include_directories(${PROJECT_NAME} PUBLIC ${CORE_INCLUDE})

SET(CORE_SRC
        src/Log/Log.cpp
        src/Log/LogConsole.cpp

        src/Base64/Base64.cpp

        src/TCP/TCP.cpp
        src/TCP/TCPConnection.cpp
        src/TCP/TCPClient.cpp
        src/TCP/TCPServer.cpp

        src/Storage/StorageServer.cpp
        src/Storage/StorageClient.cpp
        )

target_sources(${PROJECT_NAME} PRIVATE ${CORE_SRC})

find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC nlohmann_json nlohmann_json::nlohmann_json)

find_package(Boost REQUIRED)
target_include_directories(${PROJECT_NAME} PUBLIC ${Boost_INCLUDE_DIRS})

if (WIN32)
    target_link_libraries(${PROJECT_NAME} PUBLIC Ws2_32 Wldap32 Crypt32 bcrypt)
elseif (MACOSX)
    find_library(CoreFoundation CoreFoundation)
    find_library(IOKit IOKit)

    target_link_libraries(${PROJECT_NAME} PUBLIC
            $<$<PLATFORM_ID:Darwin>:${CoreFoundation}>
            $<$<PLATFORM_ID:Darwin>:${IOKit}>
            )
else()
    find_package (Threads REQUIRED)
    target_link_libraries(${PROJECT_NAME} PUBLIC
            ${CMAKE_THREAD_LIBS_INIT}
            )
endif()