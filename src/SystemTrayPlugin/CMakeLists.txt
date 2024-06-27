set(sources
  SystemTrayIcon.cpp
  SystemTrayPlugin.cpp
)

set(headers
  SystemTrayIcon.h
  exportdecl.h
)

choreonoid_make_header_public(SystemTrayIcon.h)

set(target CnoidSystemTrayPlugin)
make_gettext_mofiles(${target} mofiles)
choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
target_link_libraries(${target} PUBLIC CnoidBodyPlugin)