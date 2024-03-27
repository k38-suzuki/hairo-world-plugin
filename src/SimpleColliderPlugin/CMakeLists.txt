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

set(target CnoidSimpleColliderPlugin)
make_gettext_mofiles(${target} mofiles)
choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
target_link_libraries(${target} PUBLIC CnoidBodyPlugin)