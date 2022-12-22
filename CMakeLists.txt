if(CMAKE_PROJECT_NAME STREQUAL "Choreonoid")
  # Build inside the Choreonoid project
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

if(NOT ENABLE_GUI OR WIN32 OR APPLE)
  return()
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(src)
add_subdirectory(sample)
add_subdirectory(ext)

if(UNIX AND BUILD_PHITS_PLUGIN)
    option(ENABLE_QAD "Enable QAD option." OFF)
endif()

add_subdirectory(share)