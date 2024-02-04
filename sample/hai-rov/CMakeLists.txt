option(BUILD_HAI_ROV_SAMPLES "Building HaiROV sampels" ON)
if(NOT BUILD_HAI_ROV_SAMPLES)
  return()
endif()

if(UNIX)
  add_custom_target(
    symlink_to_HaiROV ALL
    COMMAND ${CMAKE_COMMAND} -E create_symlink
    ${CMAKE_CURRENT_SOURCE_DIR} ${CNOID_BINARY_SHARE_DIR}/HaiROV)
endif()

install(DIRECTORY model DESTINATION ${CHOREONOID_SHARE_SUBDIR}/HaiROV)

add_subdirectory(sample)