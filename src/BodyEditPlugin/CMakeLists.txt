set(sources
  BodyConverter.cpp
  ColorButton.cpp
  CrawlerGenerator.cpp
  FileFormWidget.cpp
  GratingGenerator.cpp
  BodyEditPlugin.cpp
  InertiaCalculator.cpp
  PipeGenerator.cpp
  SlopeGenerator.cpp
  StairsGenerator.cpp
  TerrainGenerator.cpp
  Notepad.cpp
)

set(headers
  BodyConverter.h
  ColorButton.h
  CrawlerGenerator.h
  FileFormWidget.h
  GratingGenerator.h
  InertiaCalculator.h
  PipeGenerator.h
  SlopeGenerator.h
  StairsGenerator.h
  TerrainGenerator.h
  Notepad.h
  exportdecl.h
)

set(target CnoidBodyEditPlugin)
make_gettext_mofiles(${target} mofiles)

if(CMAKE_PROJECT_NAME STREQUAL "Choreonoid")
  # Build inside the Choreonoid project
  choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
  target_link_libraries(${target} PUBLIC CnoidBodyPlugin)
else()
  # Stand alone build
  add_library(${target} SHARED ${sources} ${mofiles} ${headers})
  target_link_libraries(${target} ${CHOREONOID_BODY_PLUGIN_LIBRARIES})
  install(TARGETS ${target} LIBRARY DESTINATION ${CHOREONOID_PLUGIN_DIR})
endif()