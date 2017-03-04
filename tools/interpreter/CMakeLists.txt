include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
CHECK_CXX_COMPILER_FLAG("-std=c++14" COMPILER_SUPPORTS_CXX11)

add_executable(interpreter interpreter.cc)
target_link_libraries(interpreter
  shpp
  ${CMAKE_THREAD_LIBS_INIT}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY})

