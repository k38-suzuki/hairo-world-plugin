if(NOT ENABLE_GUI)
  return()
endif()

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

set(target CnoidVisualEffectPlugin)
make_gettext_mofiles(${target} mofiles)
choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
target_link_libraries(${target} PUBLIC CnoidBodyPlugin CnoidCFDPlugin)