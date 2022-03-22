
set(sources
    AxisWidget.cpp
    DraggableJoystickView.cpp
    JoystickLoggerItem.cpp
    JoystickStatusPlugin.cpp
    JoystickStatusView.cpp
    KeyConfigView.cpp
    SimulationManager.cpp
    )

set(headers
    AxisWidget.h
    ConfigurableJoystick.h
    DraggableJoystickView.h
    exportdecl.h
    gettext.h
    JoystickLoggerItem.h
    JoystickStatusView.h
    KeyConfigView.h
    SimulationManager.h
    )

choreonoid_make_header_public(ConfigurableJoystick.h)

if(CMAKE_PROJECT_NAME STREQUAL "Choreonoid")
  # Build inside the Choreonoid project
  set(target CnoidJoystickStatusPlugin)
  make_gettext_mofiles(${target} mofiles)
  choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
  target_link_libraries(${target} CnoidBodyPlugin)
else()
  # Stand alone build
  cmake_minimum_required(VERSION 3.1.0)
  project(JoystickStatusPlugin)
  find_package(Choreonoid REQUIRED)
  add_definitions(${CHOREONOID_COMPILE_DEFINITIONS})
  include_directories(${CHOREONOID_INCLUDE_DIRS})
  link_directories(${CHOREONOID_LIBRARY_DIRS})
  set(target CnoidJoystickStatusPlugin)
  make_gettext_mofiles(${target} mofiles)
  add_library(${target} SHARED  ${sources} ${mofiles} HEADERS ${headers})
  target_link_libraries(${target} ${CHOREONOID_BODY_PLUGIN_LIBRARIES})
  install(TARGETS ${target} LIBRARY DESTINATION ${CHOREONOID_PLUGIN_DIR})
endif()
