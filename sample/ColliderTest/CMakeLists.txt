if(NOT ENABLE_GUI)
  return()
endif()

option(BUILD_COLLIDER_TEST_PLUGIN "Building ColliderTestPlugin" OFF)
if(NOT BUILD_COLLIDER_TEST_PLUGIN)
  return()
endif()

set(sources
  ColliderTestPlugin.cpp
)
  
set(target CnoidColliderTestPlugin)
choreonoid_add_plugin(${target} ${sources})
target_link_libraries(${target} PUBLIC CnoidBodyPlugin CnoidSimpleColliderPlugin)