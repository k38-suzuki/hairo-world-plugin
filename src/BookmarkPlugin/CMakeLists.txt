set(sources
  ArchiveListWidget.cpp
  BookmarkManager.cpp
  BookmarkPlugin.cpp
  HamburgerMenu.cpp
  HistoryManager.cpp
  JoyKey.cpp
  KIOSKManager.cpp
  LayoutManager.cpp
  SystemTrayIcon.cpp
  WorldLogManager.cpp
  ListedWidget.cpp
  ProjectListedDialog.cpp
)

set(headers
  ArchiveListWidget.h
  BookmarkManager.h
  HamburgerMenu.h
  HistoryManager.h
  JoyKey.h
  KIOSKManager.h
  LayoutManager.h
  SystemTrayIcon.h
  WorldLogManager.h
  ListedWidget.h
  ProjectListedDialog.h
  exportdecl.h
)

choreonoid_make_header_public(SystemTrayIcon.h)
choreonoid_make_header_public(HamburgerMenu.h)
choreonoid_make_header_public(ListedWidget.h)

choreonoid_qt_add_resources(RC_SRCS BookmarkPlugin.qrc)
set(target CnoidBookmarkPlugin)
choreonoid_make_gettext_mo_files(${target} mofiles)
choreonoid_add_plugin(${target} ${sources} ${mofiles} ${RC_SRCS} HEADERS ${headers})
target_link_libraries(${target} PUBLIC CnoidBodyPlugin CnoidMotionCapturePlugin)