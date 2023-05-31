cmake_minimum_required(VERSION 3.22)
project(futexTest)

set(SOURCES main.cpp)

find_package(Threads REQUIRED)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} Threads::Threads)