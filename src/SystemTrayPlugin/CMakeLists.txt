set(sources
  SystemTrayIcon.cpp
  SystemTrayManager.cpp
  SystemTrayPlugin.cpp
)

set(headers
  SystemTrayIcon.h
  SystemTrayManager.h
  exportdecl.h
)

choreonoid_make_header_public(SystemTrayIcon.h)
choreonoid_make_header_public(SystemTrayManager.h)

set(target CnoidSystemTrayPlugin)
make_gettext_mofiles(${target} mofiles)
choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
target_link_libraries(${target} PUBLIC CnoidBodyPlugin)