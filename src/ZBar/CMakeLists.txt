set(sources
  QRDecoder.cpp
)

set(headers
  QRDecoder.h
  exportdecl.h
)

choreonoid_make_header_public(QRDecoder.h)
  
set(target CnoidZBar)
make_gettext_mofiles(${target} mofiles)
choreonoid_add_library(${target} SHARED ${sources} ${mofiles} HEADERS ${headers})
target_link_libraries(${target} PUBLIC CnoidBodyPlugin)