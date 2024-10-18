set(sources
  AxisWidget.cpp
  OnScreenJoystickWidget.cpp
  IntervalTimer.cpp
  JoystickLoggerItem.cpp
  JoystickStartPlugin.cpp
  OnScreenJoystickView.cpp
  JoystickTester.cpp
  JoystickStarter.cpp
  VirtualJoystickWidget.cpp
)

set(headers
  AxisWidget.h
  ConfigurableJoystick.h
  DelayedJoystick.h
  OnScreenJoystickWidget.h
  IntervalTimer.h
  JoystickLoggerItem.h
  OnScreenJoystickView.h
  JoystickTester.h
  JoystickStarter.h
  VirtualJoystickWidget.h
  exportdecl.h
)

choreonoid_make_header_public(ConfigurableJoystick.h)
choreonoid_make_header_public(DelayedJoystick.h)

set(target CnoidJoystickStartPlugin)
choreonoid_make_gettext_mo_files(${target} mofiles)
choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
target_link_libraries(${target} PUBLIC CnoidBodyPlugin CnoidMotionCapturePlugin CnoidBookmarkPlugin)