
option(BUILD_HAIRO_WORLD_PLUGIN "Building a HairoWorldPlugin" OFF)
if(NOT BUILD_HAIRO_WORLD_PLUGIN)
  return()
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_subdirectory(src)
add_subdirectory(include)
add_subdirectory(sample)
add_subdirectory(ext)
