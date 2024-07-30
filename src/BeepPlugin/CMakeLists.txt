set(sources
  Beeper.cpp
  BeepItem.cpp
  BeepPlugin.cpp
  BeepView.cpp
)

set(headers
  Beeper.h
  BeepItem.h
  BeepView.h
  exportdecl.h
)

choreonoid_make_header_public(Beeper.h)
choreonoid_make_header_public(BeepView.h)

set(target CnoidBeepPlugin)
make_gettext_mofiles(${target} mofiles)
choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
target_link_libraries(${target} PUBLIC CnoidBodyPlugin)