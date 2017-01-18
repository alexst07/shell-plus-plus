include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
CHECK_CXX_COMPILER_FLAG("-std=c++14" COMPILER_SUPPORTS_CXX11)

find_package(Readline REQUIRED)

include_directories(${READLINE_INCLUDE_DIR})

file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cc)

add_executable(seti ${SOURCES})
message("merda")
if(CMAKE_COMPILER_IS_GNUCXX AND COV_TESTS)
  message("gcov on")
    include(CodeCoverage)
    setup_target_for_coverage(seti_coverage "./shell/seti ../test/interpreter/run.seti" coverage)
endif()

target_link_libraries(seti
  setti
  ${CMAKE_THREAD_LIBS_INIT}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${READLINE_LIBRARY})

if(CMAKE_COMPILER_IS_GNUCXX AND COV_TESTS)
    target_link_libraries(seti gcov)
endif()

