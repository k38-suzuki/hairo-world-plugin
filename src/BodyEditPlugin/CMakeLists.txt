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
  BentPipeGenerator.cpp
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
  BentPipeGenerator.h
  Notepad.h
  exportdecl.h
)

set(target CnoidBodyEditPlugin)
make_gettext_mofiles(${target} mofiles)
choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
target_link_libraries(${target} PUBLIC CnoidBodyPlugin)