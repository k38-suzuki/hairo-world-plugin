
option(BUILD_MARKERDETECT_PLUGIN "Building a sample plugin of the marker detection" OFF)
if(NOT BUILD_MARKERDETECT_PLUGIN)
    return()
endif()

set(sources
    ScopeDevice.cpp
    MarkerDetectorItem.cpp
    MarkerDetectPlugin.cpp
   )

set(headers
    exportdecl.h
    gettext.h
    ScopeDevice.h
    MarkerDetectorItem.h
    )

choreonoid_make_header_public(ScopeDevice.h)

if(CMAKE_PROJECT_NAME STREQUAL "Choreonoid")
  # Build inside the Choreonoid project
  set(target CnoidMarkerDetectPlugin)
  make_gettext_mofiles(${target} mofiles)
  choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
  target_link_libraries(${target} CnoidBodyPlugin CnoidMotionCapturePlugin)
  apply_common_setting_for_plugin(${target})
else()
  # Stand alone build
  cmake_minimum_required(VERSION 3.1.0)
  project(MarkerDetectPlugin)
  find_package(Choreonoid REQUIRED)
  add_definitions(${CHOREONOID_DEFINITIONS})
  include_directories(${CHOREONOID_INCLUDE_DIRS})
  link_directories(${CHOREONOID_LIBRARY_DIRS})
  set(target CnoidMarkerDetectPlugin)
  make_gettext_mofiles(${target} mofiles)
  add_library(${target} SHARED ${sources} ${mofiles} HEADERS ${headers})
  target_link_libraries(${target} ${CHOREONOID_BODY_PLUGIN_LIBRARIES} CnoidMotionCapturePlugin)
  install(TARGETS ${target} LIBRARY DESTINATION ${CHOREONOID_PLUGIN_DIR})
endif()
