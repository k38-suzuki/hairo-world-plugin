#ifndef VISUAL_EFFECT_PLUGIN_EXPORTDECL_H_INCLUDED
# define VISUAL_EFFECT_PLUGIN_EXPORTDECL_H_INCLUDED

# if defined _WIN32 || defined __CYGWIN__
#  define VISUAL_EFFECT_PLUGIN_DLLIMPORT __declspec(dllimport)
#  define VISUAL_EFFECT_PLUGIN_DLLEXPORT __declspec(dllexport)
#  define VISUAL_EFFECT_PLUGIN_DLLLOCAL
# else
#  if __GNUC__ >= 4
#   define VISUAL_EFFECT_PLUGIN_DLLIMPORT __attribute__ ((visibility("default")))
#   define VISUAL_EFFECT_PLUGIN_DLLEXPORT __attribute__ ((visibility("default")))
#   define VISUAL_EFFECT_PLUGIN_DLLLOCAL  __attribute__ ((visibility("hidden")))
#  else
#   define VISUAL_EFFECT_PLUGIN_DLLIMPORT
#   define VISUAL_EFFECT_PLUGIN_DLLEXPORT
#   define VISUAL_EFFECT_PLUGIN_DLLLOCAL
#  endif
# endif

#ifdef CnoidVisualEffectPlugin_EXPORTS
#   define VISUAL_EFFECT_PLUGIN_DLLAPI VISUAL_EFFECT_PLUGIN_DLLEXPORT
#else
#   define VISUAL_EFFECT_PLUGIN_DLLAPI VISUAL_EFFECT_PLUGIN_DLLIMPORT
#endif

#endif

#ifdef CNOID_EXPORT
# undef CNOID_EXPORT
#endif
#define CNOID_EXPORT VISUAL_EFFECT_PLUGIN_DLLAPI
