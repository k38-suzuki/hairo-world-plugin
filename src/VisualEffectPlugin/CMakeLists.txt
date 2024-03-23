if(NOT ENABLE_GUI)
  return()
endif()

set(sources
  CameraEffect.cpp
  ImageGenerator.cpp
  NoisyCamera.cpp
  VEAreaItem.cpp
  VEVisionSimulatorItem.cpp
  VisualEffectPlugin.cpp
)

set(headers
  CameraEffect.h
  ImageGenerator.h
  NoisyCamera.h
  VEAreaItem.h
  VEVisionSimulatorItem.h
  exportdecl.h
)

choreonoid_make_header_public(ImageGenerator.h)
choreonoid_make_header_public(NoisyCamera.h)

set(target CnoidVisualEffectPlugin)
make_gettext_mofiles(${target} mofiles)
choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
target_link_libraries(${target} PUBLIC CnoidBodyPlugin CnoidCFDPlugin)