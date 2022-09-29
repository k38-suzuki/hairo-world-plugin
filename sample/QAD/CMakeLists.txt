if(NOT BUILD_PHITS_PLUGIN)
  return()
endif()

if(NOT ENABLE_GUI)
  return()
endif()

if(NOT ENABLE_QAD)
  return()
endif()

if(UNIX)
  add_custom_target(
    symlink_to_QAD ALL
    COMMAND ${CMAKE_COMMAND} -E create_symlink
    ${CMAKE_CURRENT_SOURCE_DIR} ${CNOID_BINARY_SHARE_DIR}/QAD)
endif()

#install(DIRECTORY
#  project
#  model
#  DESTINATION ${CNOID_SHARE_SUBDIR}/QAD)
