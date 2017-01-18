cmake_minimum_required (VERSION 2.8)
project(seti C CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

include(cmake/SettiUtils.cmake)
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++14" COMPILER_SUPPORTS_CXX14)
if(COMPILER_SUPPORTS_CXX14)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
else()
  message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++14 support. Please use a different C++ compiler.")
endif()

SETI_OPTION(BUILD_UNIT_TESTS "Build unit tests" ON)
SETI_OPTION(COV_TESTS "Coverage tests" OFF)

if (COV_TESTS)
  # put on debug
  SET(CMAKE_CXX_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage")
  SET(CMAKE_C_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage")
  set(CMAKE_BUILD_TYPE "Debug")
endif()
#
# Boost support
#
find_package(Boost 1.58 COMPONENTS system filesystem REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
find_package (Threads REQUIRED)

if (BUILD_UNIT_TESTS)
  enable_testing()
  find_package(GTest REQUIRED)
  include_directories(${GTEST_INCLUDE_DIRS})
endif ()

include_directories(${CMAKE_SOURCE_DIR}/src/)

add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(examples)
add_subdirectory(shell)
