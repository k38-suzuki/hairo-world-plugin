if(NOT BUILD_PHITS_PLUGIN)
  return()
endif()

add_cnoid_simple_controller(DoseMeterController DoseMeterController.cpp)
target_link_libraries(DoseMeterController CnoidPHITSPlugin)

install(FILES Tank-phits.cnoid Tank-phits2.cnoid Tank-phits3.cnoid
    DESTINATION ${CNOID_SHARE_SUBDIR}/project)