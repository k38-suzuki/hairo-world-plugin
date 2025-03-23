set(sources
  NoisyCamera.cpp
  VisualFilter.cpp
  VFXPlugin.cpp
  VFXEventReader.cpp
  VFXVisionSimulatorItem.cpp
)

set(headers
  NoisyCamera.h
  VisualFilter.h
  VFXEventReader.h
  VFXVisionSimulatorItem.h
  exportdecl.h
)

choreonoid_make_header_public(ImageGenerator.h)
choreonoid_make_header_public(NoisyCamera.h)
choreonoid_make_header_public(VisualFilter.h)
choreonoid_make_header_public(VFXVisionSimulatorItem.h)

set(target CnoidVFXPlugin)
choreonoid_make_gettext_mo_files(${target} mofiles)
choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
target_link_libraries(${target} PUBLIC CnoidBodyPlugin CnoidGLVisionSimulatorPlugin CnoidSimpleColliderPlugin)

include(ChoreonoidVFXBuildFunctions.cmake)
if(CHOREONOID_INSTALL_SDK)
  install(FILES ChoreonoidVFXBuildFunctions.cmake DESTINATION ${CHOREONOID_CMAKE_CONFIG_SUBDIR}/ext)
endif()