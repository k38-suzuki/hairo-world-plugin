
set(sources
    SystemTrayIcon.cpp
    SystemTrayManager.cpp
    SystemTrayPlugin.cpp
    )

set(headers
    exportdecl.h
    gettext.h
    SystemTrayIcon.h
    SystemTrayManager.h
    )

choreonoid_make_header_public(SystemTrayIcon.h)
choreonoid_make_header_public(SystemTrayManager.h)

if(CMAKE_PROJECT_NAME STREQUAL "Choreonoid")
  # Build inside the Choreonoid project
  set(target CnoidSystemTrayPlugin)
  make_gettext_mofiles(${target} mofiles)
  choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
  target_link_libraries(${target} CnoidBodyPlugin)
else()
  # Stand alone build
  set(target CnoidSystemTrayPlugin)
  make_gettext_mofiles(${target} mofiles)
  add_library(${target} SHARED ${sources} ${mofiles} ${headers})
  target_link_libraries(${target} ${CHOREONOID_BODY_PLUGIN_LIBRARIES})
  install(TARGETS ${target} LIBRARY DESTINATION ${CHOREONOID_PLUGIN_DIR})
endif()