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
#define SETTINGS_KEY_FEEDBACK_FONT "feedback_font"
#define SETTINGS_KEY_SHOW_LIMBS "show_limbs"
#define SETTINGS_KEY_SHOW_LIMBS_AFTER_LAST "show_limbs_after_last"
#define SETTINGS_FEEDBACK_FONT_DEFAULT 10
#define SETTINGS_KEY_LIMBS_PREC "limbs_precision"
#define SETTINGS_LIMBS_PREC_DEFAULT 3
#define SETTINGS_KEY_LOCK_FEEDBACK_BTN "lock_feedback_btn"
#define SETTINGS_KEY_STREAMER_MODE "streamer_mode"

#define EMOJI_LOCKED "🔒"
#define EMOJI_UNLOCKED "🔓︎"


#define LIMB_BREAK_ANIMATION_TIME 17.186
#define CAPSHOT_ANIMATION_TIME 48.109
#define MAX_NIGHT_DURATION 4800.0
#define FIRST_SET_NIGHT_START_THRESHOLD 6.0

#define LIVE_FEEDBACK_DEFAULT_MSG QObject::tr(" [Live Feedback will show here]")

#define ANALYSIS_STAT_NIGHTS QObject::tr("Nights")
#define ANALYSIS_STAT_NIGHT_NO QObject::tr("Night #")
#define ANALYSIS_STAT_RUNS QObject::tr("Runs")
#define ANALYSIS_STAT_RUN_NO QObject::tr("Run #")
#define ANALYSIS_STAT_TERALYST QObject::tr("Teralyst")
#define ANALYSIS_STAT_GANTULYST QObject::tr("Gantulyst")
#define ANALYSIS_STAT_HYDROLYST QObject::tr("Hydrolyst")
#define ANALYSIS_STAT_TERRY QObject::tr("Terry")
#define ANALYSIS_STAT_GARRY QObject::tr("Garry")
#define ANALYSIS_STAT_HARRY QObject::tr("Harry")
#define ANALYSIS_STAT_LIMB_BREAKS QObject::tr("Limb Breaks")
#define ANALYSIS_STAT_RESULT_INCOMPLETE QObject::tr("No Spawn")
#define ANALYSIS_STAT_RESULT_SPAWNED QObject::tr("Spawned")
#define ANALYSIS_STAT_RESULT_KILLED QObject::tr("Killed")
#define ANALYSIS_STAT_RESULT_CAPTURED QObject::tr("Captured")
#define ANALYSIS_STAT_SHRINE_TIME QObject::tr("Shrine Time")
#define ANALYSIS_STAT_BETWEEN_SHARDS QObject::tr("Time Between Shards")
#define ANALYSIS_STAT_SPAWN_DELAY QObject::tr("Spawn Delay")
#define ANALYSIS_STAT_WATERSHIELD QObject::tr("Watershield")
#define ANALYSIS_STAT_SHRINE_ACTIVASION QObject::tr("Shrine Delay")
#define ANALYSIS_STAT_LOOT_DROP QObject::tr("Loot Drop")
#define ANALYSIS_STAT_SHRINE_ACTIVATION QObject::tr("Shrine Activation")
#define ANALYSIS_STAT_CAPSHOT QObject::tr("Capshot")
#define ANALYSIS_STAT_LAST_LIMB QObject::tr("Last Limb Clock")
#define ANALYSIS_STAT_LIMBS_AVERAGE QObject::tr("Limbs Average")
#define ANALYSIS_STAT_CAPSHOT_TIME QObject::tr("Capshot Clock")
#define ANALYSIS_STAT_RESULT QObject::tr("Result")
#define ANALYSIS_STAT_HEADER1 QObject::tr("Attribute")
#define ANALYSIS_STAT_HEADER2 QObject::tr("Value")
#define ANALYSIS_STAT_LIMBS QObject::tr("Limbs: ")

#define THREADED_PARSING

#define FORMAT_NUMBER(x) QString::number(x, 'f', 3)
#define FORMAT_NUMBER_PREC(x, y) QString::number(x, 'f', y)



#endif // GLOBALS_H
