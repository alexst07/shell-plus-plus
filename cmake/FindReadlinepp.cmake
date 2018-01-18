
if (READLINEPP_LIBRARIES AND READLINEPP_INCLUDE_DIRS)
  # in cache already
  set(READLINEPP_FOUND TRUE)
else (READLINEPP_LIBRARIES AND READLINEPP_INCLUDE_DIRS)

  set(READLINEPP_INCLUDE_HEADERS
      buffer-string.h
      cursor.h
      key-events.h
      prompt.h
      scope-exit.h
      utils.h
      complete.h
      history.h
      log.h
      readline.h
      text.h
  )

  find_path(READLINEPP_INCLUDE_DIR
    NAMES
      ${READLINEPP_INCLUDE_HEADERS}
    PATHS
      /usr/include/readlinepp
      /usr/local/include/readlinepp
  )

  find_library(READLINEPP_LIBRARY
    NAMES
      readlinepp
    PATHS
      /usr/lib/readlinepp
      /usr/local/lib/readlinepp
  )

  set(READLINEPP_INCLUDE_DIRS
    ${READLINEPP_INCLUDE_DIR}
  )

  set(READLINEPP_LIBRARIES
    ${READLINEPP_LIBRARY}
  )

  if (READLINEPP_INCLUDE_DIRS AND READLINEPP_LIBRARIES)
    set(READLINEPP_FOUND TRUE)
  endif ()

  if (READLINEPP_FOUND)
    if (NOT READLINEPP_FIND_QUIETLY)
      message(STATUS "Found readlinepp: ${READLINEPP_LIBRARIES}")
    endif ()
  else (READLINEPP_FOUND)
    if (READLINEPP_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find libreadlinepp")
    endif ()
  endif ()

  mark_as_advanced(READLINEPP_FOUND READLINEPP_INCLUDE_DIRS READLINEPP_LIBRARIES)

endif ()
