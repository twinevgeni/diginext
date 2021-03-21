cmake_minimum_required(VERSION 3.15)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.12" CACHE STRING "Minimum OS X deployment version")
project(diginext.core)

set(CMAKE_CXX_STANDARD 17)

add_library(${PROJECT_NAME} STATIC)

SET(CORE_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_include_directories(${PROJECT_NAME} PUBLIC ${CORE_INCLUDE})

SET(CORE_SRC
        src/Log/Log.cpp
        src/Log/LogConsole.cpp)

target_sources(${PROJECT_NAME} PRIVATE ${CORE_SRC})

#find_package(nlohmann_json CONFIG REQUIRED)
#target_link_libraries(${PROJECT_NAME} PUBLIC nlohmann_json nlohmann_json::nlohmann_json)

#find_package(Boost REQUIRED)
#target_include_directories(${PROJECT_NAME} PUBLIC ${Boost_INCLUDE_DIRS})