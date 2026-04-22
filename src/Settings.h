//
// Created by jamie on 2026/4/22.
//

#pragma once

#include <string>

namespace SosGui
{
struct MenuToggleShortcut
{
    uint32_t key   = 0;
    bool     ctrl  = false;
    bool     shift = false;
    bool     alt   = false;
};

struct Settings
{
    std::string        language;
    MenuToggleShortcut menu_toggle_shortcut;
    bool               debug_log = false;

    static auto get_instance() -> Settings &;
};

namespace SettingsLoader
{
auto load() -> void;
}

} // namespace SosGui