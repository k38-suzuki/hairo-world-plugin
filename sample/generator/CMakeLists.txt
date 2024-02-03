option(BUILD_GENERATOR_SAMPLES "Building Generator sampels" ON)
if(NOT BUILD_GENERATOR_SAMPLES)
  return()
endif()

if(UNIX)
  add_custom_target(
      symlink_to_Generator ALL
      COMMAND ${CMAKE_COMMAND} -E create_symlink
      ${CMAKE_CURRENT_SOURCE_DIR} ${CNOID_BINARY_SHARE_DIR}/Generator)
endif()

install(DIRECTORY model DESTINATION ${CHOREONOID_SHARE_SUBDIR}/Generator)

add_subdirectory(sample)