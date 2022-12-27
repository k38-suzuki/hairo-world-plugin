
set(sources
    AreaItem.cpp
    FluidAreaItem.cpp
    CFDPlugin.cpp
    CFDSimulatorItem.cpp
    Rotor.cpp
    Thruster.cpp
    )

set(headers
    AreaItem.h
    FluidAreaItem.h
    CFDSimulatorItem.h
    Rotor.h
    Thruster.h
    exportdecl.h
    gettext.h
    )

choreonoid_make_header_public(AreaItem.h)
choreonoid_make_header_public(FluidAreaItem.h)
choreonoid_make_header_public(Rotor.h)
choreonoid_make_header_public(Thruster.h)

if(CMAKE_PROJECT_NAME STREQUAL "Choreonoid")
  # Build inside the Choreonoid project
  set(target CnoidCFDPlugin)
  make_gettext_mofiles(${target} mofiles)
  choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
  target_link_libraries(${target} CnoidBodyPlugin)
  apply_common_setting_for_plugin(${target})
else()
  # Stand alone build
  set(target CnoidCFDPlugin)
  make_gettext_mofiles(${target} mofiles)
  add_library(${target} SHARED ${sources} ${mofiles} ${headers})
  target_link_libraries(${target} ${CHOREONOID_BODY_PLUGIN_LIBRARIES})
  install(TARGETS ${target} LIBRARY DESTINATION ${CHOREONOID_PLUGIN_DIR})
endif()