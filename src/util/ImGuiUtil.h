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

// ImGui v1.92

namespace SosGui::ImGuiUtil
{
static constexpr ImColor RED_COLOR = ImColor(255, 0, 0, 255);

constexpr void Text(std::string_view text)
{
    ImGui::TextUnformatted(ImGuiEx::TextStart(text), ImGuiEx::TextEnd(text));
}

auto Icon(std::string_view icon) -> void;
auto IconButton(std::string_view icon, const ImVec2 &size = {}) -> bool;

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
