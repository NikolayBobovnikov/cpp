cmake_minimum_required(VERSION 3.10)

project(TaskProcessor)
set(CMAKE_CXX_STANDARD 17)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_executable(taskprocessor 
  external_api.cpp 
  process_request_func.cpp 
  request_processor.cpp 
  stopper.cpp 
  main.cpp )
find_package(Threads REQUIRED)
target_link_libraries(taskprocessor PRIVATE Threads::Threads)

# Enable clang-tidy checks
find_program(CLANG_TIDY_BIN NAMES "clang-tidy" PATHS "/usr/bin" "/usr/local/bin")
if(CLANG_TIDY_BIN)
    message(STATUS "Found clang-tidy: ${CLANG_TIDY_BIN}")
    set_target_properties(taskprocessor PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_BIN}")
else()
    message(WARNING "clang-tidy not found.")
endif()