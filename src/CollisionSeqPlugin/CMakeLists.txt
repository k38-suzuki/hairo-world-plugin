if(NOT ENABLE_GUI)
  return()
endif()

set(sources
  CollisionVisualizerItem.cpp
  CollisionSensor.cpp
  CollisionSeqPlugin.cpp
)

set(headers
  CollisionVisualizerItem.h
  CollisionSensor.h
  exportdecl.h
)

choreonoid_make_header_public(CollisionSensor.h)

set(target CnoidCollisionSeqPlugin)
make_gettext_mofiles(${target} mofiles)
choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
target_link_libraries(${target} PUBLIC CnoidBodyPlugin)