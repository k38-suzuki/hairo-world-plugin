set(sources
  CFDPlugin.cpp
  CFDSimulatorItem.cpp
  Rotor.cpp
  Thruster.cpp
  WingDevice.cpp
)

set(headers
  CFDSimulatorItem.h
  Rotor.h
  Thruster.h
  WingDevice.h
  exportdecl.h
)

choreonoid_make_header_public(Rotor.h)
choreonoid_make_header_public(Thruster.h)
choreonoid_make_header_public(WingDevice.h)

set(target CnoidCFDPlugin)
make_gettext_mofiles(${target} mofiles)
choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
target_link_libraries(${target} PUBLIC CnoidBodyPlugin CnoidSimpleColliderPlugin)