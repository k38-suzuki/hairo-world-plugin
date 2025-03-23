set(default_yaml_files elements.yaml filters.yaml nuclides.yaml shields.yaml)
foreach(file ${default_yaml_files})
  install(FILES ${file} DESTINATION ${CHOREONOID_SHARE_SUBDIR}/default)
endforeach()

if(make_symlink)
#  file(MAKE_DIRECTORY ${CNOID_BINARY_SHARE_DIR}/default)
  foreach(file ${default_yaml_files})
    add_custom_target(
      symlink_to_default_${file} ALL
      COMMAND ${CMAKE_COMMAND} -E create_symlink
      ${CNOID_SOURCE_SHARE_DIR}/default/${file} ${CNOID_BINARY_SHARE_DIR}/default/${file})
  endforeach()
endif()