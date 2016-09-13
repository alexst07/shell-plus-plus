cmake_minimum_required (VERSION 2.8)
project(setti C CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

include(cmake/SettiUtils.cmake)
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++14" COMPILER_SUPPORTS_CXX14)
if(COMPILER_SUPPORTS_CXX14)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
else()
  message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++14 support. Please use a different C++ compiler.")
endif()

#
# Boost support
#
find_package(Boost 1.58 REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

# TODO: Correct gtest address
# if (BUILD_TESTS)
#   enable_testing()
#   include(cmake/GTest.cmake)
#   include_directories(SYSTEM ${GTEST_INCLUDE_DIRS})
# endif ()

include_directories(${CMAKE_SOURCE_DIR}/src/)

add_subdirectory(test)

SETTI_OPTION(BUILD_TESTS         "Build all tests"                             ON)

