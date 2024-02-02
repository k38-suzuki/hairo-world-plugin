
set(sources
    AxisWidget.cpp
    OnScreenJoystickWidget.cpp
    JoystickLoggerItem.cpp
    JoystickStartPlugin.cpp
    OnScreenJoystickView.cpp
    JoystickTester.cpp
    KeyConfig.cpp
    JoystickStarter.cpp
    SimpleSimulationView.cpp
    SimpleTimeWidget.cpp
    )

set(headers
    AxisWidget.h
    ConfigurableJoystick.h
    OnScreenJoystickWidget.h
    JoystickLoggerItem.h
    OnScreenJoystickView.h
    JoystickTester.h
    KeyConfig.h
    JoystickStarter.h
    SimpleSimulationView.h
    SimpleTimeWidget.h
    exportdecl.h
    )

choreonoid_make_header_public(ConfigurableJoystick.h)

if(CMAKE_PROJECT_NAME STREQUAL "Choreonoid")
  # Build inside the Choreonoid project
  set(target CnoidJoystickStartPlugin)
  make_gettext_mofiles(${target} mofiles)
  choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
  target_link_libraries(${target} PUBLIC CnoidBodyPlugin)
else()
  # Stand alone build
  set(target CnoidJoystickStartPlugin)
  make_gettext_mofiles(${target} mofiles)
  add_library(${target} SHARED  ${sources} ${mofiles} ${headers})
  target_link_libraries(${target} ${CHOREONOID_BODY_PLUGIN_LIBRARIES})
  install(TARGETS ${target} LIBRARY DESTINATION ${CHOREONOID_PLUGIN_DIR})
endif()