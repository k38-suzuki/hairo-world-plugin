if(NOT ENABLE_GUI)
  return()
endif()

#option(BUILD_SIMPLE_CONTROLLER_SAMPLES "Building SimpleController samples" ON)
if(NOT BUILD_SIMPLE_CONTROLLER_SAMPLES)
  return()
endif()

set(project_files "")

# Gen2, Gen3, Gen3lite and Universal Robots
choreonoid_add_simple_controller(RAJoystickController RAJoystickController.cpp)
list(APPEND project_files
  Gen2.cnoid
  Gen3.cnoid
  Gen3lite.cnoid
  UR3-2F85.cnoid
  UR5-2F85.cnoid
  UR10-2F85.cnoid)

# Jackal and Husky
choreonoid_add_simple_controller(UGVJoystickController UGVJoystickController.cpp)
list(APPEND project_files Jackal.cnoid Husky.cnoid Husky-UR5-2F85.cnoid)

install_project_files(${project_files})