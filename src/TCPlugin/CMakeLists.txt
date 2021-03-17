
set(sources
    TCAreaItem.cpp
    TCPlugin.cpp
    TCSimulatorItem.cpp
    )

set(headers
    TCAreaItem.h
    TCSimulatorItem.h
    )

if(CMAKE_PROJECT_NAME STREQUAL "Choreonoid")
  # Build inside the Choreonoid project
  set(target CnoidTCPlugin)
  make_gettext_mofiles(${target} mofiles)
  add_cnoid_plugin(${target} SHARED ${sources} ${mofiles} ${headers})
  target_link_libraries(${target} CnoidBodyPlugin CnoidFluidDynamicsPlugin)
  apply_common_setting_for_plugin(${target})
else()
  # Stand alone build
  cmake_minimum_required(VERSION 3.1.0)
  project(TCPlugin)
  find_package(Choreonoid REQUIRED)
  add_definitions(${CHOREONOID_DEFINITIONS})
  include_directories(${CHOREONOID_INCLUDE_DIRS})
  link_directories(${CHOREONOID_LIBRARY_DIRS})
  set(target CnoidTCPlugin)
  make_gettext_mofiles(${target} mofiles)
  add_library(${target} SHARED ${sources} ${mofiles} ${headers})
  target_link_libraries(${target} ${CHOREONOID_BODY_PLUGIN_LIBRARIES} CnoidFluidDynamicsPlugin)
  install(TARGETS ${target} LIBRARY DESTINATION ${CHOREONOID_PLUGIN_DIR})
endif()
