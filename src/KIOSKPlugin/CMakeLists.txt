
set(make_symlink false)
if(NOT CNOID_SOURCE_SHARE_DIR STREQUAL CNOID_BINARY_SHARE_DIR)
  if(UNIX)
    set(make_symlink true)
  endif()
  configure_file(project/layout_base.cnoid ${CNOID_BINARY_SHARE_DIR}/kiosk/layout_base.cnoid COPYONLY)
  configure_file(project/layout_kiosk.cnoid ${CNOID_BINARY_SHARE_DIR}/kiosk/layout_kiosk.cnoid COPYONLY)
endif()

install(FILES project/layout_base.cnoid DESTINATION ${CNOID_SHARE_SUBDIR}/project)
install(FILES project/layout_kiosk.cnoid DESTINATION ${CNOID_SHARE_SUBDIR}/project)

set(sources
    BookmarkWidget.cpp
    KIOSKManager.cpp
    KIOSKPlugin.cpp
    KIOSKView.cpp
    LogWidget.cpp
    SimpleSimulationView.cpp
    )

set(headers
    exportdecl.h
    BookmarkWidget.h
    KIOSKManager.h
    KIOSKView.h
    LogWidget.h
    SimpleSimulationView.h
    )

if(CMAKE_PROJECT_NAME STREQUAL "Choreonoid")
  # Build inside the Choreonoid project
  set(target CnoidKIOSKPlugin)
  make_gettext_mofiles(${target} mofiles)
  choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
  target_link_libraries(${target} CnoidBase CnoidBodyPlugin)
else()
  # Stand alone build
  cmake_minimum_required(VERSION 3.1.0)
  project(KIOSKPlugin)
  find_package(Choreonoid REQUIRED)
  add_definitions(${CHOREONOID_COMPILE_DEFINITIONS})
  include_directories(${CHOREONOID_INCLUDE_DIRS})
  link_directories(${CHOREONOID_LIBRARY_DIRS})
  set(target CnoidKIOSKPlugin)
  make_gettext_mofiles(${target} mofiles)
  add_library(${target} SHARED  ${sources} ${mofiles} HEADERS ${headers})
  target_link_libraries(${target} ${CHOREONOID_BASE_LIBRARIES} ${CHOREONOID_BODY_PLUGIN_LIBRARIES})
  install(TARGETS ${target} LIBRARY DESTINATION ${CHOREONOID_PLUGIN_DIR})
endif()
