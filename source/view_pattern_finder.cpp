#include "view_pattern_finder.hpp"

PatternFinderView::PatternFinderView() : hex::View("Pattern Finder")
{
    hex::EventManager::subscribe<hex::EventDataChanged>(this, [this]()
                                                        {
                                                            this->m_pattern.clear();
                                                            this->m_results.clear();
                                                            this->m_pattern_size = 0;
                                                        });
    this->m_pattern.reserve(0xFFFF);
    std::memset(this->m_pattern.data(), 0x00, this->m_pattern.capacity());
}
PatternFinderView::~PatternFinderView()
{
    hex::EventManager::unsubscribe<hex::EventDataChanged>(this);
}
std::vector<uint16_t> PatternFinderView::ConvertIDAPatternToByteVector(const std::string &pattern)
{
    std::vector<u16> byteBuffer;

    for (auto i = pattern.cbegin(); i != pattern.cend(); ++i)
    {
        if (*i == ' ')
            continue;

        if (*i == '?')
        {
            if (*(i + 1) == '?')
                ++i;
            byteBuffer.push_back(256u);
        }
        else
        {
            byteBuffer.push_back(strtol(&pattern[distance(pattern.cbegin(), i)], nullptr, 16));
            ++i;
        }
    }

    return byteBuffer;
}

void PatternFinderView::findPattern(const std::vector<u8> &pattern, const std::vector<u8> &mask)
{
    this->m_results.clear();
    std::vector<u64> results;
    auto provider = hex::ImHexApi::Provider::get();

    if (hex::ImHexApi::Provider::isValid() && provider->isReadable())
    {
        for (auto offset = 0; offset <= provider->getSize() - pattern.size(); ++offset)
        {
            u8 buffer[pattern.size()];
            provider->readRaw(offset, buffer, pattern.size());
            bool succ = true;
            for (auto i = 0u; i < pattern.size(); i++)
            {
                if (mask[i] == (char)MaskType::ANY)
                {
                    continue;
                }
                else if (mask[i] == (char)MaskType::EQ)
                {
                    if (buffer[i] != pattern[i])
                    {
                        succ = false;
                        break;
                    }
                }
                else if (mask[i] == (char)MaskType::GT)
                {
                    if (buffer[i] <= pattern[i])
                    {
                        succ = false;
                        break;
                    }
                }
                else if (mask[i] == (char)MaskType::LT)
                {
                    if (buffer[i] >= pattern[i])
                    {
                        succ = false;
                        break;
                    }
                }
                else if (mask[i] == (char)MaskType::NOT)
                {
                    if (buffer[i] == pattern[i])
                    {
                        succ = false;
                        break;
                    }
                }
                else
                {
                    hex::log::error("Non masked character in mask {}", mask[i]);
                    return;
                }
            }
            if (succ)
            {
                m_results.push_back(offset);
            }
        }
    }
}

void PatternFinderView::FindPattern(const std::vector<uint16_t> &pattern)
{
    this->m_results.clear();
    std::vector<u64> results;
    auto provider = hex::ImHexApi::Provider::get();

    if (hex::ImHexApi::Provider::isValid() && provider->isReadable())
    {
        for (auto offset = 0; offset <= provider->getSize() - pattern.size(); ++offset)
        {
            u8 buffer[pattern.size()];
            provider->readRaw(offset, buffer, pattern.size());
            bool succ = true;
            for (auto i = 0u; i < pattern.size(); i++)
            {
                if (buffer[i] != pattern[i])
                {
                    if (pattern[i] != 256u)
                    {
                        succ = false;
                        break;
                    }
                }
            }
            if (succ)
            {
                m_results.push_back(offset);
            }
        }
    }
}

void PatternFinderView::search()
{
    if (this->m_pattern.size() > 0)
    {
        std::thread([this]
                    {
                        this->m_searching = true;
                        auto p = PatternFinderView::ConvertIDAPatternToByteVector(this->m_pattern);
                        std::vector<u8> pattern(p.begin(), p.end());
                        std::vector<u8> mask(this->m_mask.begin(), this->m_mask.end());
                        this->m_pattern_size = pattern.size();
                        if (this->m_pattern_size > 0)
                        {
                            findPattern(pattern, mask);
                        }
                        this->m_searching = false;
                    })
            .detach();
    }
}

void PatternFinderView::drawContent()
{

    auto provider = hex::ImHexApi::Provider::get();
    if (ImGui::Begin("Pattern Finder", &this->getWindowOpenState(), ImGuiWindowFlags_NoCollapse))
    {
        if (hex::ImHexApi::Provider::isValid() && provider->isReadable())
        {
            ImGui::Text("Patternformat: DE AD BE ?? 01 02 03");
            ImGui::Disabled([this]
                            {
                                bool pattern_matching = false;
                                ImGui::InputText(
                                    "Pattern", this->m_pattern.data(), this->m_pattern.capacity(), ImGuiInputTextFlags_CallbackEdit, [](ImGuiInputTextCallbackData *data)
                                    {
                                        const std::regex pattern_regex("([a-fA-F0-9]{2}\\s*){2,}");
                                        auto &view = *static_cast<PatternFinderView *>(data->UserData);
                                        view.m_pattern.resize(data->BufTextLen);
                                        view.m_matching_pattern = std::regex_match(data->Buf, pattern_regex);
                                        return 0;
                                    },
                                    this);
                                bool mask_matching = false;
                                ImGui::InputText(
                                    "Mask", this->m_mask.data(), this->m_mask.capacity(), ImGuiInputTextFlags_CallbackEdit, [](ImGuiInputTextCallbackData *data)
                                    {
                                        auto &view = *static_cast<PatternFinderView *>(data->UserData);
                                        view.m_mask.resize(data->BufTextLen);
                                        return 0;
                                    },
                                    this);
                                ImGui::Disabled([this]
                                                {
                                                    if (ImGui::Button("Find All"))
                                                    {
                                                        this->search();
                                                    }
                                                },
                                                !this->m_matching_pattern);
                            },
                            this->m_searching);
            if (this->m_searching)
            {
                ImGui::SameLine();
                ImGui::TextSpinner("Searching ...");
            }

            if (ImGui::BeginTable("##PatternFindResults", 1, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Sortable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY))
            {
                ImGui::TableSetupScrollFreeze(0, 1);
                ImGui::TableSetupColumn("Offset", 0, -1, ImGui::GetID("offset"));
                ImGui::TableHeadersRow();
                ImGuiListClipper clipper;
                clipper.Begin(this->m_results.size());
                while (clipper.Step())
                {
                    for (u64 i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
                    {
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        if (ImGui::Selectable(("##StringLine" + std::to_string(i)).c_str(), false, ImGuiSelectableFlags_SpanAllColumns))
                        {
                            hex::EventManager::post<hex::RequestSelectionChange>(hex::Region{m_results[i], m_pattern_size});
                        }
                        ImGui::SameLine();
                        ImGui::Text("0x%08lx", this->m_results[i]);
                    }
                }
                clipper.End();
                ImGui::EndTable();
            }
        }
    }
    ImGui::End();
}