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

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_subdirectory(src)
add_subdirectory(include)

if(ENABLE_SAMPLES)
  add_subdirectory(sample)
endif()

if(ENABLE_EXT)
  add_subdirectory(ext)
endif()

if(UNIX AND BUILD_PHITS_PLUGIN)
    option(ENABLE_QAD "Enable QAD option." OFF)
endif()

add_subdirectory(share)