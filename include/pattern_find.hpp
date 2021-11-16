#pragma once
#include <hex/plugin.hpp>
#include <hex/helpers/logger.hpp>
#include <hex/providers/provider.hpp>
#include <hex/api/imhex_api.hpp>
#include <vector>
#include <regex>

namespace PatternFind
{
    enum class MaskType : char
    {
        EQ = '.',
        GT = '>',
        LT = '<',
        NEQ = '!',
        ANY = '?'
    };
    const std::regex advanced_pattern_regex{"([a-fA-F0-9]{2}\\s*){2,}"};
    const std::regex simple_pattern_regex{"([a-fA-F0-9]{2}\\s|\\?{2}\\s)+([a-fA-F0-9]{2}|\\?{2})\\s*$"};

    std::vector<u16> ConvertIDAPatternToByteVector(const std::string &pattern);
    std::vector<u64> Find(const std::vector<u16> &pattern);
    std::vector<u64> Find(const std::vector<u16> &pattern, const std::vector<u8> &mask);
};