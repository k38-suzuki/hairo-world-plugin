if(NOT ENABLE_GUI)
  return()
endif()

option(BUILD_OMPL_PLUGIN "Building OMPLPlugin" OFF)
if(NOT BUILD_OMPL_PLUGIN)
  return()
endif()

find_package(ompl REQUIRED)
include_directories(${OMPL_INCLUDE_DIRS})
link_directories(${OMPL_LIBRARY_DIRS})
add_definitions(${OMPL_DEFINITIONS})

set(sources
  OMPLPlugin.cpp
  SimpleSetup.cpp
  SimpleSetupItem.cpp
)

set(headers
  SimpleSetup.h
  SimpleSetupItem.h
  exportdecl.h
)

choreonoid_make_header_public(SimpleSetup.h)
choreonoid_make_header_public(SimpleSetupItem.h)

set(target CnoidOMPLPlugin)
choreonoid_make_gettext_mo_files(${target} mofiles)
choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
target_link_libraries(${target} PUBLIC CnoidBodyPlugin ${OMPL_LIBRARIES})

add_subdirectory(IKPlannerPlugin)