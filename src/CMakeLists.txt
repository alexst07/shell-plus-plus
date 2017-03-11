# Copyright 2016 Alex Silva Torres
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cc)
file(GLOB SOURCES_PARSER ${CMAKE_CURRENT_SOURCE_DIR}/parser/*.cc)
file(GLOB SOURCES_INTER ${CMAKE_CURRENT_SOURCE_DIR}/interpreter/*.cc)
file(GLOB SOURCES_OBJECTS ${CMAKE_CURRENT_SOURCE_DIR}/objects/*.cc)
file(GLOB SOURCES_MODULES ${CMAKE_CURRENT_SOURCE_DIR}/modules/*.cc)
file(GLOB SOURCES_UTILS ${CMAKE_CURRENT_SOURCE_DIR}/utils/*.cc)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(shpp SHARED ${SOURCES} ${SOURCES_PARSER} ${SOURCES_INTER}
${SOURCES_OBJECTS} ${SOURCES_MODULES} ${SOURCES_UTILS})

target_link_libraries(shpp
  ${CMAKE_THREAD_LIBS_INIT}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${READLINE_LIBRARY})

install(TARGETS shpp DESTINATION "${CMAKE_INSTALL_PREFIX}/lib")
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
