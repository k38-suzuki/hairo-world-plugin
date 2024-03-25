#ifndef CNOID_VFXPLUGIN_EXPORTDECL_H_INCLUDED
# define CNOID_VFXPLUGIN_EXPORTDECL_H_INCLUDED

# if defined _WIN32 || defined __CYGWIN__
#  define CNOID_VFXPLUGIN_DLLIMPORT __declspec(dllimport)
#  define CNOID_VFXPLUGIN_DLLEXPORT __declspec(dllexport)
#  define CNOID_VFXPLUGIN_DLLLOCAL
# else
#  if __GNUC__ >= 4
#   define CNOID_VFXPLUGIN_DLLIMPORT __attribute__ ((visibility("default")))
#   define CNOID_VFXPLUGIN_DLLEXPORT __attribute__ ((visibility("default")))
#   define CNOID_VFXPLUGIN_DLLLOCAL  __attribute__ ((visibility("hidden")))
#  else
#   define CNOID_VFXPLUGIN_DLLIMPORT
#   define CNOID_VFXPLUGIN_DLLEXPORT
#   define CNOID_VFXPLUGIN_DLLLOCAL
#  endif
# endif

#ifdef CnoidVFXPlugin_EXPORTS
#   define CNOID_VFXPLUGIN_DLLAPI CNOID_VFXPLUGIN_DLLEXPORT
#else
#   define CNOID_VFXPLUGIN_DLLAPI CNOID_VFXPLUGIN_DLLIMPORT
#endif

#endif

#ifdef CNOID_EXPORT
# undef CNOID_EXPORT
#endif
#define CNOID_EXPORT CNOID_VFXPLUGIN_DLLAPI