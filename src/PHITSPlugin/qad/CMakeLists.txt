if(NOT UNIX)
  return()
endif()

option(ENABLE_QAD "Enable QAD" OFF)
if(NOT ENABLE_QAD)
  return()
endif()

set(default_dat_files ATTEN.DAT BUF.DAT DOSECON2.DAT)
foreach(file ${default_dat_files})
  install(FILES LIB/${file} DESTINATION ${CNOID_SHARE_SUBDIR}/default)
endforeach()