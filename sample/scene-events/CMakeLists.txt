option(BUILD_SCENE_EVENTS_SAMPLES "Building SceneEvents sampels" OFF)
if(NOT BUILD_SCENE_EVENTS_SAMPLES)
  return()
endif()

if(UNIX)
  add_custom_target(
    symlink_to_SceneEvents ALL
    COMMAND ${CMAKE_COMMAND} -E create_symlink
    ${CMAKE_CURRENT_SOURCE_DIR} ${CNOID_BINARY_SHARE_DIR}/SceneEvents)
endif()

install(DIRECTORY model DESTINATION ${CHOREONOID_SHARE_SUBDIR}/SceneEvents)

add_subdirectory(sample)