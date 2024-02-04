if(NOT ENABLE_GUI)
  return()
endif()

set(sources
  BookmarkBar.cpp
  BookmarkManagerDialog.cpp
  BookmarkPlugin.cpp
  HistoryManager.cpp
  JoyKey.cpp
  KIOSKManager.cpp
  WorldLogManagerDialog.cpp
)

set(headers
  BookmarkBar.h
  BookmarkManagerDialog.h
  HistoryManager.h
  JoyKey.h
  KIOSKManager.h
  WorldLogManagerDialog.h
)

set(target CnoidBookmarkPlugin)
make_gettext_mofiles(${target} mofiles)
choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
target_link_libraries(${target} PUBLIC CnoidBodyPlugin)