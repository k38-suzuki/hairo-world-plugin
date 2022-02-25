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
  model
  project
  DESTINATION ${CNOID_SHARE_SUBDIR}/HAIROWorld)

set(make_symlink false)
if(NOT CNOID_SOURCE_SHARE_DIR STREQUAL CNOID_BINARY_SHARE_DIR)
  if(UNIX)
    set(make_symlink true)
  endif()
  configure_file(default/materials_agxcrawler.yaml ${CNOID_BINARY_SHARE_DIR}/default/materials_agxcrawler.yaml COPYONLY)
endif()

install(FILES default/materials_agxcrawler.yaml DESTINATION ${CNOID_SHARE_SUBDIR}/default)

add_subdirectory(controller)
