
set(sources
    ImageGenerator.cpp
    VEAreaItem.cpp
    VisualEffectorItem.cpp
    VisualEffectPlugin.cpp
    )

set(headers
    ImageGenerator.h
    VEAreaItem.h
    VisualEffectorItem.h
    exportdecl.h
    )

choreonoid_make_header_public(ImageGenerator.h)

if(CMAKE_PROJECT_NAME STREQUAL "Choreonoid")
  # Build inside the Choreonoid project
  set(target CnoidVisualEffectPlugin)
  make_gettext_mofiles(${target} mofiles)
  choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
  target_link_libraries(${target} PUBLIC CnoidBodyPlugin CnoidCFDPlugin)
  apply_common_setting_for_plugin(${target})
else()
  # Stand alone build
  set(target CnoidVisualEffectPlugin)
  make_gettext_mofiles(${target} mofiles)
  add_library(${target} SHARED ${sources} ${mofiles} ${headers})
  target_link_libraries(${target} ${CHOREONOID_BODY_PLUGIN_LIBRARIES} CnoidCFDPlugin)
  install(TARGETS ${target} LIBRARY DESTINATION ${CHOREONOID_PLUGIN_DIR})
endif()