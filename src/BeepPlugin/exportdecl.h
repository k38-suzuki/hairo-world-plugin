#ifndef CNOID_BEEPPLUGIN_EXPORTDECL_H_INCLUDED
# define CNOID_BEEPPLUGIN_EXPORTDECL_H_INCLUDED

# if defined _WIN32 || defined __CYGWIN__
#  define CNOID_BEEPPLUGIN_DLLIMPORT __declspec(dllimport)
#  define CNOID_BEEPPLUGIN_DLLEXPORT __declspec(dllexport)
#  define CNOID_BEEPPLUGIN_DLLLOCAL
# else
#  if __GNUC__ >= 4
#   define CNOID_BEEPPLUGIN_DLLIMPORT __attribute__ ((visibility("default")))
#   define CNOID_BEEPPLUGIN_DLLEXPORT __attribute__ ((visibility("default")))
#   define CNOID_BEEPPLUGIN_DLLLOCAL  __attribute__ ((visibility("hidden")))
#  else
#   define CNOID_BEEPPLUGIN_DLLIMPORT
#   define CNOID_BEEPPLUGIN_DLLEXPORT
#   define CNOID_BEEPPLUGIN_DLLLOCAL
#  endif
# endif

# ifdef CNOID_BEEPPLUGIN_STATIC
#  define CNOID_BEEPPLUGIN_DLLAPI
#  define CNOID_BEEPPLUGIN_LOCAL
# else
#  ifdef CnoidBeepPlugin_EXPORTS
#   define CNOID_BEEPPLUGIN_DLLAPI CNOID_BEEPPLUGIN_DLLEXPORT
#  else
#   define CNOID_BEEPPLUGIN_DLLAPI CNOID_BEEPPLUGIN_DLLIMPORT
#  endif
#  define CNOID_BEEPPLUGIN_LOCAL CNOID_BEEPPLUGIN_DLLLOCAL
# endif

#endif

#ifdef CNOID_EXPORT
# undef CNOID_EXPORT
#endif
#define CNOID_EXPORT CNOID_BEEPPLUGIN_DLLAPI
