if(NOT ENABLE_GUI)
  return()
endif()

#option(BUILD_SIMPLE_CONTROLLER_SAMPLES "Building SimpleController samples" ON)
if(NOT BUILD_SIMPLE_CONTROLLER_SAMPLES)
  return()
endif()

set(project_files "")

# Gen2
choreonoid_add_simple_controller(Gen2JoystickController Gen2JoystickController.cpp)
list(APPEND project_files Gen2.cnoid)

# Husky
choreonoid_add_simple_controller(HuskyJoystickController HuskyJoystickController.cpp)
list(APPEND project_files Husky.cnoid Husky-UR5-2F85.cnoid)

# Jackal
choreonoid_add_simple_controller(JackalJoystickController JackalJoystickController.cpp)
list(APPEND project_files Jackal.cnoid)

# Universal Robots
choreonoid_add_simple_controller(URJoystickController URJoystickController.cpp)
list(APPEND project_files UR3-2F85.cnoid UR5-2F85.cnoid UR10-2F85.cnoid)

# Gen3lite
choreonoid_add_simple_controller(Gen3liteJoystickController Gen3liteJoystickController.cpp)
list(APPEND project_files Gen3lite.cnoid)

# Gen3
choreonoid_add_simple_controller(Gen3JoystickController Gen3JoystickController.cpp)
list(APPEND project_files Gen3.cnoid)

choreonoid_add_simple_controller(JointMapController JointMapController.cpp)

install_project_files(${project_files})
