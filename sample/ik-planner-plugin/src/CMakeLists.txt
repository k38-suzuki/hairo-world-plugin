
option(BUILD_IKPLANNER_PLUGIN "Building IKPlannerPlugin" OFF)
if(NOT BUILD_IKPLANNER_PLUGIN OR NOT BUILD_OMPL_PLUGIN)
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
make_gettext_mofiles(${target} mofiles)
choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
target_link_libraries(${target} PUBLIC CnoidBodyPlugin CnoidOMPLPlugin)