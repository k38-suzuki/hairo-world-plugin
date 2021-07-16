
option(BUILD_STARTUP_PLUGIN "Building a StartupPlugin" OFF)
if(NOT BUILD_STARTUP_PLUGIN)
  return()
endif()

set(sources
    StartupDialog.cpp
    StartupPlugin.cpp
    )

set(headers
    gettext.h
    StartupDialog.h
    )

set(qtsources
    JProcess.cpp
    )

set(qtheaders
    JProcess.h
    )

QT5_WRAP_CPP(qtsources ${qtheaders})

set(sources ${sources} ${qtsources})

if(CMAKE_PROJECT_NAME STREQUAL "Choreonoid")
  # Build inside the Choreonoid project
  set(target CnoidStartupPlugin)
  make_gettext_mofiles(${target} mofiles)
  choreonoid_add_plugin(${target} ${sources} ${mofiles} ${headers})
  target_link_libraries(${target} CnoidBodyPlugin)
else()
  # Stand alone build
  cmake_minimum_required(VERSION 3.1.0)
  project(StartupPlugin)
  find_package(Choreonoid REQUIRED)
  add_definitions(${CHOREONOID_COMPILE_DEFINITIONS})
  include_directories(${CHOREONOID_INCLUDE_DIRS})
  link_directories(${CHOREONOID_LIBRARY_DIRS})
  set(target CnoidStartupPlugin)
  make_gettext_mofiles(${target} mofiles)
  add_library(${target} SHARED  ${sources} ${mofiles} ${headers})
  target_link_libraries(${target} ${CHOREONOID_BODY_PLUGIN_LIBRARIES})
  install(TARGETS ${target} LIBRARY DESTINATION ${CHOREONOID_PLUGIN_DIR})
endif()
