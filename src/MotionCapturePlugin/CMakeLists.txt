
set(sources
    MotionCaptureItem.cpp
    MotionCapturePlugin.cpp
    PassiveMarker.cpp
   )

set(headers
    exportdecl.h
    gettext.h
    MotionCaptureItem.h
    PassiveMarker.h
    )

choreonoid_make_header_public(PassiveMarker.h)

if(CMAKE_PROJECT_NAME STREQUAL "Choreonoid")
  # Build inside the Choreonoid project
  set(target CnoidMotionCapturePlugin)
  make_gettext_mofiles(${target} mofiles)
  choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
  target_link_libraries(${target} CnoidBodyPlugin)
  apply_common_setting_for_plugin(${target})
else()
  # Stand alone build
  set(target CnoidMotionCapturePlugin)
  make_gettext_mofiles(${target} mofiles)
  add_library(${target} SHARED ${sources} ${mofiles} ${headers})
  target_link_libraries(${target} ${CHOREONOID_BODY_PLUGIN_LIBRARIES})
  install(TARGETS ${target} LIBRARY DESTINATION ${CHOREONOID_PLUGIN_DIR})
endif()