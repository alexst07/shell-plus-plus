include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
CHECK_CXX_COMPILER_FLAG("-std=c++14" COMPILER_SUPPORTS_CXX11)

find_package(Readline REQUIRED)

include_directories(${READLINE_INCLUDE_DIR})

file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cc)

add_executable(seti_exe ${SOURCES})
set_target_properties(seti_exe PROPERTIES OUTPUT_NAME seti)

if(CMAKE_COMPILER_IS_GNUCXX AND COV_TESTS)
  message("gcov on")
    include(CodeCoverage)
    setup_target_for_coverage(seti_coverage "./shell/seti ../test/interpreter/run.seti" coverage)
endif()

target_link_libraries(seti_exe seti)

if(CMAKE_COMPILER_IS_GNUCXX AND COV_TESTS)
    target_link_libraries(seti_exe gcov)
endif()

install(TARGETS seti_exe RUNTIME DESTINATION /usr/local/bin)

