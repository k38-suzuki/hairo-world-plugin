if(NOT ENABLE_GUI)
  return()
endif()

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

set(target CnoidBeepPlugin)
make_gettext_mofiles(${target} mofiles)
choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
target_link_libraries(${target} PUBLIC CnoidBodyPlugin)