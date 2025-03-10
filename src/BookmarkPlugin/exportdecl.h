#ifndef CNOID_BOOKMARKPLUGIN_EXPORTDECL_H_INCLUDED
# define CNOID_BOOKMARKPLUGIN_EXPORTDECL_H_INCLUDED
# if defined _WIN32 || defined __CYGWIN__
#  define CNOID_BOOKMARKPLUGIN_DLLIMPORT __declspec(dllimport)
#  define CNOID_BOOKMARKPLUGIN_DLLEXPORT __declspec(dllexport)
#  define CNOID_BOOKMARKPLUGIN_DLLLOCAL
# else
#  if __GNUC__ >= 4
#   define CNOID_BOOKMARKPLUGIN_DLLIMPORT __attribute__ ((visibility("default")))
#   define CNOID_BOOKMARKPLUGIN_DLLEXPORT __attribute__ ((visibility("default")))
#   define CNOID_BOOKMARKPLUGIN_DLLLOCAL  __attribute__ ((visibility("hidden")))
#  else
#   define CNOID_BOOKMARKPLUGIN_DLLIMPORT
#   define CNOID_BOOKMARKPLUGIN_DLLEXPORT
#   define CNOID_BOOKMARKPLUGIN_DLLLOCAL
#  endif
# endif

# ifdef CNOID_BOOKMARKPLUGIN_STATIC
#  define CNOID_BOOKMARKPLUGIN_DLLAPI
#  define CNOID_BOOKMARKPLUGIN_LOCAL
# else
#  ifdef CnoidBookmarkPlugin_EXPORTS
#   define CNOID_BOOKMARKPLUGIN_DLLAPI CNOID_BOOKMARKPLUGIN_DLLEXPORT
#  else
#   define CNOID_BOOKMARKPLUGIN_DLLAPI CNOID_BOOKMARKPLUGIN_DLLIMPORT
#  endif
#  define CNOID_BOOKMARKPLUGIN_LOCAL CNOID_BOOKMARKPLUGIN_DLLLOCAL
# endif

#endif

#ifdef CNOID_EXPORT
# undef CNOID_EXPORT
#endif
#define CNOID_EXPORT CNOID_BOOKMARKPLUGIN_DLLAPI