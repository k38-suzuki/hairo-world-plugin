if(CMAKE_PROJECT_NAME STREQUAL "Choreonoid")
  # Build inside the Choreonoid project

else()
  # Stand alone build
  cmake_minimum_required(VERSION 3.1.0)
  project(hairo_world_plugin)
  find_package(Choreonoid REQUIRED)
  set(CMAKE_CXX_STANDARD ${CHOREONOID_CXX_STANDARD})
  set(CMAKE_CXX_EXTENSIONS OFF)
  include_directories(${CHOREONOID_INCLUDE_DIRS})
  link_directories(${CHOREONOID_LIBRARY_DIRS})

endif()

option(BUILD_HAIRO_WORLD_PLUGIN "Building HAIROWorldPlugin" ON)
if(NOT BUILD_HAIRO_WORLD_PLUGIN)
  return()
endif()

set(CHOREONOID_SHARE_DIR ${CMAKE_INSTALL_PREFIX}/${CHOREONOID_SHARE_SUBDIR})
set(CHOREONOID_SOURCE_SHARE_DIR ${PROJECT_SOURCE_DIR}/share)

if(PROJECT_BINARY_DIR STREQUAL PROJECT_SOURCE_DIR)
  set(CHOREONOID_BINARY_SHARE_DIR ${CHOREONOID_SOURCE_SHARE_DIR})
else()
  set(CHOREONOID_BINARY_SHARE_DIR ${PROJECT_BINARY_DIR}/${CHOREONOID_SHARE_SUBDIR})
endif()

add_subdirectory(src)
add_subdirectory(include)

if(UNIX AND BUILD_PHITS_PLUGIN)
  option(ENABLE_QAD "Enable QAD" OFF)
endif()
add_subdirectory(share)

if(ENABLE_SAMPLES)
  add_subdirectory(sample)
endif()