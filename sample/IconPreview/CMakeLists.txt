if(NOT ENABLE_GUI)
  return()
endif()

option(BUILD_ICON_PREVIEW "Building an icon previewer" OFF)
if(NOT BUILD_ICON_PREVIEW)
  return()
endif()

set(sources
  IconPreviewPlugin.cpp
)

set(headers
  IconTheme.h
)

set(target CnoidIconPreviewPlugin)
choreonoid_add_plugin(${target} ${sources} HEADERS ${headers})
target_link_libraries(${target} PUBLIC CnoidBodyPlugin)