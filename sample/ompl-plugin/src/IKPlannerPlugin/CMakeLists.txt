if(NOT ENABLE_GUI)
  return()
endif()

option(BUILD_IK_PLANNER_PLUGIN "Building IKPlannerPlugin" OFF)
if(NOT BUILD_IK_PLANNER_PLUGIN)
  return()
endif()

set(sources
  IKPlannerItem.cpp
  IKPlannerPlugin.cpp
)

set(headers
  IKPlannerItem.h
)

set(target CnoidIKPlannerPlugin)
choreonoid_make_gettext_mo_files(${target} mofiles)
choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
target_link_libraries(${target} PUBLIC CnoidBodyPlugin CnoidOMPLPlugin)