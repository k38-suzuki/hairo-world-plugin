if(NOT ENABLE_SAMPLES)
  return()
endif()

if(UNIX)
  set(sample_model_dirs Spot)
  foreach(dir ${sample_model_dirs})
    add_custom_target(
      symlink_to_${dir} ALL
      COMMAND ${CMAKE_COMMAND} -E create_symlink
      ${CMAKE_CURRENT_SOURCE_DIR}/model/${dir} ${CHOREONOID_BINARY_SHARE_DIR}/model/${dir})
  endforeach()
endif()

install(DIRECTORY model DESTINATION ${CHOREONOID_SHARE_SUBDIR})

add_subdirectory(sample)