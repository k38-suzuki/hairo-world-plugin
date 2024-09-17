set(sources
  Beeper.cpp
  BeepEventReader.cpp
  BeepItem.cpp
  BeepPlugin.cpp
)

set(headers
  Beeper.h
  BeepEventReader.h
  BeepItem.h
  exportdecl.h
)

choreonoid_make_header_public(Beeper.h)

set(target CnoidBeepPlugin)
make_gettext_mofiles(${target} mofiles)
choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
target_link_libraries(${target} PUBLIC CnoidBodyPlugin)