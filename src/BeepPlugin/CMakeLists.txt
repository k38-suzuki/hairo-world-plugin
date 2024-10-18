set(sources
  BeepCommandItem.cpp
  BeepEventReader.cpp
  BeepItem.cpp
  BeepPlugin.cpp
)

set(headers
  BeepCommandItem.h
  BeepEventReader.h
  BeepItem.h
  exportdecl.h
)

choreonoid_make_header_public(BeepCommandItem.h)

set(target CnoidBeepPlugin)
choreonoid_make_gettext_mo_files(${target} mofiles)
choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
target_link_libraries(${target} PUBLIC CnoidBodyPlugin)