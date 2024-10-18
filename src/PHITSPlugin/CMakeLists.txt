if(NOT ENABLE_GUI)
  return()
endif()

option(BUILD_PHITS_PLUGIN "Building PHITSPlugin" OFF)
if(NOT BUILD_PHITS_PLUGIN)
  return()
endif()

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
  GammaEffect.cpp
  GammaImageGenerator.cpp
  GammaImagerItem.cpp
  GammaVisionSimulatorItem.cpp
  OrthoNodeData.cpp
  PHITSPlugin.cpp
  PHITSRunner.cpp
  PHITSWriter.cpp
  PinholeCamera.cpp
  QADWriter.cpp
)

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
  GammaCamera.h
  GammaData.h
  GammaEffect.h
  GammaImageGenerator.h
  GammaImagerItem.h
  GammaVisionSimulatorItem.h
  OrthoNodeData.h
  PHITSRunner.h
  PHITSWriter.h
  PinholeCamera.h
  QADWriter.h
  stb_image_write.h
  exportdecl.h
)

choreonoid_make_header_public(ComptonCamera.h)
choreonoid_make_header_public(DoseMeter.h)
choreonoid_make_header_public(PinholeCamera.h)

set(target CnoidPHITSPlugin)
choreonoid_make_gettext_mo_files(${target} mofiles)
choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
target_link_libraries(${target} PUBLIC CnoidBodyPlugin CnoidVFXPlugin)