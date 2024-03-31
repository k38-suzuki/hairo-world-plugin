set(sources
  AxisWidget.cpp
  OnScreenJoystickWidget.cpp
  JoystickLoggerItem.cpp
  JoystickStartPlugin.cpp
  OnScreenJoystickView.cpp
  JoystickTester.cpp
  KeyConfig.cpp
  JoystickStarter.cpp
  VirtualJoystickWidget.cpp
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
  VirtualJoystickWidget.h
  exportdecl.h
)

choreonoid_make_header_public(ConfigurableJoystick.h)

set(target CnoidJoystickStartPlugin)
make_gettext_mofiles(${target} mofiles)
choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
target_link_libraries(${target} PUBLIC CnoidBodyPlugin)