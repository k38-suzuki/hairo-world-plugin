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
list(APPEND project_files Husky.cnoid)

# Jackal
choreonoid_add_simple_controller(JackalJoystickController JackalJoystickController.cpp)
list(APPEND project_files Jackal.cnoid)

# Universal Robot
choreonoid_add_simple_controller(UniversalRobotJoystickController UniversalRobotJoystickController.cpp)
list(APPEND project_files UR3.cnoid UR5.cnoid UR10.cnoid)

install_project_files(${project_files})
