#ifndef CNOID_LAYOUTSWITCHER_EXPORTDECL_H_INCLUDED
# define CNOID_LAYOUTSWITCHER_EXPORTDECL_H_INCLUDED

# if defined _WIN32 || defined __CYGWIN__
#  define CNOID_LAYOUTSWITCHER_DLLIMPORT __declspec(dllimport)
#  define CNOID_LAYOUTSWITCHER_DLLEXPORT __declspec(dllexport)
#  define CNOID_LAYOUTSWITCHER_DLLLOCAL
# else
#  if __GNUC__ >= 4
#   define CNOID_LAYOUTSWITCHER_DLLIMPORT __attribute__ ((visibility("default")))
#   define CNOID_LAYOUTSWITCHER_DLLEXPORT __attribute__ ((visibility("default")))
#   define CNOID_LAYOUTSWITCHER_DLLLOCAL  __attribute__ ((visibility("hidden")))
#  else
#   define CNOID_LAYOUTSWITCHER_DLLIMPORT
#   define CNOID_LAYOUTSWITCHER_DLLEXPORT
#   define CNOID_LAYOUTSWITCHER_DLLLOCAL
#  endif
# endif

# ifdef CNOID_LAYOUTSWITCHER_STATIC
#  define CNOID_LAYOUTSWITCHER_DLLAPI
#  define CNOID_LAYOUTSWITCHER_LOCAL
# else
#  ifdef CnoidLayoutSwitcherPlugin_EXPORTS
#   define CNOID_LAYOUTSWITCHER_DLLAPI CNOID_LAYOUTSWITCHER_DLLEXPORT
#  else
#   define CNOID_LAYOUTSWITCHER_DLLAPI CNOID_LAYOUTSWITCHER_DLLIMPORT
#  endif
#  define CNOID_LAYOUTSWITCHER_LOCAL CNOID_LAYOUTSWITCHER_DLLLOCAL
# endif

#endif

#ifdef CNOID_EXPORT
# undef CNOID_EXPORT
#endif
#define CNOID_EXPORT CNOID_LAYOUTSWITCHER_DLLAPI