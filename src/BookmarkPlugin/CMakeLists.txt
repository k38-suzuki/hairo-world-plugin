set(sources
  ArchiveListDialog.cpp
  BookmarkManager.cpp
  BookmarkPlugin.cpp
  HamburgerMenu.cpp
  HistoryManager.cpp
  JoyKey.cpp
  KIOSKManager.cpp
  LayoutManager.cpp
  SystemTrayIcon.cpp
  WorldLogManager.cpp
)

set(headers
  ArchiveListDialog.h
  BookmarkManager.h
  HamburgerMenu.h
  HistoryManager.h
  JoyKey.h
  KIOSKManager.h
  LayoutManager.h
  SystemTrayIcon.h
  WorldLogManager.h
  exportdecl.h
)

choreonoid_make_header_public(SystemTrayIcon.h)
choreonoid_make_header_public(HamburgerMenu.h)

set(target CnoidBookmarkPlugin)
make_gettext_mofiles(${target} mofiles)
choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
target_link_libraries(${target} PUBLIC CnoidBodyPlugin CnoidMotionCapturePlugin CnoidGooglePlugin)