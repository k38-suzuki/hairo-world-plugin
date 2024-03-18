choreonoid_add_simple_controller(FireEventController FireEventController.cpp)
target_link_libraries(FireEventController CnoidBodyPlugin CnoidSceneEffectsPlugin)

choreonoid_add_simple_controller(FountainEventController FountainEventController.cpp)
target_link_libraries(FountainEventController CnoidBodyPlugin CnoidSceneEffectsPlugin)

install(FILES fire.cnoid fountain.cnoid
        DESTINATION ${CNOID_SHARE_SUBDIR}/project)
