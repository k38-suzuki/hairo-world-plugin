set(sources
  BodyLocator.cpp
  FileExplorerPlugin.cpp
)

set(headers
  BodyLocator.h
  exportdecl.h
)

set(target CnoidFileExplorerPlugin)
make_gettext_mofiles(${target} mofiles)
choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
target_link_libraries(${target} PUBLIC CnoidBodyPlugin)