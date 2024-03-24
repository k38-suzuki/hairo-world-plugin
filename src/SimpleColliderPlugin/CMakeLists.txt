if(NOT ENABLE_GUI)
  return()
endif()

set(sources
  SimpleColliderItem.cpp
  SimpleColliderPlugin.cpp
)

set(headers
  SimpleColliderItem.h
  exportdecl.h
)

choreonoid_make_header_public(SimpleColliderItem.h)

set(target CnoidSimpleColliderPlugin)
make_gettext_mofiles(${target} mofiles)
choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
target_link_libraries(${target} PUBLIC CnoidBodyPlugin)