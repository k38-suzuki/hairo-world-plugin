set(sources
  ImageGenerator.cpp
  NoisyCamera.cpp
  VFXConverter.cpp
  VFXPlugin.cpp
  VFXVisionSimulatorItem.cpp
)

set(headers
  ImageGenerator.h
  NoisyCamera.h
  VFXConverter.h
  VFXVisionSimulatorItem.h
  exportdecl.h
)

choreonoid_make_header_public(ImageGenerator.h)
choreonoid_make_header_public(NoisyCamera.h)
choreonoid_make_header_public(VFXConverter.h)

set(target CnoidVFXPlugin)
make_gettext_mofiles(${target} mofiles)

if(CMAKE_PROJECT_NAME STREQUAL "Choreonoid")
  # Build inside the Choreonoid project
  choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
  target_link_libraries(${target} PUBLIC CnoidBodyPlugin CnoidSimpleColliderPlugin)
else()
  # Stand alone build
  add_library(${target} SHARED ${sources} ${mofiles} ${headers})
  target_link_libraries(${target} ${CHOREONOID_BODY_PLUGIN_LIBRARIES} CnoidSimpleColliderPlugin)
  install(TARGETS ${target} LIBRARY DESTINATION ${CHOREONOID_PLUGIN_DIR})
endif()