
set(sources
    AxisWidget.cpp
    OnScreenJoystickView.cpp
    JoystickLoggerItem.cpp
    JoystickStatusPlugin.cpp
    JoystickStatusView.cpp
    JoystickTester.cpp
    KeyConfig.cpp
    SimulationManager.cpp
    SimpleSimulationView.cpp
    SimpleTimeWidget.cpp
    )

set(headers
    AxisWidget.h
    ConfigurableJoystick.h
    OnScreenJoystickView.h
    exportdecl.h
    gettext.h
    JoystickLoggerItem.h
    JoystickStatusView.h
    JoystickTester.h
    KeyConfig.h
    SimulationManager.h
    SimpleSimulationView.h
    SimpleTimeWidget.h
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
  set(target CnoidJoystickStatusPlugin)
  make_gettext_mofiles(${target} mofiles)
  add_library(${target} SHARED  ${sources} ${mofiles} ${headers})
  target_link_libraries(${target} ${CHOREONOID_BODY_PLUGIN_LIBRARIES})
  install(TARGETS ${target} LIBRARY DESTINATION ${CHOREONOID_PLUGIN_DIR})
endif()