#ifndef GLOBALS_H
#define GLOBALS_H

QT_BEGIN_NAMESPACE
class QString;
QT_END_NAMESPACE

// that way you aren't compiling QString into every header file you put this in...
// aka faster build times.

#define PATH_EE_LOG_RELATIVE "Warframe/EE.log"
#define SETTINGS_ORGANIZATION "palindromiq"
#define SETTINGS_APPLICATION "yate"
#define SETTINGS_KEY_EE_LOG "eelogpath"
#define SETTINGS_KEY_WATERSHIELD "watershield"
#define SETTINGS_OPT_EXACT "exact"
#define SETTINGS_OPT_SHARD "shard"
#define SETTINGS_KEY_FEEDBACK_POS_X "feedback_xpos"
#define SETTINGS_KEY_FEEDBACK_POS_Y "feedback_ypos"

#define LIMB_BREAK_ANIMATION_TIME 17.186
#define CAPSHOT_ANIMATION_TIME 48.109

#endif // GLOBALS_H
