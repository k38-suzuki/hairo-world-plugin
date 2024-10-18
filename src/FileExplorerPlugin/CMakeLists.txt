set(sources
  BodyLocator.cpp
  FileExplorer.cpp
  FileExplorerPlugin.cpp
)

set(headers
  BodyLocator.h
  FileExplorer.h
  exportdecl.h
)

set(target CnoidFileExplorerPlugin)
choreonoid_make_gettext_mo_files(${target} mofiles)
choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
target_link_libraries(${target} PUBLIC CnoidBodyPlugin)