if(NOT BUILD_PHITS_PLUGIN)
  return()
endif()

add_cnoid_simple_controller(DoseMeterController DoseMeterController.cpp)
target_link_libraries(DoseMeterController CnoidPHITSPlugin)
