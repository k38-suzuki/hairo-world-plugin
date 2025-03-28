#ifndef CNOID_MINIOCLIENTPLUGIN_EXPORTDECL_H_INCLUDED
# define CNOID_MINIOCLIENTPLUGIN_EXPORTDECL_H_INCLUDED

# if defined _WIN32 || defined __CYGWIN__
#  define CNOID_MINIOCLIENTPLUGIN_DLLIMPORT __declspec(dllimport)
#  define CNOID_MINIOCLIENTPLUGIN_DLLEXPORT __declspec(dllexport)
#  define CNOID_MINIOCLIENTPLUGIN_DLLLOCAL
# else
#  if __GNUC__ >= 4
#   define CNOID_MINIOCLIENTPLUGIN_DLLIMPORT __attribute__ ((visibility("default")))
#   define CNOID_MINIOCLIENTPLUGIN_DLLEXPORT __attribute__ ((visibility("default")))
#   define CNOID_MINIOCLIENTPLUGIN_DLLLOCAL  __attribute__ ((visibility("hidden")))
#  else
#   define CNOID_MINIOCLIENTPLUGIN_DLLIMPORT
#   define CNOID_MINIOCLIENTPLUGIN_DLLEXPORT
#   define CNOID_MINIOCLIENTPLUGIN_DLLLOCAL
#  endif
# endif

# ifdef CNOID_MINIOCLIENTPLUGIN_STATIC
#  define CNOID_MINIOCLIENTPLUGIN_DLLAPI
#  define CNOID_MINIOCLIENTPLUGIN_LOCAL
# else
#  ifdef CnoidMinIOClientPlugin_EXPORTS
#   define CNOID_MINIOCLIENTPLUGIN_DLLAPI CNOID_MINIOCLIENTPLUGIN_DLLEXPORT
#  else
#   define CNOID_MINIOCLIENTPLUGIN_DLLAPI CNOID_MINIOCLIENTPLUGIN_DLLIMPORT
#  endif
#  define CNOID_MINIOCLIENTPLUGIN_LOCAL CNOID_MINIOCLIENTPLUGIN_DLLLOCAL
# endif

#endif

#ifdef CNOID_EXPORT
# undef CNOID_EXPORT
#endif
#define CNOID_EXPORT CNOID_MINIOCLIENTPLUGIN_DLLAPI