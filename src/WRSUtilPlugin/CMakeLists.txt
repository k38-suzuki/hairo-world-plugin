set(sources
  WRSUtilBar.cpp
  WRSUtilPlugin.cpp
)

set(headers
  WRSUtilBar.h
  exportdecl.h
)

choreonoid_qt_add_resources(RC_SRCS WRSUtilPlugin.qrc)

choreonoid_make_header_public(WRSUtilBar.h)

set(target CnoidWRSUtilPlugin)
make_gettext_mofiles(${target} mofiles)
choreonoid_add_plugin(${target} ${sources} ${mofiles} ${RC_SRCS} HEADERS ${headers})
target_link_libraries(${target} PUBLIC CnoidBodyPlugin)