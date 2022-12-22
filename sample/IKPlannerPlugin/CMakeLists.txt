
option(BUILD_IKPLANNER_PLUGIN "Building IKPlannerPlugin" OFF)
if(NOT BUILD_IKPLANNER_PLUGIN OR NOT BUILD_OMPL_PLUGIN)
  return()
endif()

set(sources
    IKPlannerItem.cpp
    IKPlannerPlugin.cpp
    )

set(headers
    gettext.h
    IKPlannerItem.h
    )

if(CMAKE_PROJECT_NAME STREQUAL "Choreonoid")
  # Build inside the Choreonoid project
  set(target CnoidIKPlannerPlugin)
  make_gettext_mofiles(${target} mofiles)
  choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
  target_link_libraries(${target} CnoidBodyPlugin CnoidOMPLPlugin)
else()
  # Stand alone build
  set(target CnoidIKPlannerPlugin)
  make_gettext_mofiles(${target} mofiles)
  choreonoid_add_library(${target} SHARED ${sources} ${mofiles} HEADERS ${headers})
  target_link_libraries(${target} ${CHOREONOID_BODY_PLUGIN_LIBRARIES} CnoidOMPLPlugin)
  install(TARGETS ${target} LIBRARY DESTINATION ${CHOREONOID_PLUGIN_DIR})
endif()