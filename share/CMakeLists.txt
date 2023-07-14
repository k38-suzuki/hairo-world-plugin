set(make_symlink false)

if(UNIX AND NOT CHOREONOID_SOURCE_SHARE_DIR STREQUAL CHOREONOID_BINARY_SHARE_DIR)
  set(make_symlink true)
endif()

if(BUILD_PHITS_PLUGIN)
  # "default" directory
  install(FILES default/elements.yaml DESTINATION ${CHOREONOID_SHARE_SUBDIR}/default)
  install(FILES default/filters.yaml DESTINATION ${CHOREONOID_SHARE_SUBDIR}/default)
  install(FILES default/nuclides.yaml DESTINATION ${CHOREONOID_SHARE_SUBDIR}/default)
  install(FILES default/shields.yaml DESTINATION ${CHOREONOID_SHARE_SUBDIR}/default)
  if(ENABLE_QAD)
    install(FILES default/LIB/ATTEN.DAT DESTINATION ${CHOREONOID_SHARE_SUBDIR}/default)
    install(FILES default/LIB/BUF.DAT DESTINATION ${CHOREONOID_SHARE_SUBDIR}/default)
    install(FILES default/LIB/DOSECON2.DAT DESTINATION ${CHOREONOID_SHARE_SUBDIR}/default)
  endif()
  if(make_symlink)
  #  file(MAKE_DIRECTORY ${CHOREONOID_BINARY_SHARE_DIR}/default)
    set(default_yaml_files
      elements filters nuclides shields)
    foreach(file ${default_yaml_files})
      add_custom_target(
        symlink_to_default_${file} ALL
        COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_CURRENT_SOURCE_DIR}/default/${file}.yaml ${CHOREONOID_BINARY_SHARE_DIR}/default/${file}.yaml)
    endforeach()
  endif()
endif()

# "model" directory
install(DIRECTORY model DESTINATION ${CHOREONOID_SHARE_SUBDIR})
if(make_symlink)
#  file(MAKE_DIRECTORY ${CHOREONOID_BINARY_SHARE_DIR}/model)
  if(ENABLE_SAMPLES)
    set(sample_model_dirs
      Gen2 Gen3 Gen3lite Husky Jackal MockupStairs RobotTestPool TestField TrainingStage)
    foreach(dir ${sample_model_dirs})
      add_custom_target(
	symlink_to_model_${dir} ALL
	COMMAND ${CMAKE_COMMAND} -E create_symlink
	${CMAKE_CURRENT_SOURCE_DIR}/model/${dir} ${CHOREONOID_BINARY_SHARE_DIR}/model/${dir})
    endforeach()
  endif()
endif()

# "project" directory
#if(make_symlink)
#  file(MAKE_DIRECTORY ${CHOREONOID_BINARY_SHARE_DIR}/project)
#endif()
