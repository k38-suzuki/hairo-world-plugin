option(BUILD_TUTORIAL_SAMPLES "Building Tutorial sampels" ON)
if(NOT BUILD_TUTORIAL_SAMPLES)
  return()
endif()

if(UNIX)
  add_custom_target(
      symlink_to_Tutorial ALL
      COMMAND ${CMAKE_COMMAND} -E create_symlink
      ${CMAKE_CURRENT_SOURCE_DIR} ${CNOID_BINARY_SHARE_DIR}/Tutorial)
endif()

install(DIRECTORY model DESTINATION ${CHOREONOID_SHARE_SUBDIR}/Tutorial)

add_subdirectory(sample)