#ifndef CNOID_PHITSPLUGIN_EXPORTDECL_H_INCLUDED
# define CNOID_PHITSPLUGIN_EXPORTDECL_H_INCLUDED

# if defined _WIN32 || defined __CYGWIN__
#  define CNOID_PHITSPLUGIN_DLLIMPORT __declspec(dllimport)
#  define CNOID_PHITSPLUGIN_DLLEXPORT __declspec(dllexport)
#  define CNOID_PHITSPLUGIN_DLLLOCAL
# else
#  if __GNUC__ >= 4
#   define CNOID_PHITSPLUGIN_DLLIMPORT __attribute__ ((visibility("default")))
#   define CNOID_PHITSPLUGIN_DLLEXPORT __attribute__ ((visibility("default")))
#   define CNOID_PHITSPLUGIN_DLLLOCAL  __attribute__ ((visibility("hidden")))
#  else
#   define CNOID_PHITSPLUGIN_DLLIMPORT
#   define CNOID_PHITSPLUGIN_DLLEXPORT
#   define CNOID_PHITSPLUGIN_DLLLOCAL
#  endif
# endif

# ifdef CNOID_PHITSPLUGIN_STATIC
#  define CNOID_PHITSPLUGIN_DLLAPI
#  define CNOID_PHITSPLUGIN_LOCAL
# else
#  ifdef CnoidPHITSPlugin_EXPORTS
#   define CNOID_PHITSPLUGIN_DLLAPI CNOID_PHITSPLUGIN_DLLEXPORT
#  else
#   define CNOID_PHITSPLUGIN_DLLAPI CNOID_PHITSPLUGIN_DLLIMPORT
#  endif
#  define CNOID_PHITSPLUGIN_LOCAL CNOID_PHITSPLUGIN_DLLLOCAL
# endif

#endif

#ifdef CNOID_EXPORT
# undef CNOID_EXPORT
#endif
#define CNOID_EXPORT CNOID_PHITSPLUGIN_DLLAPI
