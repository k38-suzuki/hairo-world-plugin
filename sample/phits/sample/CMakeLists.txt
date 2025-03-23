choreonoid_add_simple_controller(DoseMeterController DoseMeterController.cpp)
target_link_libraries(DoseMeterController CnoidPHITSPlugin)

install(FILES Tank-phits.cnoid Tank-phits2.cnoid Tank-phits3.cnoid
    DESTINATION ${CHOREONOID_SHARE_SUBDIR}/project)