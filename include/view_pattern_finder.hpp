#include <hex/plugin.hpp>
#include <hex/views/view.hpp>
#include <hex/providers/provider.hpp>
#include <hex/api/imhex_api.hpp>
#include <hex/api/event.hpp>
#include <hex/helpers/logger.hpp>
#include <thread>
#include <regex>

enum class MaskType : char
{
    EQ = '.',
    GT = '>',
    LT = '<',
    NOT = '!',
    ANY = '?'
};

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
    u32 m_pattern_size = 0;
    bool m_searching = false;
    bool m_matching_pattern = false;
    bool m_matching_mask = false;
    bool m_advanced_mode = false;
    const std::regex advanced_pattern_regex{"([a-fA-F0-9]{2}\\s*){2,}"};
    const std::regex simple_pattern_regex{"([a-fA-F0-9]{2}\\s|\\?{2}\\s)+([a-fA-F0-9]{2}|\\?{2})\\s*$"};

    static std::vector<u16> ConvertIDAPatternToByteVector(const std::string &pattern);
    void FindPattern(const std::vector<u16> &pattern);
    void FindPattern(const std::vector<u16> &pattern, const std::vector<u8> &mask);
    void search();
};