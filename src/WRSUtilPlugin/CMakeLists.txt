set(sources
  WRSUtilBar.cpp
  WRSUtilPlugin.cpp
)

set(headers
  WRSUtilBar.h
  exportdecl.h
)

choreonoid_make_header_public(WRSUtilBar.h)

set(target CnoidWRSUtilPlugin)
choreonoid_make_gettext_mo_files(${target} mofiles)
choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
target_link_libraries(${target} PUBLIC CnoidBodyPlugin)