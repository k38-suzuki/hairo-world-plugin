if(NOT ENABLE_GUI)
  return()
endif()

option(BUILD_MARKER_DETECTION_SAMPLE "Building a marker detection sample" OFF)
if(NOT BUILD_MARKER_DETECTION_SAMPLE)
    return()
endif()

set(sources
  ScopeDevice.cpp
  MarkerDetectorItem.cpp
  MarkerDetectPlugin.cpp
)

set(headers
  ScopeDevice.h
  MarkerDetectorItem.h
  exportdecl.h
)

choreonoid_make_header_public(ScopeDevice.h)

set(target CnoidMarkerDetectPlugin)
choreonoid_make_gettext_mo_files(${target} mofiles)
choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
target_link_libraries(${target} PUBLIC CnoidBodyPlugin CnoidMotionCapturePlugin)