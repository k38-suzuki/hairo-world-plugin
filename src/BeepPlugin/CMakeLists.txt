set(sources
  Beeper.cpp
  BeepItem.cpp
  BeepPlugin.cpp
  BeepView.cpp
)

set(headers
  Beeper.h
  BeepItem.h
  BeepView.h
  exportdecl.h
)

choreonoid_make_header_public(Beeper.h)
choreonoid_make_header_public(BeepView.h)

set(target CnoidBeepPlugin)
make_gettext_mofiles(${target} mofiles)

if(CMAKE_PROJECT_NAME STREQUAL "Choreonoid")
  # Build inside the Choreonoid project
  choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
  target_link_libraries(${target} PUBLIC CnoidBodyPlugin)
else()
  # Stand alone build
  add_library(${target} SHARED ${sources} ${mofiles} ${headers})
  target_link_libraries(${target} ${CHOREONOID_BODY_PLUGIN_LIBRARIES})
  install(TARGETS ${target} LIBRARY DESTINATION ${CHOREONOID_PLUGIN_DIR})
endif()