
option(BUILD_OMPL_PLUGIN "Building OMPLPlugin" OFF)
if(NOT BUILD_OMPL_PLUGIN)
  return()
endif()

find_package(ompl REQUIRED)
include_directories(${OMPL_INCLUDE_DIRS})
link_directories(${OMPL_LIBRARY_DIRS})
add_definitions(${OMPL_DEFINITIONS})

set(sources
    OMPLPlugin.cpp
    SimpleSetup.cpp
    SimpleSetupItem.cpp
    )

set(headers
    exportdecl.h
    gettext.h
    SimpleSetup.h
    SimpleSetupItem.h
    )

choreonoid_make_header_public(SimpleSetup.h)
choreonoid_make_header_public(SimpleSetupItem.h)

if(CMAKE_PROJECT_NAME STREQUAL "Choreonoid")
  # Build inside the Choreonoid project
  set(target CnoidOMPLPlugin)
  make_gettext_mofiles(${target} mofiles)
  choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
  target_link_libraries(${target} PUBLIC CnoidBodyPlugin ${OMPL_LIBRARIES})
else()
  # Stand alone build
  set(target CnoidOMPLPlugin)
  make_gettext_mofiles(${target} mofiles)
  add_library(${target} SHARED ${sources} ${mofiles} ${headers})
  target_link_libraries(${target} ${CHOREONOID_BODY_PLUGIN_LIBRARIES} ${OMPL_LIBRARIES})
  install(TARGETS ${target} LIBRARY DESTINATION ${CHOREONOID_PLUGIN_DIR})
endif()