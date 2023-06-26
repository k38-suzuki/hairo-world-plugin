
set(sources
    CrawlerGenerator.cpp
    FileFormWidget.cpp
    GratingGenerator.cpp
    BodyEditPlugin.cpp
    InertiaCalculator.cpp
    PipeGenerator.cpp
    SlopeGenerator.cpp
    StairsGenerator.cpp
    TerrainGenerator.cpp
    )

set(headers
    CrawlerGenerator.h
    FileFormWidget.h
    GratingGenerator.h
    InertiaCalculator.h
    PipeGenerator.h
    SlopeGenerator.h
    StairsGenerator.h
    TerrainGenerator.h
    gettext.h
    )

if(CMAKE_PROJECT_NAME STREQUAL "Choreonoid")
  # Build inside the Choreonoid project
  set(target CnoidBodyEditPlugin)
  make_gettext_mofiles(${target} mofiles)
  choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
  target_link_libraries(${target} PUBLIC CnoidBodyPlugin)
else()
  # Stand alone build
  set(target CnoidBodyEditPlugin)
  make_gettext_mofiles(${target} mofiles)
  add_library(${target} SHARED  ${sources} ${mofiles} ${headers})
  target_link_libraries(${target} ${CHOREONOID_BODY_PLUGIN_LIBRARIES})
  install(TARGETS ${target} LIBRARY DESTINATION ${CHOREONOID_PLUGIN_DIR})
endif()
