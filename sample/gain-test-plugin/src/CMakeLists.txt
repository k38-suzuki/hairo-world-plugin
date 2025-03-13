if(NOT ENABLE_GUI)
  return()
endif()

option(BUILD_GAIN_TEST_PLUGIN "Building GainTestPlugin" OFF)
if(NOT BUILD_GAIN_TEST_PLUGIN)
  return()
endif()

set(sources
  GainTestPlugin.cpp
  GainTestView.cpp
)

set(headers
  GainTestView.h
  exportdecl.h
)

choreonoid_make_header_public(GainTestView.h)

set(target CnoidGainTestPlugin)
choreonoid_make_gettext_mo_files(${target} mofiles)
choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
target_link_libraries(${target} PUBLIC CnoidBodyPlugin CnoidBookmarkPlugin)