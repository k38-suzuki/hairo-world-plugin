add_subdirectory(BodyGeneratorPlugin)
add_subdirectory(BookmarkPlugin)
add_subdirectory(CFDPlugin)
add_subdirectory(CollisionSeqPlugin)
add_subdirectory(InertiaCalculatorPlugin)
add_subdirectory(JoystickStatusPlugin)
add_subdirectory(MotionCapturePlugin)
add_subdirectory(PHITSPlugin)
add_subdirectory(VisualEffectPlugin)

if(CMAKE_PROJECT_NAME STREQUAL "Choreonoid")
  add_subdirectory(KIOSKPlugin)
else()

endif()

if(WIN32 OR APPLE)
  return()
endif()

add_subdirectory(BeepPlugin)
add_subdirectory(OMPLPlugin)
add_subdirectory(FileExplorerPlugin)
add_subdirectory(NetEmPlugin)