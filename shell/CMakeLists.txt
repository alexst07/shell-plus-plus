include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
CHECK_CXX_COMPILER_FLAG("-std=c++14" COMPILER_SUPPORTS_CXX11)

find_package(Readline REQUIRED)

include_directories(${READLINE_INCLUDE_DIR})

file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cc)

add_executable(shpp_exe ${SOURCES})
set_target_properties(shpp_exe PROPERTIES OUTPUT_NAME shpp)

if(CMAKE_COMPILER_IS_GNUCXX AND COV_TESTS)
  message("gcov on")
    include(CodeCoverage)
    setup_target_for_coverage(shpp_coverage "./shell/shpp ../test/interpreter/run.shpp" coverage)
endif()

target_link_libraries(shpp_exe shpp ${READLINE_LIBRARY})

if(CMAKE_COMPILER_IS_GNUCXX AND COV_TESTS)
    target_link_libraries(shpp_exe gcov)
endif()

install(TARGETS shpp_exe RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

