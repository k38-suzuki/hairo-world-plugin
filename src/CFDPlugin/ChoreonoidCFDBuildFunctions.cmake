function(choreonoid_add_drone_controller target)

  choreonoid_add_simple_controller(${target} ${ARGN})
  target_link_libraries(${target} CnoidCFDPlugin)

endfunction()

add_library(Choreonoid::CnoidCFDPlugin SHARED IMPORTED GLOBAL)
if(WIN32)
  set_target_properties(Choreonoid::CnoidCFDPlugin PROPERTIES
    IMPORTED_LOCATION ${CHOREONOID_PLUGIN_DIR}/CnoidCFDPlugin.dll
    IMPORTED_IMPLIB ${CHOREONOID_LIB_DIR}/CnoidCFDPlugin.lib
    IMPORTED_LOCATION_DEBUG ${CHOREONOID_PLUGIN_DIR}/CnoidCFDPlugind.dll
    IMPORTED_IMPLIB_DEBUG ${CHOREONOID_LIB_DIR}/CnoidCFDPlugind.lib
    IMPORTED_CONFIGURATIONS "RELEASE;DEBUG")
else()
  set_target_properties(Choreonoid::CnoidCFDPlugin PROPERTIES
    IMPORTED_LOCATION ${CHOREONOID_PLUGIN_DIR}/libCnoidCFDPlugin.so)
endif()
target_link_libraries(Choreonoid::CnoidCFDPlugin INTERFACE Choreonoid::CnoidBodyPlugin)
set(CHOREONOID_CFD_PLUGIN_LIBRARIES Choreonoid::CnoidCFDPlugin)
