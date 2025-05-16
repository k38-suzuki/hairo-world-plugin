choreonoid_add_simple_controller(HaiROVJoystickController HaiROVJoystickController.cpp)
target_link_libraries(HaiROVJoystickController CnoidCFDPlugin)

install(FILES HaiROV.cnoid
    DESTINATION ${CHOREONOID_SHARE_SUBDIR}/project)