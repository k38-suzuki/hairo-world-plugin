set(sources
  BodyCreatorDialog.cpp
  FormatConverter.cpp
  ColorButton.cpp
  CrawlerCreator.cpp
  CreatorToolBar.cpp
  GratingCreator.cpp
  BodyCreatorPlugin.cpp
  InertiaCalculator.cpp
  PipeCreator.cpp
  SlopeCreator.cpp
  StairsCreator.cpp
  TerrainCreator.cpp
  BentPipeCreator.cpp
  FileDroppableWidget.cpp
  Notepad.cpp
)

set(headers
  BodyCreator.h
  BodyCreatorDialog.h
  ColorButton.h
  CreatorToolBar.h
  FileDroppableWidget.h
  Notepad.h
  WidgetInfo.h
  exportdecl.h
)

choreonoid_make_header_public(FileDroppableWidget.h)

set(target CnoidBodyCreatorPlugin)
choreonoid_make_gettext_mo_files(${target} mofiles)
choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
target_link_libraries(${target} PUBLIC CnoidBodyPlugin CnoidBookmarkPlugin)