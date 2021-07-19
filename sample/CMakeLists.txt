add_subdirectory(HAIROWorld)
add_subdirectory(tutorial)

option(BUILD_HAIRO_WORLD_TOOLS "Building HAIROWorldTools" OFF)
if(NOT BUILD_HAIRO_WORLD_TOOLS)
  return()
endif()

add_subdirectory(FileBoxPlugin)
add_subdirectory(OMPLPlugin)
add_subdirectory(StartupPlugin)
