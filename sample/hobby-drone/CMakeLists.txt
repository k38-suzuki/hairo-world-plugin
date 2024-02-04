option(BUILD_HOBBY_DRONE_SAMPLES "Building HobbyDrone sampels" ON)
if(NOT BUILD_HOBBY_DRONE_SAMPLES)
  return()
endif()

if(UNIX)
  add_custom_target(
    symlink_to_HobbyDrone ALL
    COMMAND ${CMAKE_COMMAND} -E create_symlink
    ${CMAKE_CURRENT_SOURCE_DIR} ${CNOID_BINARY_SHARE_DIR}/HobbyDrone)
endif()

install(DIRECTORY model DESTINATION ${CHOREONOID_SHARE_SUBDIR}/HobbyDrone)

add_subdirectory(sample)