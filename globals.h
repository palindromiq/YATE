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
#define MAX_NIGHT_DURATION 4800.0
#define FIRST_SET_NIGHT_START_THRESHOLD 6.0

#define ANALYSIS_STAT_NIGHTS QObject::tr("Nights")
#define ANALYSIS_STAT_NIGHT_NO QObject::tr("Night #")
#define ANALYSIS_STAT_RUNS QObject::tr("Runs")
#define ANALYSIS_STAT_RUN_NO QObject::tr("Run #")
#define ANALYSIS_STAT_TERRALYST QObject::tr("Terralyst")
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
#define ANALYSIS_STAT_LAST_LIMB QObject::tr("Last Limb Time")
#define ANALYSIS_STAT_LIMBS_AVERAGE QObject::tr("Limbs Average")
#define ANALYSIS_STAT_CAPSHOT_TIME QObject::tr("Capshot Time")
#define ANALYSIS_STAT_RESULT QObject::tr("Result")
#define ANALYSIS_STAT_HEADER1 QObject::tr("Attribute")
#define ANALYSIS_STAT_HEADER2 QObject::tr("Value")

#define THREADED_PARSING

#define FORMAT_NUMBER(x) QString::number(x, 'f', 3)



#endif // GLOBALS_H
