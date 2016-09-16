file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/parser/*.cc)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(setti ${SOURCES})
