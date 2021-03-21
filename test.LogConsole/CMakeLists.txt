cmake_minimum_required(VERSION 3.15)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.12" CACHE STRING "Minimum OS X deployment version")
project(diginext.test.LogConsole)

set(CMAKE_CXX_STANDARD 17)

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE diginext.core)

target_link_libraries(${PROJECT_NAME} PRIVATE
        ${Boost_DATE_TIME_LIBRARY}
        ${Boost_CHRONO_LIBRARY})