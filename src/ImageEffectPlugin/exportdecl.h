#ifndef IMAGE_EFFECT_PLUGIN_EXPORTDECL_H_INCLUDED
# define IMAGE_EFFECT_PLUGIN_EXPORTDECL_H_INCLUDED

# if defined _WIN32 || defined __CYGWIN__
#  define IMAGE_EFFECT_PLUGIN_DLLIMPORT __declspec(dllimport)
#  define IMAGE_EFFECT_PLUGIN_DLLEXPORT __declspec(dllexport)
#  define IMAGE_EFFECT_PLUGIN_DLLLOCAL
# else
#  if __GNUC__ >= 4
#   define IMAGE_EFFECT_PLUGIN_DLLIMPORT __attribute__ ((visibility("default")))
#   define IMAGE_EFFECT_PLUGIN_DLLEXPORT __attribute__ ((visibility("default")))
#   define IMAGE_EFFECT_PLUGIN_DLLLOCAL  __attribute__ ((visibility("hidden")))
#  else
#   define IMAGE_EFFECT_PLUGIN_DLLIMPORT
#   define IMAGE_EFFECT_PLUGIN_DLLEXPORT
#   define IMAGE_EFFECT_PLUGIN_DLLLOCAL
#  endif
# endif

#ifdef CnoidImageEffectPlugin_EXPORTS
#   define IMAGE_EFFECT_PLUGIN_DLLAPI IMAGE_EFFECT_PLUGIN_DLLEXPORT
#else
#   define IMAGE_EFFECT_PLUGIN_DLLAPI IMAGE_EFFECT_PLUGIN_DLLIMPORT
#endif

#endif

#ifdef CNOID_EXPORT
# undef CNOID_EXPORT
#endif
#define CNOID_EXPORT IMAGE_EFFECT_PLUGIN_DLLAPI
