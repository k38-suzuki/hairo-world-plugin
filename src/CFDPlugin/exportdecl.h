#ifndef CNOID_CFDPLUGIN_EXPORTDECL_H_INCLUDED
# define CNOID_CFDPLUGIN_EXPORTDECL_H_INCLUDED
# if defined _WIN32 || defined __CYGWIN__
#  define CNOID_CFDPLUGIN_DLLIMPORT __declspec(dllimport)
#  define CNOID_CFDPLUGIN_DLLEXPORT __declspec(dllexport)
#  define CNOID_CFDPLUGIN_DLLLOCAL
# else
#  if __GNUC__ >= 4
#   define CNOID_CFDPLUGIN_DLLIMPORT __attribute__ ((visibility("default")))
#   define CNOID_CFDPLUGIN_DLLEXPORT __attribute__ ((visibility("default")))
#   define CNOID_CFDPLUGIN_DLLLOCAL  __attribute__ ((visibility("hidden")))
#  else
#   define CNOID_CFDPLUGIN_DLLIMPORT
#   define CNOID_CFDPLUGIN_DLLEXPORT
#   define CNOID_CFDPLUGIN_DLLLOCAL
#  endif
# endif

# ifdef CNOID_CFDPLUGIN_STATIC
#  define CNOID_CFDPLUGIN_DLLAPI
#  define CNOID_CFDPLUGIN_LOCAL
# else
#  ifdef CnoidCFDPlugin_EXPORTS
#   define CNOID_CFDPLUGIN_DLLAPI CNOID_CFDPLUGIN_DLLEXPORT
#  else
#   define CNOID_CFDPLUGIN_DLLAPI CNOID_CFDPLUGIN_DLLIMPORT
#  endif
#  define CNOID_CFDPLUGIN_LOCAL CNOID_CFDPLUGIN_DLLLOCAL
# endif

#endif

#ifdef CNOID_EXPORT
# undef CNOID_EXPORT
#endif
#define CNOID_EXPORT CNOID_CFDPLUGIN_DLLAPI