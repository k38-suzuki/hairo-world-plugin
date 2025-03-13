if(NOT BUILD_PHITS_PLUGIN)
  return()
endif()

option(BUILD_PHITS_SAMPLES "Building PHITS samples" OFF)
if(NOT BUILD_PHITS_SAMPLES)
  return()
endif()

if(UNIX)
  add_custom_target(
    symlink_to_PHITS ALL
    COMMAND ${CMAKE_COMMAND} -E create_symlink
    ${CMAKE_CURRENT_SOURCE_DIR} ${CHOREONOID_BINARY_SHARE_DIR}/PHITS)
endif()

install(DIRECTORY model DESTINATION ${CHOREONOID_SHARE_SUBDIR}/PHITS)

add_subdirectory(sample)