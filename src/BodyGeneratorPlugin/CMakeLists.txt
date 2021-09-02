
set(sources
    BodyGenerator.cpp
    CellManager.cpp
    CrawlerBuilderDialog.cpp
    FileFormWidget.cpp
    GratingBuilderDialog.cpp
    BodyGeneratorPlugin.cpp
    PipeBuilderDialog.cpp
    SlopeBuilderDialog.cpp
    TerrainBuilderDialog.cpp
    )

set(headers
    BodyGenerator.h
    CellManager.h
    CrawlerBuilderDialog.h
    FileFormWidget.h
    GratingBuilderDialog.h
    PipeBuilderDialog.h
    SlopeBuilderDialog.h
    TerrainBuilderDialog.h
    gettext.h
    )

if(CMAKE_PROJECT_NAME STREQUAL "Choreonoid")
  # Build inside the Choreonoid project
  set(target CnoidBodyGeneratorPlugin)
  make_gettext_mofiles(${target} mofiles)
  choreonoid_add_plugin(${target} ${sources} ${mofiles} ${headers})
  target_link_libraries(${target} CnoidBodyPlugin)
else()
  # Stand alone build
  cmake_minimum_required(VERSION 3.1.0)
  project(BodyGeneratorPlugin)
  find_package(Choreonoid REQUIRED)
  add_definitions(${CHOREONOID_COMPILE_DEFINITIONS})
  include_directories(${CHOREONOID_INCLUDE_DIRS})
  link_directories(${CHOREONOID_LIBRARY_DIRS})
  set(target CnoidBodyGeneratorPlugin)
  make_gettext_mofiles(${target} mofiles)
  add_library(${target} SHARED  ${sources} ${mofiles} ${headers})
  target_link_libraries(${target} ${CHOREONOID_BODY_PLUGIN_LIBRARIES})
  install(TARGETS ${target} LIBRARY DESTINATION ${CHOREONOID_PLUGIN_DIR})
endif()
