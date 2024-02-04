if(NOT ENABLE_GUI)
  return()
endif()

set(sources
  LayoutSwitcher.cpp
  LayoutSwitcherPlugin.cpp
  LayoutSwitcherView.cpp
)

set(headers
  LayoutSwitcher.h
  LayoutSwitcherView.h
  exportdecl.h
)

set(target CnoidLayoutSwitcherPlugin)
make_gettext_mofiles(${target} mofiles)
choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
target_link_libraries(${target} PUBLIC CnoidBodyPlugin)
