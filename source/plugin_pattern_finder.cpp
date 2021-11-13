#include "view_pattern_finder.hpp"

IMHEX_PLUGIN_SETUP("Pattern Finder", "QDL", "Search IDA-Style Patterns")
{
    ContentRegistry::Views::add<PatternFinderView>();
}