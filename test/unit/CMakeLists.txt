include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
CHECK_CXX_COMPILER_FLAG("-std=c++14" COMPILER_SUPPORTS_CXX11)

message("${CMAKE_CURRENT_SOURCE_DIR}")
file(GLOB SOURCES ${CMAKE_SOURCE_DIR}/src/*.cc)

include_directories(SYSTEM ${GTEST_INCLUDE_DIRS})

file(GLOB SOURCES_TEST ${CMAKE_CURRENT_SOURCE_DIR}/*.cc)
foreach(local_file ${SOURCES_TEST})
  get_filename_component(local_filename ${local_file} NAME_WE)
  message("Compiling: ${local_file} ${SOURCES} ${TBB_LIBRARIES} ${GTEST_MAIN_LIBRARIES} ${GTEST_LIBRARIES}")
  add_executable(${local_filename} ${local_file} ${SOURCES})
  target_link_libraries(${local_filename} ${TBB_LIBRARIES} ${GTEST_MAIN_LIBRARIES} ${GTEST_LIBRARIES})
endforeach()
