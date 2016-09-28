include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
CHECK_CXX_COMPILER_FLAG("-std=c++14" COMPILER_SUPPORTS_CXX11)

add_executable(print_tokens print_tokens.cc)
target_link_libraries(print_tokens setti ${CMAKE_THREAD_LIBS_INIT})
