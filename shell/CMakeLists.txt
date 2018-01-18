include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
CHECK_CXX_COMPILER_FLAG("-std=c++14" COMPILER_SUPPORTS_CXX11)

find_package(Readlinepp)

if (NOT READLINEPP_FOUND)
  find_package(Readline REQUIRED)
endif ()

if (READLINEPP_FOUND)
  message(STATUS "READLINEPP_FOUND: Yes")
  include_directories(${READLINEPPINCLUDE_DIRS})
  set (DREADLINE_LIBRARIES ${READLINEPP_LIBRARIES})
else (READLINEPP_FOUND)
  message(STATUS "READLINEPP_FOUND: No")
  include_directories(${READLINE_INCLUDE_DIR})
  set (DREADLINE_LIBRARIES ${READLINE_LIBRARY})
endif ()

file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cc)

add_executable(shpp_exe ${SOURCES})
set_target_properties(shpp_exe PROPERTIES OUTPUT_NAME shpp)

if(CMAKE_COMPILER_IS_GNUCXX AND COV_TESTS)
  message("gcov on")
  include(CodeCoverage)
  setup_target_for_coverage(shpp_coverage "./shell/shpp ../test/interpreter/run.shpp" coverage)
endif()

message("DREADLINE_LIBRARIES: ${DREADLINE_LIBRARIES}")
target_link_libraries(shpp_exe shpp /home/alex/Projects/readline/build/libreadlinepp.so)

if(CMAKE_COMPILER_IS_GNUCXX AND COV_TESTS)
    target_link_libraries(shpp_exe gcov)
endif()

install(TARGETS shpp_exe RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
