
set(sources
    LayoutSwitcher.cpp
    LayoutSwitcherBar.cpp
    LayoutSwitcherPlugin.cpp
    LayoutSwitcherView.cpp
    )

set(headers
    LayoutSwitcher.h
    LayoutSwitcherBar.h
    LayoutSwitcherView.h
    exportdecl.h
    )

if(CMAKE_PROJECT_NAME STREQUAL "Choreonoid")
  # Build inside the Choreonoid project
  set(target CnoidLayoutSwitcherPlugin)
  make_gettext_mofiles(${target} mofiles)
  choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
  target_link_libraries(${target} PUBLIC CnoidBodyPlugin)
else()
  # Stand alone build
  set(target CnoidLayoutSwitcherPlugin)
  make_gettext_mofiles(${target} mofiles)
  add_library(${target} SHARED  ${sources} ${mofiles} ${headers})
  target_link_libraries(${target} ${CHOREONOID_BODY_PLUGIN_LIBRARIES})
  install(TARGETS ${target} LIBRARY DESTINATION ${CHOREONOID_PLUGIN_DIR})
endif()
