
if(NOT ENABLE_GUI OR WIN32 OR APPLE)
  return()
endif()

option(BUILD_HAIRO_WORLD_PLUGIN "Building a HAIROWorldPlugin" OFF)
if(NOT BUILD_HAIRO_WORLD_PLUGIN)
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
