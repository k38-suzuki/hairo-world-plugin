if(NOT ENABLE_GUI)
  return()
endif()

set(sources
  NetEm.cpp
  NetEmPlugin.cpp
  NetworkEmulator.cpp
  NetworkEmulatorItem.cpp
  TCColliderItem.cpp
)

set(headers
  NetEm.h
  NetworkEmulator.h
  NetworkEmulatorItem.h
  TCColliderItem.h
)

set(target CnoidNetEmPlugin)
make_gettext_mofiles(${target} mofiles)
choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
target_link_libraries(${target} PUBLIC CnoidBodyPlugin CnoidSimpleColliderPlugin)