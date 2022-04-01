#ifndef CNOID_OMPL_PLUGIN_EXPORTDECL_H
# define CNOID_OMPL_PLUGIN_EXPORTDECL_H
# if defined _WIN32 || defined __CYGWIN__
#  define CNOID_OMPL_PLUGIN_DLLIMPORT __declspec(dllimport)
#  define CNOID_OMPL_PLUGIN_DLLEXPORT __declspec(dllexport)
#  define CNOID_OMPL_PLUGIN_DLLLOCAL
# else
#  if __GNUC__ >= 4
#   define CNOID_OMPL_PLUGIN_DLLIMPORT __attribute__ ((visibility("default")))
#   define CNOID_OMPL_PLUGIN_DLLEXPORT __attribute__ ((visibility("default")))
#   define CNOID_OMPL_PLUGIN_DLLLOCAL  __attribute__ ((visibility("hidden")))
#  else
#   define CNOID_OMPL_PLUGIN_DLLIMPORT
#   define CNOID_OMPL_PLUGIN_DLLEXPORT
#   define CNOID_OMPL_PLUGIN_DLLLOCAL
#  endif
# endif

# ifdef CNOID_OMPL_PLUGIN_STATIC
#  define CNOID_OMPL_PLUGIN_DLLAPI
#  define CNOID_OMPL_PLUGIN_LOCAL
# else
#  ifdef CnoidOMPLPlugin_EXPORTS
#   define CNOID_OMPL_PLUGIN_DLLAPI CNOID_OMPL_PLUGIN_DLLEXPORT
#  else
#   define CNOID_OMPL_PLUGIN_DLLAPI CNOID_OMPL_PLUGIN_DLLIMPORT
#  endif
#  define CNOID_OMPL_PLUGIN_LOCAL CNOID_OMPL_PLUGIN_DLLLOCAL
# endif

#endif

#ifdef CNOID_EXPORT
# undef CNOID_EXPORT
#endif
#define CNOID_EXPORT CNOID_OMPL_PLUGIN_DLLAPI
