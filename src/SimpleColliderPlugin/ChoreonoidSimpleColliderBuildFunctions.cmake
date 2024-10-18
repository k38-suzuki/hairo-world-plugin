add_library(Choreonoid::CnoidSimpleColliderPlugin SHARED IMPORTED GLOBAL)
if(WIN32)
  set_target_properties(Choreonoid::CnoidSimpleColliderPlugin PROPERTIES
    IMPORTED_LOCATION ${CHOREONOID_PLUGIN_DIR}/CnoidSimpleColliderPlugin.dll
    IMPORTED_IMPLIB ${CHOREONOID_LIB_DIR}/CnoidSimpleColliderPlugin.lib
    IMPORTED_LOCATION_DEBUG ${CHOREONOID_PLUGIN_DIR}/CnoidSimpleColliderPlugind.dll
    IMPORTED_IMPLIB_DEBUG ${CHOREONOID_LIB_DIR}/CnoidSimpleColliderPlugind.lib
    IMPORTED_CONFIGURATIONS "RELEASE;DEBUG")
else()
  set_target_properties(Choreonoid::CnoidSimpleColliderPlugin PROPERTIES
    IMPORTED_LOCATION ${CHOREONOID_PLUGIN_DIR}/libCnoidSimpleColliderPlugin.so)
endif()
target_link_libraries(Choreonoid::CnoidSimpleColliderPlugin INTERFACE Choreonoid::CnoidBodyPlugin)
set(CHOREONOID_SIMPLE_COLLIDER_PLUGIN_LIBRARIES Choreonoid::CnoidSimpleColliderPlugin)
