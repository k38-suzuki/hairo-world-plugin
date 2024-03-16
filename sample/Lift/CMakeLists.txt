if(NOT ENABLE_GUI)
  return()
endif()

option(BUILD_LIFT_SAMPLE "Building a lift sample" OFF)
if(NOT BUILD_LIFT_SAMPLE)
  return()
endif()

set(sources
  LiftSamplePlugin.cpp
  LiftSimulatorItem.cpp
)

set(target CnoidLiftSamplePlugin)
choreonoid_add_plugin(${target} ${sources})
target_link_libraries(${target} PUBLIC CnoidBodyPlugin CnoidCFDPlugin)
install_project_files(LiftSample.cnoid)

choreonoid_add_simple_controller(WingController WingController.cpp)