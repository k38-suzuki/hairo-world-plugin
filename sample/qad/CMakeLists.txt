if(NOT ENABLE_QAD)
  return()
endif()

option(BUILD_QAD_SAMPLES "Building QAD samples" OFF)
if(NOT BUILD_QAD_SAMPLES)
  return()
endif()

if(UNIX)
  add_custom_target(
    symlink_to_QAD ALL
    COMMAND ${CMAKE_COMMAND} -E create_symlink
    ${CMAKE_CURRENT_SOURCE_DIR} ${CHOREONOID_BINARY_SHARE_DIR}/QAD)
endif()

install(DIRECTORY model DESTINATION ${CHOREONOID_SHARE_SUBDIR}/QAD)

add_subdirectory(sample)