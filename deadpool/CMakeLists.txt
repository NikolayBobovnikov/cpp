cmake_minimum_required(VERSION 3.10)

project(TaskProcessor)
set(CMAKE_CXX_STANDARD 17)
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${SOURCE_DIR})

message("--! Collecting source files from ${SOURCE_DIR}")
file(GLOB_RECURSE SOURCE
    ${SOURCE_DIR}/*.cpp
)
list(FILTER SOURCE EXCLUDE REGEX "main.cpp")

message("SOURCE : ${SOURCE}")
# build objects which would be used for linking in main project and tests
set(OBJECT_LIB "target_objects")
add_library(
    ${OBJECT_LIB}
    OBJECT
    ${SOURCE}
)

add_executable(taskprocessor 
  ${SOURCE_DIR}/main.cpp 
  $<TARGET_OBJECTS:${OBJECT_LIB}>)
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

# tests
enable_testing()
find_package(GTest REQUIRED)
add_executable(tests ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests.cpp $<TARGET_OBJECTS:${OBJECT_LIB}> )
target_link_libraries(tests PRIVATE GTest::GTest GTest::Main)