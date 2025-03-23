choreonoid_add_simple_controller(UGVJoystickController UGVJoystickController.cpp)

install(FILES Jackal.cnoid Husky.cnoid Husky-UR5-2F85.cnoid
    DESTINATION ${CHOREONOID_SHARE_SUBDIR}/project)