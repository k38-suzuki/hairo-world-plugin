set(sources
  CollisionVisualizerItem.cpp
  MotionCaptureItem.cpp
  MotionCapturePlugin.cpp
  PassiveMarker.cpp
)

set(headers
  CollisionVisualizerItem.h
  MotionCaptureItem.h
  PassiveMarker.h
  exportdecl.h
)

choreonoid_make_header_public(PassiveMarker.h)

set(target CnoidMotionCapturePlugin)
make_gettext_mofiles(${target} mofiles)
choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
target_link_libraries(${target} PUBLIC CnoidBodyPlugin)