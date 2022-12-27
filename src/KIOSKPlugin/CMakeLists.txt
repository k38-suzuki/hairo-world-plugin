
set(make_symlink false)
if(NOT CHOREONOID_SOURCE_SHARE_DIR STREQUAL CHOREONOID_BINARY_SHARE_DIR)
  if(UNIX)
    set(make_symlink true)
  endif()
  configure_file(project/layout_base.cnoid ${CHOREONOID_BINARY_SHARE_DIR}/kiosk/layout_base.cnoid COPYONLY)
  configure_file(project/layout_kiosk.cnoid ${CHOREONOID_BINARY_SHARE_DIR}/kiosk/layout_kiosk.cnoid COPYONLY)
endif()

install(FILES project/layout_base.cnoid DESTINATION ${CHOREONOID_SHARE_SUBDIR}/project)
install(FILES project/layout_kiosk.cnoid DESTINATION ${CHOREONOID_SHARE_SUBDIR}/project)

set(sources
    BookmarkWidget.cpp
    JoyKey.cpp
    KIOSKManager.cpp
    KIOSKPlugin.cpp
    KIOSKView.cpp
    LogWidget.cpp
    SimpleSimulationView.cpp
    SimpleTimeWidget.cpp
    )

set(headers
    exportdecl.h
    BookmarkWidget.h
    JoyKey.h
    KIOSKManager.h
    KIOSKView.h
    LogWidget.h
    SimpleSimulationView.h
    SimpleTimeWidget.h
    )

QT5_ADD_RESOURCES(RC_SRCS KIOSKPlugin.qrc)

if(CMAKE_PROJECT_NAME STREQUAL "Choreonoid")
  # Build inside the Choreonoid project
  set(target CnoidKIOSKPlugin)
  make_gettext_mofiles(${target} mofiles)
  choreonoid_add_plugin(${target} ${sources} ${mofiles} ${RC_SRCS} HEADERS ${headers})
  target_link_libraries(${target} CnoidBase CnoidBodyPlugin)
else()
  # Stand alone build
  set(target CnoidKIOSKPlugin)
  make_gettext_mofiles(${target} mofiles)
  choreonoid_add_library(${target} SHARED  ${sources} ${mofiles} ${RC_SRCS} HEADERS ${headers})
  target_link_libraries(${target} ${CHOREONOID_BASE_LIBRARIES} ${CHOREONOID_BODY_PLUGIN_LIBRARIES})
  install(TARGETS ${target} LIBRARY DESTINATION ${CHOREONOID_PLUGIN_DIR})
endif()