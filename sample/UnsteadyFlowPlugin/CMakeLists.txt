option(BUILD_UNSTEADYFLOW_PLUGIN "Building UnsteadyFlowPlugin" OFF)
if(NOT BUILD_UNSTEADYFLOW_PLUGIN)
  return()
endif()

if(CMAKE_PROJECT_NAME STREQUAL "Choreonoid")
  # Build inside the Choreonoid project
  set(target CnoidUnsteadyFlowPlugin)
  choreonoid_add_plugin(${target} UnsteadyFlowPlugin.cpp)
  target_link_libraries(${target} CnoidBodyPlugin CnoidCFDPlugin)
else()
  # Stand alone build
  set(target CnoidUnsteadyFlowPlugin)
  add_library(${target} SHARED UnsteadyFlowPlugin.cpp)
  target_link_libraries(${target} ${CHOREONOID_BODY_PLUGIN_LIBRARIES} CnoidCFDPlugin)
  install(TARGETS ${target} LIBRARY DESTINATION ${CHOREONOID_PLUGIN_DIR})
endif()