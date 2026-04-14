//
// Created by jamie on 2025/5/3.
//
#pragma once

namespace SosGui::Settings
{
struct UiSettings
{
    static constexpr std::string_view ICON_FONT = "lucide-icons.ttf";

    enum DefaultThemeIndex
    {
        DefaultThemeIndex_Classic = -1,
        DefaultThemeIndex_Dark    = -2,
        DefaultThemeIndex_Light   = -3,
        DefaultThemeIndex_Invalid = -4,
    };

    int32_t selectedThemeIndex = DefaultThemeIndex_Invalid;

    static auto GetInstance() -> UiSettings *
    {
        static UiSettings g_instance;
        return &g_instance;
    }
};
} // namespace SosGui::Settings
