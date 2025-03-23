choreonoid_add_simple_controller(RAJoystickController RAJoystickController.cpp)

install(FILES Gen2.cnoid Gen3.cnoid Gen3lite.cnoid UR3-2F85.cnoid UR5-2F85.cnoid UR10-2F85.cnoid
    DESTINATION ${CHOREONOID_SHARE_SUBDIR}/project)