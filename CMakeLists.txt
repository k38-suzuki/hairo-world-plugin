if(CMAKE_PROJECT_NAME STREQUAL "Choreonoid")
  # Build inside the Choreonoid project
  if(NOT ENABLE_GUI)
    return()
  endif()

  option(BUILD_HAIRO_WORLD_PLUGIN "Building a HAIROWorldPlugin" ON)
  if(NOT BUILD_HAIRO_WORLD_PLUGIN)
    return()
  endif()
else()
  # Stand alone build
  cmake_minimum_required(VERSION 3.5.0)
  project(HAIROWorldPlugin)
  find_package(Choreonoid REQUIRED)
  if(CHOREONOID_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD ${CHOREONOID_CXX_STANDARD})
    set(CMAKE_CXX_EXTENSIONS OFF)
  else()
    add_compile_options(-std=c++14)
  endif()
endif()

set(CHOREONOID_SHARE_DIR ${CMAKE_INSTALL_PREFIX}/${CHOREONOID_SHARE_SUBDIR})
set(CHOREONOID_SOURCE_SHARE_DIR ${PROJECT_SOURCE_DIR}/share)

if(PROJECT_BINARY_DIR STREQUAL PROJECT_SOURCE_DIR)
  set(CHOREONOID_BINARY_SHARE_DIR ${CHOREONOID_SOURCE_SHARE_DIR})
else()
  set(CHOREONOID_BINARY_SHARE_DIR ${PROJECT_BINARY_DIR}/${CHOREONOID_SHARE_SUBDIR})
endif()

if(NOT PROJECT_BINARY_DIR STREQUAL PROJECT_SOURCE_DIR)
  include_directories(${CMAKE_CURRENT_BINARY_DIR})
  include_directories(${CMAKE_CURRENT_BINARY_DIR}/include)
endif()
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_subdirectory(src)

if(ENABLE_SAMPLES)
  add_subdirectory(sample)
endif()

if(UNIX AND BUILD_PHITS_PLUGIN)
    option(ENABLE_QAD "Enable QAD option." OFF)
endif()

add_subdirectory(share)