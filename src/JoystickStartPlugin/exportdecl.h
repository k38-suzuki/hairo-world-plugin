#ifndef CNOID_JOYSTICKSTATUSPLUGIN_EXPORTDECL_H_INCLUDED
# define CNOID_JOYSTICKSTATUSPLUGIN_EXPORTDECL_H_INCLUDED

# if defined _WIN32 || defined __CYGWIN__
#  define CNOID_JOYSTICKSTATUSPLUGIN_DLLIMPORT __declspec(dllimport)
#  define CNOID_JOYSTICKSTATUSPLUGIN_DLLEXPORT __declspec(dllexport)
#  define CNOID_JOYSTICKSTATUSPLUGIN_DLLLOCAL
# else
#  if __GNUC__ >= 4
#   define CNOID_JOYSTICKSTATUSPLUGIN_DLLIMPORT __attribute__ ((visibility("default")))
#   define CNOID_JOYSTICKSTATUSPLUGIN_DLLEXPORT __attribute__ ((visibility("default")))
#   define CNOID_JOYSTICKSTATUSPLUGIN_DLLLOCAL  __attribute__ ((visibility("hidden")))
#  else
#   define CNOID_JOYSTICKSTATUSPLUGIN_DLLIMPORT
#   define CNOID_JOYSTICKSTATUSPLUGIN_DLLEXPORT
#   define CNOID_JOYSTICKSTATUSPLUGIN_DLLLOCAL
#  endif
# endif

# ifdef CNOID_JOYSTICKSTATUSPLUGIN_STATIC
#  define CNOID_JOYSTICKSTATUSPLUGIN_DLLAPI
#  define CNOID_JOYSTICKSTATUSPLUGIN_LOCAL
# else
#  ifdef CnoidJoystickStartPlugin_EXPORTS
#   define CNOID_JOYSTICKSTATUSPLUGIN_DLLAPI CNOID_JOYSTICKSTATUSPLUGIN_DLLEXPORT
#  else
#   define CNOID_JOYSTICKSTATUSPLUGIN_DLLAPI CNOID_JOYSTICKSTATUSPLUGIN_DLLIMPORT
#  endif
#  define CNOID_JOYSTICKSTATUSPLUGIN_LOCAL CNOID_JOYSTICKSTATUSPLUGIN_DLLLOCAL
# endif

#endif

#ifdef CNOID_EXPORT
# undef CNOID_EXPORT
#endif
#define CNOID_EXPORT CNOID_JOYSTICKSTATUSPLUGIN_DLLAPI