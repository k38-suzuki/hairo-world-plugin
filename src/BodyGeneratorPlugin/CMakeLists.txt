
set(sources
    CrawlerGenerator.cpp
    FileFormWidget.cpp
    GratingGenerator.cpp
    BodyGeneratorPlugin.cpp
    PipeGenerator.cpp
    SlopeGenerator.cpp
    TerrainGenerator.cpp
    )

set(headers
    CrawlerGenerator.h
    FileFormWidget.h
    GratingGenerator.h
    PipeGenerator.h
    SlopeGenerator.h
    TerrainGenerator.h
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
  cmake_minimum_required(VERSION 3.1.0)
  project(BodyGeneratorPlugin)
  find_package(Choreonoid REQUIRED)
  add_definitions(${CHOREONOID_COMPILE_DEFINITIONS})
  include_directories(${CHOREONOID_INCLUDE_DIRS})
  link_directories(${CHOREONOID_LIBRARY_DIRS})
  set(target CnoidBodyGeneratorPlugin)
  make_gettext_mofiles(${target} mofiles)
  add_library(${target} SHARED  ${sources} ${mofiles} HEADERS ${headers})
  target_link_libraries(${target} ${CHOREONOID_BODY_PLUGIN_LIBRARIES})
  install(TARGETS ${target} LIBRARY DESTINATION ${CHOREONOID_PLUGIN_DIR})
endif()
