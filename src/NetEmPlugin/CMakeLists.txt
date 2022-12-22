
set(sources
    NetEm.cpp
    NetEmPlugin.cpp
    NetworkEmulator.cpp
    NetworkEmulatorItem.cpp
    TCAreaItem.cpp
    )

set(headers
    gettext.h
    NetEm.h
    NetworkEmulator.h
    NetworkEmulatorItem.h
    TCAreaItem.h
    )

if(CMAKE_PROJECT_NAME STREQUAL "Choreonoid")
  # Build inside the Choreonoid project
  set(target CnoidNetEmPlugin)
  make_gettext_mofiles(${target} mofiles)
  choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
  target_link_libraries(${target} CnoidBodyPlugin CnoidCFDPlugin)
  apply_common_setting_for_plugin(${target})
else()
  # Stand alone build
  set(target CnoidNetEmPlugin)
  make_gettext_mofiles(${target} mofiles)
  choreonoid_add_library(${target} SHARED ${sources} ${mofiles} HEADERS ${headers})
  target_link_libraries(${target} ${CHOREONOID_BODY_PLUGIN_LIBRARIES} CnoidCFDPlugin)
  install(TARGETS ${target} LIBRARY DESTINATION ${CHOREONOID_PLUGIN_DIR})
endif()