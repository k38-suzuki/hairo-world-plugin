#ifndef CNOID_ZBARPLUGIN_EXPORTDECL_H_INCLUDED
# define CNOID_ZBARPLUGIN_EXPORTDECL_H_INCLUDED

# if defined _WIN32 || defined __CYGWIN__
#  define CNOID_ZBARPLUGIN_DLLIMPORT __declspec(dllimport)
#  define CNOID_ZBARPLUGIN_DLLEXPORT __declspec(dllexport)
#  define CNOID_ZBARPLUGIN_DLLLOCAL
# else
#  if __GNUC__ >= 4
#   define CNOID_ZBARPLUGIN_DLLIMPORT __attribute__ ((visibility("default")))
#   define CNOID_ZBARPLUGIN_DLLEXPORT __attribute__ ((visibility("default")))
#   define CNOID_ZBARPLUGIN_DLLLOCAL  __attribute__ ((visibility("hidden")))
#  else
#   define CNOID_ZBARPLUGIN_DLLIMPORT
#   define CNOID_ZBARPLUGIN_DLLEXPORT
#   define CNOID_ZBARPLUGIN_DLLLOCAL
#  endif
# endif

#ifdef CnoidZBarPlugin_EXPORTS
#   define CNOID_ZBARPLUGIN_DLLAPI CNOID_ZBARPLUGIN_DLLEXPORT
#else
#   define CNOID_ZBARPLUGIN_DLLAPI CNOID_ZBARPLUGIN_DLLIMPORT
#endif

#endif

#ifdef CNOID_EXPORT
# undef CNOID_EXPORT
#endif
#define CNOID_EXPORT CNOID_ZBARPLUGIN_DLLAPI