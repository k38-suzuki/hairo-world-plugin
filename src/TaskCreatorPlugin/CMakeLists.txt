set(sources
  TaskCreator.cpp
  TaskCreatorPlugin.cpp
)

set(headers
  TaskCreator.h
)

set(target CnoidTaskCreatorPlugin)
make_gettext_mofiles(${target} mofiles)
choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
target_link_libraries(${target} PUBLIC CnoidBodyPlugin)