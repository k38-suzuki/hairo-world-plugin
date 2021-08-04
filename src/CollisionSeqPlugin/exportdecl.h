#ifndef COLLISION_SEQ_PLUGIN_EXPORTDECL_H_INCLUDED
# define COLLISION_SEQ_PLUGIN_EXPORTDECL_H_INCLUDED

# if defined _WIN32 || defined __CYGWIN__
#  define COLLISION_SEQ_PLUGIN_DLLIMPORT __declspec(dllimport)
#  define COLLISION_SEQ_PLUGIN_DLLEXPORT __declspec(dllexport)
#  define COLLISION_SEQ_PLUGIN_DLLLOCAL
# else
#  if __GNUC__ >= 4
#   define COLLISION_SEQ_PLUGIN_DLLIMPORT __attribute__ ((visibility("default")))
#   define COLLISION_SEQ_PLUGIN_DLLEXPORT __attribute__ ((visibility("default")))
#   define COLLISION_SEQ_PLUGIN_DLLLOCAL  __attribute__ ((visibility("hidden")))
#  else
#   define COLLISION_SEQ_PLUGIN_DLLIMPORT
#   define COLLISION_SEQ_PLUGIN_DLLEXPORT
#   define COLLISION_SEQ_PLUGIN_DLLLOCAL
#  endif
# endif

#ifdef CnoidCollisionSeqPlugin_EXPORTS
#   define COLLISION_SEQ_PLUGIN_DLLAPI COLLISION_SEQ_PLUGIN_DLLEXPORT
#else
#   define COLLISION_SEQ_PLUGIN_DLLAPI COLLISION_SEQ_PLUGIN_DLLIMPORT
#endif

#endif

#ifdef CNOID_EXPORT
# undef CNOID_EXPORT
#endif
#define CNOID_EXPORT COLLISION_SEQ_PLUGIN_DLLAPI
