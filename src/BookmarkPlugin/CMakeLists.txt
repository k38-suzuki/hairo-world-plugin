set(sources
  BookmarkManager.cpp
  BookmarkPlugin.cpp
  HistoryManager.cpp
  JoyKey.cpp
  KIOSKManager.cpp
  WorldLogManager.cpp
)

set(headers
  BookmarkManager.h
  HistoryManager.h
  JoyKey.h
  KIOSKManager.h
  WorldLogManager.h
)

set(target CnoidBookmarkPlugin)
make_gettext_mofiles(${target} mofiles)
choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
target_link_libraries(${target} PUBLIC CnoidBodyPlugin)