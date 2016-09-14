include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
CHECK_CXX_COMPILER_FLAG("-std=c++14" COMPILER_SUPPORTS_CXX11)

message("${CMAKE_CURRENT_SOURCE_DIR}")
file(GLOB SOURCES ${CMAKE_SOURCE_DIR}/src/*.cc)

include_directories(SYSTEM ${GTEST_INCLUDE_DIRS})

file(GLOB SOURCES_TEST ${CMAKE_CURRENT_SOURCE_DIR}/*.cc)
foreach(local_file ${SOURCES_TEST})
  get_filename_component(local_filename ${local_file} NAME_WE)
  message("Compiling test: ${local_file} ${SOURCES} ${GTEST_BOTH_LIBRARIES}")
  add_executable(${local_filename} ${local_file} ${SOURCES})
  target_link_libraries(${local_filename} ${CMAKE_THREAD_LIBS_INIT} ${GTEST_BOTH_LIBRARIES})
  add_test(UnitTests ${local_filename})
endforeach()
