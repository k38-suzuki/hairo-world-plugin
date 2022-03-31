
option(BUILD_OMPL_PLUGIN "Building OMPLPlugin" OFF)
if(NOT BUILD_OMPL_PLUGIN)
  return()
endif()

find_package(ompl REQUIRED)
include_directories(${OMPL_INCLUDE_DIRS})
link_directories(${OMPL_LIBRARY_DIRS})
add_definitions(${OMPL_DEFINITIONS})

set(sources
    IKPlannerItem.cpp
    SimpleSetupItem.cpp
    OMPLPlugin.cpp
    SimpleSetup.cpp
    )

set(headers
    gettext.h
    IKPlannerItem.h
    SimpleSetupItem.h
    SimpleSetup.h
    )

choreonoid_make_header_public(SimpleSetup.h)
choreonoid_make_header_public(SimpleSetupItem.h)

if(CMAKE_PROJECT_NAME STREQUAL "Choreonoid")
  # Build inside the Choreonoid project
  set(target CnoidOMPLPlugin)
  make_gettext_mofiles(${target} mofiles)
  choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
  target_link_libraries(${target} CnoidBodyPlugin ${OMPL_LIBRARIES})
else()
  # Stand alone build
  cmake_minimum_required(VERSION 3.1.0)
  project(OMPLPlugin)
  find_package(Choreonoid REQUIRED)
  add_definitions(${CHOREONOID_COMPILE_DEFINITIONS})
  include_directories(${CHOREONOID_INCLUDE_DIRS})
  link_directories(${CHOREONOID_LIBRARY_DIRS})
  set(target CnoidOMPLPlugin)
  make_gettext_mofiles(${target} mofiles)
  add_library(${target} SHARED ${sources} ${mofiles} HEADERS ${headers})
  target_link_libraries(${target} ${CHOREONOID_BODY_PLUGIN_LIBRARIES} ${OMPL_LIBRARIES})
  install(TARGETS ${target} LIBRARY DESTINATION ${CHOREONOID_PLUGIN_DIR})
endif()
