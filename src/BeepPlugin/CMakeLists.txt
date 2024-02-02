
option(BUILD_BEEP_PLUGIN "Building BeepPlugin" OFF)
if(NOT BUILD_BEEP_PLUGIN)
  return()
endif()

set(sources
    BeepItem.cpp
    BeepPlugin.cpp
    BeepView.cpp
    BeepWidget.cpp
    )

set(headers
    BeepItem.h
    BeepView.h
    BeepWidget.h
    exportdecl.h
    )

choreonoid_make_header_public(BeepView.h)
choreonoid_make_header_public(BeepWidget.h)

if(CMAKE_PROJECT_NAME STREQUAL "Choreonoid")
  # Build inside the Choreonoid project
  set(target CnoidBeepPlugin)
  make_gettext_mofiles(${target} mofiles)
  choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
  target_link_libraries(${target} PUBLIC CnoidBodyPlugin)
else()
  # Stand alone build
  set(target CnoidBeepPlugin)
  make_gettext_mofiles(${target} mofiles)
  add_library(${target} SHARED ${sources} ${mofiles} ${headers})
  target_link_libraries(${target} ${CHOREONOID_BODY_PLUGIN_LIBRARIES})
  install(TARGETS ${target} LIBRARY DESTINATION ${CHOREONOID_PLUGIN_DIR})
endif()