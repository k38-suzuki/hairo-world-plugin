set(sources
  ArchiveListDialog.cpp
  BookmarkManager.cpp
  BookmarkPlugin.cpp
  HistoryManager.cpp
  JoyKey.cpp
  KIOSKManager.cpp
  ToolsMenu.cpp
  WorldLogManager.cpp
)

set(headers
  ArchiveListDialog.h
  BookmarkManager.h
  HistoryManager.h
  JoyKey.h
  KIOSKManager.h
  ToolsMenu.h
  WorldLogManager.h
  exportdecl.h
)

choreonoid_make_header_public(ToolsMenu.h)

set(target CnoidBookmarkPlugin)
make_gettext_mofiles(${target} mofiles)
choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
target_link_libraries(${target} PUBLIC CnoidBodyPlugin)