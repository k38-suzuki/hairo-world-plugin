set(sources
  ArchiveListDialog.cpp
  BookmarkBar.cpp
  BookmarkManager.cpp
  BookmarkPlugin.cpp
  HamburgerButton.cpp
  HistoryManager.cpp
  JoyKey.cpp
  KIOSKManager.cpp
  SystemTrayIcon.cpp
  ToolsUtil.cpp
  WorldLogManager.cpp
)

set(headers
  ArchiveListDialog.h
  BookmarkBar.h
  BookmarkManager.h
  HamburgerButton.h
  HistoryManager.h
  JoyKey.h
  KIOSKManager.h
  SystemTrayIcon.h
  ToolsUtil.h
  WorldLogManager.h
  exportdecl.h
)

choreonoid_make_header_public(BookmarkBar.h)
choreonoid_make_header_public(SystemTrayIcon.h)
choreonoid_make_header_public(ToolsUtil.h)

set(target CnoidBookmarkPlugin)
make_gettext_mofiles(${target} mofiles)
choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
target_link_libraries(${target} PUBLIC CnoidBodyPlugin CnoidMotionCapturePlugin CnoidGooglePlugin)