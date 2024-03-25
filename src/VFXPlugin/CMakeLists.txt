set(sources
  VisualEffects.cpp
  ImageGenerator.cpp
  NoisyCamera.cpp
  VFXColliderItem.cpp
  VFXVisionSimulatorItem.cpp
  VFXPlugin.cpp
)

set(headers
  VisualEffects.h
  ImageGenerator.h
  NoisyCamera.h
  VFXColliderItem.h
  VFXVisionSimulatorItem.h
  exportdecl.h
)

choreonoid_make_header_public(VisualEffects.h)
choreonoid_make_header_public(ImageGenerator.h)
choreonoid_make_header_public(NoisyCamera.h)

set(target CnoidVFXPlugin)
make_gettext_mofiles(${target} mofiles)
choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
target_link_libraries(${target} PUBLIC CnoidBodyPlugin CnoidSimpleColliderPlugin)