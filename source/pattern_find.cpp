#include <pattern_find.hpp>

std::vector<uint16_t> PatternFind::ConvertIDAPatternToByteVector(const std::string &pattern)
{
    std::vector<u16> buffer;

    for (auto i = pattern.cbegin(); i != pattern.cend(); ++i)
    {
        if (*i == ' ')
            continue;

        if (*i == '?')
        {
            if (*(i + 1) == '?')
                ++i;
            buffer.push_back(256u);
        }
        else
        {
            buffer.push_back(strtol(&pattern[distance(pattern.cbegin(), i)], nullptr, 16));
            ++i;
        }
    }

    return buffer;
}

std::vector<u64> PatternFind::Find(const std::vector<u16> &pattern, const std::vector<u8> &mask)
{
    std::vector<u64> results;
    auto provider = hex::ImHexApi::Provider::get();
    size_t bufferSize = std::min(provider->getSize(), 0xFFFFlu);
    u8 buffer[bufferSize];
    hex::log::debug("Buffersize {} bytes", bufferSize);
    if (hex::ImHexApi::Provider::isValid() && provider->isReadable())
    {
        u16 matching_bytes = 0;
        for (auto offset = 0; offset <= provider->getSize() - pattern.size(); offset += bufferSize)
        {
            size_t readSize = std::min(bufferSize, provider->getSize() - offset);
            provider->readRaw(offset, buffer, readSize);
            for (auto i = 0u; i < readSize - pattern.size(); i++)
            {
                for (auto j = 0u; j < pattern.size(); j++)
                {
                    bool match = false;
                    switch ((MaskType)mask[j])
                    {
                    case MaskType::ANY:
                    {
                        match = true;
                        break;
                    }
                    case MaskType::EQ:
                    {
                        match = (buffer[i + j] == pattern[j]);
                        break;
                    }
                    case MaskType::NEQ:
                    {
                        match = (buffer[i + j] != pattern[j]);
                        break;
                    }
                    case MaskType::GT:
                    {
                        match = (buffer[i + j] > pattern[j]);
                        break;
                    }
                    case MaskType::LT:
                    {
                        match = (buffer[i + j] < pattern[j]);
                        break;
                    }
                    }
                    if (!match)
                    {
                        matching_bytes = 0;
                        break;
                    }
                    else
                        ++matching_bytes;
                }
                if (matching_bytes == pattern.size())
                {
                    results.push_back(offset + i);
                    matching_bytes = 0;
                }
            }
        }
    }
    return results;
}

std::vector<u64> PatternFind::Find(const std::vector<uint16_t> &pattern)
{
    std::vector<u64> results;
    auto provider = hex::ImHexApi::Provider::get();
    size_t bufferSize = std::min(provider->getSize(), 0xFFFFlu);
    u8 buffer[bufferSize];
    hex::log::debug("Buffersize {} bytes", bufferSize);
    if (hex::ImHexApi::Provider::isValid() && provider->isReadable())
    {
        u16 matching_bytes = 0;
        for (auto offset = 0; offset <= provider->getSize() - pattern.size(); offset += bufferSize)
        {
            size_t readSize = std::min(bufferSize, provider->getSize() - offset);
            provider->readRaw(offset, buffer, readSize);
            for (auto i = 0u; i < readSize - pattern.size(); i++)
            {
                for (auto j = 0u; j < pattern.size(); j++)
                {
                    if (pattern[j] == 256u)
                    {
                        matching_bytes++;
                        continue;
                    }
                    else if (pattern[j] == buffer[i + j])
                    {
                        matching_bytes++;
                        continue;
                    }
                    else
                    {
                        matching_bytes = 0;
                        break;
                    }
                }
                if (matching_bytes == pattern.size())
                {
                    results.push_back(offset + i);
                    matching_bytes = 0;
                }
            }
        }
    }
    return results;
}