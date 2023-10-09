cmake_minimum_required(VERSION 3.10)

project(TaskProcessor)
set(CMAKE_CXX_STANDARD 17)
add_executable(taskprocessor main.cpp)
find_package(Threads REQUIRED)
target_link_libraries(taskprocessor PRIVATE Threads::Threads)