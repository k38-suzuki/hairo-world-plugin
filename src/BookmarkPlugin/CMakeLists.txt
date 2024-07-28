set(sources
  ArchiveListDialog.cpp
  BookmarkBar.cpp
  BookmarkManager.cpp
  BookmarkPlugin.cpp
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

if(CMAKE_PROJECT_NAME STREQUAL "Choreonoid")
  # Build inside the Choreonoid project
  choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
  target_link_libraries(${target} PUBLIC CnoidBodyPlugin)
else()
  # Stand alone build
  add_library(${target} SHARED ${sources} ${mofiles} ${headers})
  target_link_libraries(${target} ${CHOREONOID_BODY_PLUGIN_LIBRARIES})
  install(TARGETS ${target} LIBRARY DESTINATION ${CHOREONOID_PLUGIN_DIR})
endif()