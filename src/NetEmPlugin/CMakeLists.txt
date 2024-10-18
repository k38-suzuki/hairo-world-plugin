set(sources
  NetEm.cpp
  NetEmPlugin.cpp
  NetworkEmulator.cpp
  NetworkEmulatorItem.cpp
)

set(headers
  NetEm.h
  NetworkEmulator.h
  NetworkEmulatorItem.h
)

set(target CnoidNetEmPlugin)
choreonoid_make_gettext_mo_files(${target} mofiles)
choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
target_link_libraries(${target} PUBLIC CnoidBodyPlugin CnoidSimpleColliderPlugin CnoidBookmarkPlugin)