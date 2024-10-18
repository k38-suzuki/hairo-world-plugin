if(NOT ENABLE_GUI)
  return()
endif()

option(BUILD_HAIRO_WORLD_PLUGIN "Building HAIROWorldPlugin" ON)
if(NOT BUILD_HAIRO_WORLD_PLUGIN)
  return()
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

if(INSTALL_SDK)
  add_subdirectory(cmake)
endif()
add_subdirectory(misc)
