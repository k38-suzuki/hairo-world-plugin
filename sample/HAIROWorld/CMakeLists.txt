if(NOT ENABLE_GUI)
  return()
endif()

if(UNIX)
  add_custom_target(
    symlink_to_HAIROWorld ALL
    COMMAND ${CMAKE_COMMAND} -E create_symlink
    ${CMAKE_CURRENT_SOURCE_DIR} ${CNOID_BINARY_SHARE_DIR}/HAIROWorld)
endif()

install(DIRECTORY
  default
  model
  project
  DESTINATION ${CNOID_SHARE_SUBDIR}/HAIROWorld)

add_subdirectory(controller)
