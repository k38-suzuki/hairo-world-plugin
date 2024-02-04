#ifndef CNOID_OMPLPLUGIN_EXPORTDECL_H
# define CNOID_OMPLPLUGIN_EXPORTDECL_H
# if defined _WIN32 || defined __CYGWIN__
#  define CNOID_OMPLPLUGIN_DLLIMPORT __declspec(dllimport)
#  define CNOID_OMPLPLUGIN_DLLEXPORT __declspec(dllexport)
#  define CNOID_OMPLPLUGIN_DLLLOCAL
# else
#  if __GNUC__ >= 4
#   define CNOID_OMPLPLUGIN_DLLIMPORT __attribute__ ((visibility("default")))
#   define CNOID_OMPLPLUGIN_DLLEXPORT __attribute__ ((visibility("default")))
#   define CNOID_OMPLPLUGIN_DLLLOCAL  __attribute__ ((visibility("hidden")))
#  else
#   define CNOID_OMPLPLUGIN_DLLIMPORT
#   define CNOID_OMPLPLUGIN_DLLEXPORT
#   define CNOID_OMPLPLUGIN_DLLLOCAL
#  endif
# endif

# ifdef CNOID_OMPLPLUGIN_STATIC
#  define CNOID_OMPLPLUGIN_DLLAPI
#  define CNOID_OMPLPLUGIN_LOCAL
# else
#  ifdef CnoidOMPLPlugin_EXPORTS
#   define CNOID_OMPLPLUGIN_DLLAPI CNOID_OMPLPLUGIN_DLLEXPORT
#  else
#   define CNOID_OMPLPLUGIN_DLLAPI CNOID_OMPLPLUGIN_DLLIMPORT
#  endif
#  define CNOID_OMPLPLUGIN_LOCAL CNOID_OMPLPLUGIN_DLLLOCAL
# endif

#endif

#ifdef CNOID_EXPORT
# undef CNOID_EXPORT
#endif
#define CNOID_EXPORT CNOID_OMPLPLUGIN_DLLAPI