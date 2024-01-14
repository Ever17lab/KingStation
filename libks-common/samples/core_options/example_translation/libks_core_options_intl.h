#ifndef LIBKS_CORE_OPTIONS_INTL_H__
#define LIBKS_CORE_OPTIONS_INTL_H__

#if defined(_MSC_VER) && (_MSC_VER >= 1500 && _MSC_VER < 1900)
/* https://support.microsoft.com/en-us/kb/980263 */
#pragma execution_character_set("utf-8")
#pragma warning(disable:4566)
#endif

#include <libks.h>

/*
 ********************************
 * VERSION: 1.3
 ********************************
 *
 * - 1.3: Move translations to libks_core_options_intl.h
 *        - libks_core_options_intl.h includes BOM and utf-8
 *          fix for MSVC 2010-2013
 *        - Added HAVE_NO_LANGEXTRA flag to disable translations
 *          on platforms/compilers without BOM support
 * - 1.2: Use core options v1 interface when
 *        KS_ENVIRONMENT_GET_CORE_OPTIONS_VERSION is >= 1
 *        (previously required KS_ENVIRONMENT_GET_CORE_OPTIONS_VERSION == 1)
 * - 1.1: Support generation of core options v0 ks_core_option_value
 *        arrays containing options with a single value
 * - 1.0: First commit
*/

#ifdef __cplusplus
extern "C" {
#endif

/*
 ********************************
 * Core Option Definitions
 ********************************
*/

/* KS_LANGUAGE_JAPANESE */

/* KS_LANGUAGE_FRENCH */

struct ks_core_option_definition option_defs_fr[] = {
   {
      "mycore_region",                             /* key must match option_defs_us entry */
      "Région de la console",                      /* translated description */
      "Spécifiez la région d'origine du système.", /* translated sublabel */
      {
         { "auto",   "Auto" },                     /* value must match option_defs_us entry   */
         { "ntsc-j", "Japon" },                    /* > only value_label should be translated */
         { "ntsc-u", "Amérique" },
         { "pal",    "L'Europe" },
         { NULL, NULL },
      },
      NULL                                         /* default_value is taken from option_defs_us -> can set to NULL here */
   },
   {
      "mycore_video_scale",
      "Échelle vidéo",
      "Définir le facteur d'échelle vidéo interne.",
      {
         { NULL, NULL }, /* If value_labels do not require translation (e.g. numbers), values may be omitted */
      },
      NULL
   },
   {
      "mycore_overclock",
      "Réduire le ralentissement",
      "Activer l'overclocking du processeur (non sécurisé).",
      {
         { NULL, NULL }, /* 'enabled' and 'disabled' values should not be translated */
      },
      NULL
   },
   { NULL, NULL, NULL, {{0}}, NULL },
};

/* KS_LANGUAGE_SPANISH */

/* KS_LANGUAGE_GERMAN */

/* KS_LANGUAGE_ITALIAN */

/* KS_LANGUAGE_DUTCH */

/* KS_LANGUAGE_PORTUGUESE_BRAZIL */

/* KS_LANGUAGE_PORTUGUESE_PORTUGAL */

/* KS_LANGUAGE_RUSSIAN */

/* KS_LANGUAGE_KOREAN */

/* KS_LANGUAGE_CHINESE_TRADITIONAL */

/* KS_LANGUAGE_CHINESE_SIMPLIFIED */

/* KS_LANGUAGE_ESPERANTO */

/* KS_LANGUAGE_POLISH */

/* KS_LANGUAGE_VIETNAMESE */

/* KS_LANGUAGE_ARABIC */

/* KS_LANGUAGE_GREEK */

/* KS_LANGUAGE_TURKISH */

/* KS_LANGUAGE_SLOVAK */

/* KS_LANGUAGE_PERSIAN */

/* KS_LANGUAGE_HEBREW */

/* KS_LANGUAGE_ASTURIAN */

/* KS_LANGUAGE_FINNISH */

#ifdef __cplusplus
}
#endif

#endif
