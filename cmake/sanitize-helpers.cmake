set(ASAN_LIB_NAMES
asan
libasan.so.0
libasan.so.1
libasan.so.2
libasan.so.3
)
set(UBSAN_LIB_NAMES
ubsan
libubsan.so.0
)
set(TSAN_LIB_NAMES
tsan
libtsan.so.0
)

option(SANITIZE_ADDRESS "Enable Address Sanitizer" OFF)
option(SANITIZE_UNDEFINED "Enable Undefined Behavior Sanitizer" OFF)
option(SANITIZE_THREAD "Enable Thread Sanitizer" OFF)

if ( SANITIZE_ADDRESS )
  if (SANITIZE_ADDRESS AND SANITIZE_THREAD)
      message(FATAL_ERROR "AddressSanitizer is not compatible with "
          "ThreadSanitizer or MemorySanitizer.")
  endif ()

  find_library(__libasan NAMES ${ASAN_LIB_NAMES} PATHS /usr/lib /usr/lib/x86_64-linux-gnu)
  if (NOT __libasan)
    message(STATUS "Enabling Address Sanitizer - Failed")
    message(STATUS "Missing libasan.so")
    unset(SANITIZE_ADDRESS CACHE)
  else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
    execute_process (COMMAND bash -c "ls /usr/bin/llvm-symbolizer-*" OUTPUT_VARIABLE llvm_sym)
    set(ASAN_SYMBOLIZER_PATH "${llvm_sym}")
    set(ASAN_OPTIONS "symbolize=1")
    message(STATUS "Enabling Address Sanitizer - Success")
    message("")
    message("### For a better understanding of Asan output, execute:")
    message("")
    message("###   export ASAN_SYMBOLIZER_PATH=${ASAN_SYMBOLIZER_PATH}")
    message("###   export ASAN_OPTIONS=${ASAN_OPTIONS}")
    message("")
  endif ()
endif()

if ( SANITIZE_UNDEFINED )
  find_library(__libubsan NAMES ${UBSAN_LIB_NAMES} PATHS /usr/lib /usr/lib/x86_64-linux-gnu)
  if (NOT __libubsan)
    message(STATUS "Enabling Undefined Behavior Sanitizer - Failed")
    message(STATUS "Missing libubsan.so")
    unset(SANITIZE_UNDEFINED CACHE)
  else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined")
    message(STATUS "Enabling Undefined Behavior Sanitizer - Success")
  endif ()
endif()

if ( SANITIZE_THREAD )
  find_library(__libtsan NAMES ${TSAN_LIB_NAMES} PATHS /usr/lib /usr/lib/x86_64-linux-gnu)
  if (NOT __libtsan)
    message(STATUS "Enabling Thread Sanitizer - Failed")
    message(STATUS "Missing libtsan.so")
    unset(SANITIZE_THREAD CACHE)
  else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread")
    message(STATUS "Enabling Thread Sanitizer - Success")
  endif ()
endif()

