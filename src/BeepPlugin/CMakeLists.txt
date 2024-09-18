set(sources
  Beeper.cpp
  BeepCommandItem.cpp
  BeepEventReader.cpp
  BeepItem.cpp
  BeepPlugin.cpp
)

set(headers
  Beeper.h
  BeepCommandItem.h
  BeepEventReader.h
  BeepItem.h
  exportdecl.h
)

choreonoid_make_header_public(Beeper.h)
choreonoid_make_header_public(BeepCommandItem.h)

set(target CnoidBeepPlugin)
make_gettext_mofiles(${target} mofiles)
choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
target_link_libraries(${target} PUBLIC CnoidBodyPlugin)