set(sources
  ImageGenerator.cpp
  NoisyCamera.cpp
  VFXConverter.cpp
  VFXPlugin.cpp
  VFXVisionSimulatorItem.cpp
  VisualEffects.cpp
)

set(headers
  ImageGenerator.h
  NoisyCamera.h
  VFXConverter.h
  VFXVisionSimulatorItem.h
  VisualEffects.h
  exportdecl.h
)

choreonoid_make_header_public(VisualEffects.h)
choreonoid_make_header_public(ImageGenerator.h)
choreonoid_make_header_public(NoisyCamera.h)
choreonoid_make_header_public(VFXConverter.h)

set(target CnoidVFXPlugin)
make_gettext_mofiles(${target} mofiles)
choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
target_link_libraries(${target} PUBLIC CnoidBodyPlugin CnoidSimpleColliderPlugin)