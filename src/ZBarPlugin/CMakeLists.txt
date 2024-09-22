set(sources
  QRDecoder.cpp
  QRReader.cpp
  ZBarPlugin.cpp
)

set(headers
  QRDecoder.h
  QRReader.h
  exportdecl.h
)

choreonoid_make_header_public(QRDecoder.h)
  
set(target CnoidZBarPlugin)
make_gettext_mofiles(${target} mofiles)
choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
target_link_libraries(${target} PUBLIC CnoidBodyPlugin CnoidGooglePlugin)