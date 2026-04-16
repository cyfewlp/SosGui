//
// Created by jamie on 2025/4/6.
//

#pragma once

#include "Translation.h"
#include "imgui.h"
#include "imguiex/ImGuiEx.h"
#include "imguiex/icon.h"

#include <array>
#include <string>

namespace SosGui::ImGuiUtil
{
static constexpr ImColor RED_COLOR = ImColor(255, 0, 0, 255);

constexpr void Text(std::string_view text)
{
    ImGui::TextUnformatted(ImGuiEx::TextStart(text), ImGuiEx::TextEnd(text));
}

auto Icon(std::string_view icon) -> void;
auto IconButton(std::string_view icon, const ImVec2 &size = {}) -> bool;

namespace MultiSelection
{

/**
 * @brief Simplest implementation of multi selection. Support single column sort direction.
 *
 * In our case, we won't really sort view data, instead we just reverse the index when sort direction is descended.
 * But the multi-selection always assume the index is in ascend order, so when sort direction is descended, we need to reverse the RangeFirstItem and
 * RangeLastItem.
 */
constexpr void ApplyRequest(ImGuiSelectionBasicStorage &selection, const ImGuiMultiSelectIO *ms_io, const bool ascend)
{
    for (const ImGuiSelectionRequest &req : ms_io->Requests)
    {
        if (req.Type == ImGuiSelectionRequestType_SetAll)
        {
            selection.Clear();
            if (req.Selected)
            {
                for (int n = 0; n < ms_io->ItemsCount; n++)
                {
                    selection.SetItemSelected(selection.GetStorageIdFromIndex(n), true);
                }
            }
        }
        else if (req.Type == ImGuiSelectionRequestType_SetRange)
        {
            const auto start = ascend ? req.RangeFirstItem : req.RangeLastItem;
            const auto end   = ascend ? req.RangeLastItem : req.RangeFirstItem;
            for (int n = static_cast<int>(start); n <= static_cast<int>(end); n++)
            {
                selection.SetItemSelected(selection.GetStorageIdFromIndex(n), req.Selected);
            }
        }
    }
}
} // namespace MultiSelection

struct DebounceInput
{
    std::chrono::time_point<std::chrono::system_clock> prevEditTime;
    ImGuiTextFilter                                    filter;
    std::chrono::milliseconds                          duration = 200ms;
    bool                                               dirty    = true;

    explicit DebounceInput() : prevEditTime(std::chrono::system_clock::now()) {}

    virtual ~DebounceInput() = default;

    bool PassFilter(std::string_view text) const { return filter.PassFilter(text.data(), text.data() + text.size()); }

    virtual void OnInput()
    {
        filter.Build();
        prevEditTime = std::chrono::system_clock::now();
        dirty        = true;
    }

    virtual bool Draw(const char *label, const char *hintText);

    virtual void Clear();
};

} // namespace SosGui::ImGuiUtil
