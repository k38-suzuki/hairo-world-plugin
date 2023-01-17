
set(sources
    CrawlerGeneratorDialog.cpp
    FileFormWidget.cpp
    GratingGeneratorDialog.cpp
    BodyGeneratorPlugin.cpp
    PipeGeneratorDialog.cpp
    SlopeGeneratorDialog.cpp
    TerrainGeneratorDialog.cpp
    )

set(headers
    CrawlerGeneratorDialog.h
    FileFormWidget.h
    GratingGeneratorDialog.h
    PipeGeneratorDialog.h
    SlopeGeneratorDialog.h
    TerrainGeneratorDialog.h
    gettext.h
    )

if(CMAKE_PROJECT_NAME STREQUAL "Choreonoid")
  # Build inside the Choreonoid project
  set(target CnoidBodyGeneratorPlugin)
  make_gettext_mofiles(${target} mofiles)
  choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
  target_link_libraries(${target} CnoidBodyPlugin)
else()
  # Stand alone build
  set(target CnoidBodyGeneratorPlugin)
  make_gettext_mofiles(${target} mofiles)
  add_library(${target} SHARED  ${sources} ${mofiles} ${headers})
  target_link_libraries(${target} ${CHOREONOID_BODY_PLUGIN_LIBRARIES})
  install(TARGETS ${target} LIBRARY DESTINATION ${CHOREONOID_PLUGIN_DIR})
endif()