set(sources
  CollisionSeqLoggerItem.cpp
#  CollisionVisualizerItem.cpp
  MotionCaptureItem.cpp
  MotionCapturePlugin.cpp
  PassiveMarker.cpp
  LoggerUtil.cpp
)

set(headers
  CollisionSeqLoggerItem.h
#  CollisionVisualizerItem.h
  MotionCaptureItem.h
  PassiveMarker.h
  LoggerUtil.h
  exportdecl.h
)

choreonoid_make_header_public(PassiveMarker.h)
choreonoid_make_header_public(LoggerUtil.h)

set(target CnoidMotionCapturePlugin)
choreonoid_make_gettext_mo_files(${target} mofiles)
choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
target_link_libraries(${target} PUBLIC CnoidBodyPlugin)