
set(sources
    AreaItem.cpp
    CFDBody.cpp
    CFDLink.cpp
    FluidAreaItem.cpp
    FluidDynamicsPlugin.cpp
    FluidDynamicsSimulatorItem.cpp
    Rotor.cpp
    Thruster.cpp
    )

set(headers
    AreaItem.h
    CFDBody.h
    CFDLink.h
    FluidAreaItem.h
    FluidDynamicsSimulatorItem.h
    Rotor.h
    Thruster.h
    exportdecl.h
    gettext.h
    )

if(CMAKE_PROJECT_NAME STREQUAL "Choreonoid")
  # Build inside the Choreonoid project
  set(target CnoidFluidDynamicsPlugin)
  make_gettext_mofiles(${target} mofiles)
  choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
  target_link_libraries(${target} CnoidBodyPlugin)
  apply_common_setting_for_plugin(${target})
else()
  # Stand alone build
  cmake_minimum_required(VERSION 3.1.0)
  project(FluidDynamicsPlugin)
  find_package(Choreonoid REQUIRED)
  add_definitions(${CHOREONOID_DEFINITIONS})
  include_directories(${CHOREONOID_INCLUDE_DIRS})
  link_directories(${CHOREONOID_LIBRARY_DIRS})
  set(target CnoidFluidDynamicsPlugin)
  make_gettext_mofiles(${target} mofiles)
  add_library(${target} SHARED ${sources} ${mofiles} HEADERS ${headers})
  target_link_libraries(${target} ${CHOREONOID_BODY_PLUGIN_LIBRARIES})
  install(TARGETS ${target} LIBRARY DESTINATION ${CHOREONOID_PLUGIN_DIR})
endif()
