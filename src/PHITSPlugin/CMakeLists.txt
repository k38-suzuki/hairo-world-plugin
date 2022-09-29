
option(BUILD_PHITS_PLUGIN "Building a phits plugin" OFF)
if(NOT BUILD_PHITS_PLUGIN)
  return()
endif()

set(headers
    Array3D.h
    Box.h
    ColorScale.h
    ComptonCone.h
    ComptonCamera.h
    ComptonConesReconstruct.h
    ConfigTable.h
    CrossSectionItem.h
    DoseMeter.h
    DoseSimulatorItem.h
    EnergyFilter.h
    exportdecl.h
    GammaCamera.h
    GammaData.h
    GammaImageGenerator.h
    GammaImagerItem.h
    gettext.h
    OrthoNodeData.h
    PHITSRunner.h
    PHITSWriter.h
    PinholeCamera.h
    QADWriter.h
    stb_image_write.h
    )

choreonoid_make_header_public(ComptonCamera.h)
choreonoid_make_header_public(DoseMeter.h)
choreonoid_make_header_public(PinholeCamera.h)

set(sources
    ColorScale.cpp
    ComptonCone.cpp
    ComptonCamera.cpp
    ComptonConesReconstruct.cpp
    ConfigTable.cpp
    CrossSectionItem.cpp
    DoseMeter.cpp
    DoseSimulatorItem.cpp
    EnergyFilter.cpp
    GammaCamera.cpp
    GammaData.cpp
    GammaImageGenerator.cpp
    GammaImagerItem.cpp
    OrthoNodeData.cpp
    PHITSPlugin.cpp
    PHITSRunner.cpp
    PHITSWriter.cpp
    PinholeCamera.cpp
    QADWriter.cpp
    )

if(CMAKE_PROJECT_NAME STREQUAL "Choreonoid")
  # Build inside the Choreonoid project
  set(target CnoidPHITSPlugin)
  make_gettext_mofiles(${target} mofiles)
  add_cnoid_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
  target_link_libraries(${target} CnoidBodyPlugin)
  apply_common_setting_for_plugin(${target})
else()
  # Stand alone build
  cmake_minimum_required(VERSION 3.1.0)
  project(PHITSPlugin)
  find_package(Choreonoid REQUIRED)
  add_definitions(${CHOREONOID_DEFINITIONS})
  include_directories(${CHOREONOID_INCLUDE_DIRS})
  link_directories(${CHOREONOID_LIBRARY_DIRS})
  set(target CnoidPHITSPlugin)
  make_gettext_mofiles(${target} mofiles)
  add_library(${target} SHARED ${sources} ${mofiles} HEADERS ${headers})
  target_link_libraries(${target} ${CHOREONOID_BODY_PLUGIN_LIBRARIES})
  install(TARGETS ${target} LIBRARY DESTINATION ${CHOREONOID_PLUGIN_DIR})
endif()
