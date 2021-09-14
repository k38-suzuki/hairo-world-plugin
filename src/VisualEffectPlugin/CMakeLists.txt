
set(sources
    ImageGenerator.cpp
    VisualEffect.cpp
    VisualEffectDialog.cpp
    VisualEffectorItem.cpp
    VisualEffectPlugin.cpp
    )

set(headers
    ImageGenerator.h
    VisualEffect.h
    VisualEffectDialog.h
    VisualEffectorItem.h
    exportdecl.h
    gettext.h
    )

if(CMAKE_PROJECT_NAME STREQUAL "Choreonoid")
  # Build inside the Choreonoid project
  set(target CnoidVisualEffectPlugin)
  make_gettext_mofiles(${target} mofiles)
  add_cnoid_plugin(${target} SHARED ${sources} ${mofiles} ${headers})
  target_link_libraries(${target} CnoidBodyPlugin)
  apply_common_setting_for_plugin(${target})
else()
  # Stand alone build
  cmake_minimum_required(VERSION 3.1.0)
  project(VisualEffectPlugin)
  find_package(Choreonoid REQUIRED)
  add_definitions(${CHOREONOID_DEFINITIONS})
  include_directories(${CHOREONOID_INCLUDE_DIRS})
  link_directories(${CHOREONOID_LIBRARY_DIRS})
  set(target CnoidVisualEffectPlugin)
  make_gettext_mofiles(${target} mofiles)
  add_library(${target} SHARED ${sources} ${mofiles} ${headers})
  target_link_libraries(${target} ${CHOREONOID_BODY_PLUGIN_LIBRARIES})
  install(TARGETS ${target} LIBRARY DESTINATION ${CHOREONOID_PLUGIN_DIR})
endif()
