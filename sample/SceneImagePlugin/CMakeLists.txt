
option(BUILD_SCENE_IMAGE_PLUGIN "Building SceneImagePlugin" OFF)
if(NOT BUILD_SCENE_IMAGE_PLUGIN)
  return()
endif()

set(sources
    SceneImagePlugin.cpp
    SceneImageView.cpp
    SceneImageWidget.cpp
    )

set(headers
    SceneImageView.h
    SceneImageWidget.h
    )

if(CMAKE_PROJECT_NAME STREQUAL "Choreonoid")
  # Build inside the Choreonoid project
  set(target CnoidSceneImagePlugin)
  make_gettext_mofiles(${target} mofiles)
  choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
  target_link_libraries(${target} CnoidBodyPlugin)
else()
  # Stand alone build
  cmake_minimum_required(VERSION 3.1.0)
  project(SceneImagePlugin)
  find_package(Choreonoid REQUIRED)
  add_definitions(${CHOREONOID_COMPILE_DEFINITIONS})
  include_directories(${CHOREONOID_INCLUDE_DIRS})
  link_directories(${CHOREONOID_LIBRARY_DIRS})
  set(target CnoidSceneImagePlugin)
  make_gettext_mofiles(${target} mofiles)
  add_library(${target} SHARED  ${sources} ${mofiles} HEADERS ${headers})
  target_link_libraries(${target} ${CHOREONOID_BODY_PLUGIN_LIBRARIES})
  install(TARGETS ${target} LIBRARY DESTINATION ${CHOREONOID_PLUGIN_DIR})
endif()
