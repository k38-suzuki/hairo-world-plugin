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
choreonoid_make_gettext_mo_files(${target} mofiles)
choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
target_link_libraries(${target} PUBLIC CnoidBodyPlugin)

include(ChoreonoidSimpleColliderBuildFunctions.cmake)
if(CHOREONOID_INSTALL_SDK)
  install(FILES ChoreonoidSimpleColliderBuildFunctions.cmake DESTINATION ${CHOREONOID_CMAKE_CONFIG_SUBDIR}/ext)
endif()