set(sources
  AxisWidget.cpp
  OnScreenJoystickWidget.cpp
  IntervalStarterBar.cpp
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
  DelayedJoystick.h
  OnScreenJoystickWidget.h
  IntervalStarterBar.h
  JoystickLoggerItem.h
  OnScreenJoystickView.h
  JoystickTester.h
  KeyConfig.h
  JoystickStarter.h
  VirtualJoystickWidget.h
  exportdecl.h
)

choreonoid_qt_add_resources(RC_SRCS JoystickStartPlugin.qrc)

choreonoid_make_header_public(ConfigurableJoystick.h)
choreonoid_make_header_public(DelayedJoystick.h)

set(target CnoidJoystickStartPlugin)
make_gettext_mofiles(${target} mofiles)
choreonoid_add_plugin(${target} ${sources} ${mofiles} ${RC_SRCS} HEADERS ${headers})
target_link_libraries(${target} PUBLIC CnoidBodyPlugin CnoidMotionCapturePlugin)