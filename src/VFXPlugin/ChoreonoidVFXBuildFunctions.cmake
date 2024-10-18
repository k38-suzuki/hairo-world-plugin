add_library(Choreonoid::CnoidVFXPlugin SHARED IMPORTED GLOBAL)
if(WIN32)
  set_target_properties(Choreonoid::CnoidVFXPlugin PROPERTIES
    IMPORTED_LOCATION ${CHOREONOID_PLUGIN_DIR}/CnoidVFXPlugin.dll
    IMPORTED_IMPLIB ${CHOREONOID_LIB_DIR}/CnoidVFXPlugin.lib
    IMPORTED_LOCATION_DEBUG ${CHOREONOID_PLUGIN_DIR}/CnoidVFXPlugind.dll
    IMPORTED_IMPLIB_DEBUG ${CHOREONOID_LIB_DIR}/CnoidVFXPlugind.lib
    IMPORTED_CONFIGURATIONS "RELEASE;DEBUG")
else()
  set_target_properties(Choreonoid::CnoidVFXPlugin PROPERTIES
    IMPORTED_LOCATION ${CHOREONOID_PLUGIN_DIR}/libCnoidVFXPlugin.so)
endif()
target_link_libraries(Choreonoid::CnoidVFXPlugin INTERFACE Choreonoid::CnoidBodyPlugin)
set(CHOREONOID_VFX_PLUGIN_LIBRARIES Choreonoid::CnoidVFXPlugin)
