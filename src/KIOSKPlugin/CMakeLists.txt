
set(sources
    JoyKey.cpp
    KIOSKManager.cpp
    KIOSKPlugin.cpp
    SimpleSimulationView.cpp
    SimpleTimeWidget.cpp
    )

set(headers
    exportdecl.h
    JoyKey.h
    KIOSKManager.h
    SimpleSimulationView.h
    SimpleTimeWidget.h
    )

if(CMAKE_PROJECT_NAME STREQUAL "Choreonoid")
  # Build inside the Choreonoid project
  set(target CnoidKIOSKPlugin)
  make_gettext_mofiles(${target} mofiles)
  choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
  target_link_libraries(${target} CnoidBase CnoidBodyPlugin CnoidBookmarkPlugin)
else()
  # Stand alone build
  set(target CnoidKIOSKPlugin)
  make_gettext_mofiles(${target} mofiles)
  add_library(${target} SHARED  ${sources} ${mofiles} ${headers})
  target_link_libraries(${target} ${CHOREONOID_BASE_LIBRARIES} ${CHOREONOID_BODY_PLUGIN_LIBRARIES} CnoidBookmarkPlugin)
  install(TARGETS ${target} LIBRARY DESTINATION ${CHOREONOID_PLUGIN_DIR})
endif()