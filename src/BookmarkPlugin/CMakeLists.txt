set(sources
  ArchiveListDialog.cpp
  BookmarkBar.cpp
  BookmarkManager.cpp
  BookmarkPlugin.cpp
  HistoryManager.cpp
  JoyKey.cpp
  KIOSKManager.cpp
  ToolsUtil.cpp
  WorldLogManager.cpp
)

set(headers
  ArchiveListDialog.h
  BookmarkBar.h
  BookmarkManager.h
  HistoryManager.h
  JoyKey.h
  KIOSKManager.h
  ToolsUtil.h
  WorldLogManager.h
  exportdecl.h
)

choreonoid_make_header_public(BookmarkBar.h)
choreonoid_make_header_public(ToolsUtil.h)

set(target CnoidBookmarkPlugin)
make_gettext_mofiles(${target} mofiles)
choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
target_link_libraries(${target} PUBLIC CnoidBodyPlugin)