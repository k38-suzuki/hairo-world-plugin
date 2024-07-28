set(sources
  CustomEffects.cpp
  MultiColliderItem.cpp
  MultiColliderItemCustomization.cpp
  SimpleColliderItem.cpp
  SimpleColliderPlugin.cpp
)

set(headers
  CustomEffects.h
  MultiColliderItem.h
  SimpleColliderItem.h
  exportdecl.h
)

choreonoid_make_header_public(SimpleColliderItem.h)
choreonoid_make_header_public(MultiColliderItem.h)
choreonoid_make_header_public(CustomEffects.h)

set(target CnoidSimpleColliderPlugin)
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