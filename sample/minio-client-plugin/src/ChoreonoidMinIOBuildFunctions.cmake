add_library(Choreonoid::CnoidMinIOClientPlugin SHARED IMPORTED GLOBAL)
if(WIN32)
  set_target_properties(Choreonoid::CnoidMinIOClientPlugin PROPERTIES
    IMPORTED_LOCATION ${CHOREONOID_PLUGIN_DIR}/CnoidMinIOClientPlugin.dll
    IMPORTED_IMPLIB ${CHOREONOID_LIB_DIR}/CnoidMinIOClientPlugin.lib
    IMPORTED_LOCATION_DEBUG ${CHOREONOID_PLUGIN_DIR}/CnoidMinIOClientPlugind.dll
    IMPORTED_IMPLIB_DEBUG ${CHOREONOID_LIB_DIR}/CnoidMinIOClientPlugind.lib
    IMPORTED_CONFIGURATIONS "RELEASE;DEBUG")
else()
  set_target_properties(Choreonoid::CnoidMinIOClientPlugin PROPERTIES
    IMPORTED_LOCATION ${CHOREONOID_PLUGIN_DIR}/libCnoidMinIOClientPlugin.so)
endif()
target_link_libraries(Choreonoid::CnoidMinIOClientPlugin INTERFACE Choreonoid::Plugin)
set(CHOREONOID_MINIO_CLIENT_LIBRARIES Choreonoid::CnoidMinIOClientPlugin)
