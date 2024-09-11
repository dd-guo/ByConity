#include <Statistics/StatisticsSettings.h>

namespace DB::Statistics
{
void refreshClusterStatsCache(ContextPtr context, const StatsTableIdentifier & table_identifier, bool is_drop);
StatisticsSettings fetchStatisticsSettings(ContextPtr context);
}
