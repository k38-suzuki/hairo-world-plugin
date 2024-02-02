option(BUILD_CRAWLER_SAMPLES "Building Crawler sampels" ON)
if(NOT BUILD_CRAWLER_SAMPLES)
  return()
endif()

if(UNIX)
  add_custom_target(
      symlink_to_Crawler ALL
      COMMAND ${CMAKE_COMMAND} -E create_symlink
      ${CMAKE_CURRENT_SOURCE_DIR} ${CNOID_BINARY_SHARE_DIR}/Crawler)
endif()

install(DIRECTORY model DESTINATION ${CHOREONOID_SHARE_SUBDIR}/Crawler)

add_subdirectory(sample)