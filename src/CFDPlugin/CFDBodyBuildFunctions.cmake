function(choreonoid_add_drone_controller target)

  choreonoid_add_simple_controller(${target} ${ARGN})
  target_link_libraries(${target} CnoidCFDPlugin)

endfunction()
