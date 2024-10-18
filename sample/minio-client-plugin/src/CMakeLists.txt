if(NOT ENABLE_GUI)
  return()
endif()

option(BUILD_MINIO_CLIENT_PLUGIN "Building MinIOClientPlugin" OFF)
if(NOT BUILD_MINIO_CLIENT_PLUGIN)
  return()
endif()

set(sources
  MinIOClient.cpp
  MinIOClientPlugin.cpp
  ObjectBrowser.cpp
)

set(headers
  MinIOClient.h
  ObjectBrowser.h
  exportdecl.h
)

choreonoid_make_header_public(MinIOClient.h)
choreonoid_make_header_public(ObjectBrowser.h)

set(target CnoidMinIOClientPlugin)
choreonoid_make_gettext_mo_files(${target} mofiles)
choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
target_link_libraries(${target} PUBLIC CnoidBodyPlugin)

include(ChoreonoidMinIOBuildFunctions.cmake)
if(CHOREONOID_INSTALL_SDK)
  install(FILES ChoreonoidMinIOBuildFunctions.cmake DESTINATION ${CHOREONOID_CMAKE_CONFIG_SUBDIR}/ext)
endif()