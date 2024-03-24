if(NOT ENABLE_GUI)
  return()
endif()

set(sources
  FileExplorerPlugin.cpp
  Notepad.cpp
)

set(headers
  Notepad.h
  exportdecl.h
)

set(target CnoidFileExplorerPlugin)
make_gettext_mofiles(${target} mofiles)
choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
target_link_libraries(${target} PUBLIC CnoidBodyPlugin)