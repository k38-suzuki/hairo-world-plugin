
file(GLOB subdirs "*")

foreach(subdir ${subdirs})
  if(EXISTS ${subdir}/CMakeLists.txt)
    add_subdirectory(${subdir})
  endif()
endforeach()