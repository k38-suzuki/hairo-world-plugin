option(BUILD_SPOT_SAMPLE "Building Spot sample" ON)
if(NOT BUILD_SPOT_SAMPLE)
  return()
endif()

if(UNIX)
  add_custom_target(
    symlink_to_Spot ALL
    COMMAND ${CMAKE_COMMAND} -E create_symlink
    ${CMAKE_CURRENT_SOURCE_DIR} ${CHOREONOID_BINARY_SHARE_DIR}/Spot)
endif()

install(DIRECTORY model DESTINATION ${CHOREONOID_SHARE_SUBDIR}/Spot)

add_subdirectory(sample)