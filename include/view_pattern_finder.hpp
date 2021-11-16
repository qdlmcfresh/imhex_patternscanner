#include <hex/plugin.hpp>
#include <hex/views/view.hpp>
#include <hex/providers/provider.hpp>
#include <hex/api/imhex_api.hpp>
#include <hex/api/event.hpp>
#include <hex/helpers/logger.hpp>
#include <thread>
#include <regex>
#include <chrono>
#include "pattern_find.hpp"

class PatternFinderView : public hex::View
{
public:
    PatternFinderView();
    ~PatternFinderView() override;
    void drawContent() override;

private:
    std::string m_pattern;
    std::string m_mask;
    std::vector<u16> m_pattern_vec;
    std::vector<u64> m_results;
    bool m_searching = false;
    bool m_matching_pattern = false;
    bool m_matching_mask = false;
    bool m_advanced_mode = false;
    std::chrono::milliseconds m_search_duration;
    void search();
};