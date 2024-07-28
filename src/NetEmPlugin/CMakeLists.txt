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
make_gettext_mofiles(${target} mofiles)

if(CMAKE_PROJECT_NAME STREQUAL "Choreonoid")
  # Build inside the Choreonoid project
  choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
  target_link_libraries(${target} PUBLIC CnoidBodyPlugin CnoidSimpleColliderPlugin)
else()
  # Stand alone build
  add_library(${target} SHARED ${sources} ${mofiles} ${headers})
  target_link_libraries(${target} ${CHOREONOID_BODY_PLUGIN_LIBRARIES} CnoidSimpleColliderPlugin)
  install(TARGETS ${target} LIBRARY DESTINATION ${CHOREONOID_PLUGIN_DIR})
endif()