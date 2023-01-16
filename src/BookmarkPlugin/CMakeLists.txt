
set(sources
    BookmarkBar.cpp
    BookmarkManager.cpp
    BookmarkPlugin.cpp
    HistoryManager.cpp
    JoyKey.cpp
    KIOSKManager.cpp
    WorldLogManager.cpp
    )

set(headers
    BookmarkBar.h
    BookmarkManager.h
    gettext.h
    HistoryManager.h
    JoyKey.h
    KIOSKManager.h
    WorldLogManager.h
    )

if(CMAKE_PROJECT_NAME STREQUAL "Choreonoid")
  # Build inside the Choreonoid project
  set(target CnoidBookmarkPlugin)
  make_gettext_mofiles(${target} mofiles)
  choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
  target_link_libraries(${target} CnoidBodyPlugin)
  apply_common_setting_for_plugin(${target})
else()
  # Stand alone build
  set(target CnoidBookmarkPlugin)
  make_gettext_mofiles(${target} mofiles)
  add_library(${target} SHARED ${sources} ${mofiles} ${headers})
  target_link_libraries(${target} ${CHOREONOID_BODY_PLUGIN_LIBRARIES})
  install(TARGETS ${target} LIBRARY DESTINATION ${CHOREONOID_PLUGIN_DIR})
endif()