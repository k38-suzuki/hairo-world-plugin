if(NOT ENABLE_GUI)
  return()
endif()

option(BUILD_UNSTEADYFLOW_SAMPLE "Building an unsteady flow sample" OFF)
if(NOT BUILD_UNSTEADYFLOW_SAMPLE)
  return()
endif()

set(sources
  UnsteadyFlowPlugin.cpp
)

set(target CnoidUnsteadyFlowPlugin)
choreonoid_add_plugin(${target} ${sources})
target_link_libraries(${target} PUBLIC CnoidBodyPlugin CnoidCFDPlugin)