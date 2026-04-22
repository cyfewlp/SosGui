//
// Created by jamie on 2026/4/22.
//
#include "Settings.h"

#include <log.h>
#include <path_utils.h>
#include <toml++/toml.hpp>

namespace SosGui::SettingsLoader
{

namespace
{

struct Key
{
    const char *key_name;
    uint32_t    keycode;
};

static const std::vector<Key> named_keys = {
    {"escape",       0x01},
    {"num1",         0x02},
    {"num2",         0x03},
    {"num3",         0x04},
    {"num4",         0x05},
    {"num5",         0x06},
    {"num6",         0x07},
    {"num7",         0x08},
    {"num8",         0x09},
    {"num9",         0x0A},
    {"num0",         0x0B},
    {"minus",        0x0C},
    {"equals",       0x0D},
    {"backspace",    0x0E},
    {"tab",          0x0F},
    {"q",            0x10},
    {"w",            0x11},
    {"e",            0x12},
    {"r",            0x13},
    {"t",            0x14},
    {"y",            0x15},
    {"u",            0x16},
    {"i",            0x17},
    {"o",            0x18},
    {"p",            0x19},
    {"bracketleft",  0x1A},
    {"bracketright", 0x1B},
    {"enter",        0x1C},
    // {"leftcontrol",  0x1D},
    {"a",            0x1E},
    {"s",            0x1F},
    {"d",            0x20},
    {"f",            0x21},
    {"g",            0x22},
    {"h",            0x23},
    {"j",            0x24},
    {"k",            0x25},
    {"l",            0x26},
    {"semicolon",    0x27},
    {"apostrophe",   0x28},
    {"tilde",        0x29},
    // {"leftshift",    0x2A},
    {"backslash",    0x2B},
    {"z",            0x2C},
    {"x",            0x2D},
    {"c",            0x2E},
    {"v",            0x2F},
    {"b",            0x30},
    {"n",            0x31},
    {"m",            0x32},
    {"comma",        0x33},
    {"period",       0x34},
    {"slash",        0x35},
    // {"rightshift",   0x36},
    {"kp_multiply",  0x37},
    // {"leftalt",      0x38},
    {"spacebar",     0x39},
    {"capslock",     0x3A},
    {"f1",           0x3B},
    {"f2",           0x3C},
    {"f3",           0x3D},
    {"f4",           0x3E},
    {"f5",           0x3F},
    {"f6",           0x40},
    {"f7",           0x41},
    {"f8",           0x42},
    {"f9",           0x43},
    {"f10",          0x44},
    {"numlock",      0x45},
    {"scrolllock",   0x46},
    {"kp_7",         0x47},
    {"kp_8",         0x48},
    {"kp_9",         0x49},
    {"kp_subtract",  0x4A},
    {"kp_4",         0x4B},
    {"kp_5",         0x4C},
    {"kp_6",         0x4D},
    {"kp_plus",      0x4E},
    {"kp_1",         0x4F},
    {"kp_2",         0x50},
    {"kp_3",         0x51},
    {"kp_0",         0x52},
    {"kp_decimal",   0x53},
    {"f11",          0x57},
    {"f12",          0x58},
    {"kp_enter",     0x9C},
    // {"rightcontrol", 0x9D},
    {"kp_divide",    0xB5},
    {"printscreen",  0xB7},
    // {"rightalt",     0xB8},
    {"pause",        0xC5},
    {"home",         0xC7},
    {"up",           0xC8},
    {"pageup",       0xC9},
    {"left",         0xCB},
    {"right",        0xCD},
    {"end",          0xCF},
    {"down",         0xD0},
    {"pagedown",     0xD1},
    {"insert",       0xD2},
    {"delete",       0xD3},
    // {"leftwin",      0xDB},
    // {"rightwin",     0xDC}
};

auto try_parse_named_key(MenuToggleShortcut &shortcut, std::string_view name) -> bool
{
    const auto it = std::ranges::find(named_keys, name, &Key::key_name);
    if (it != named_keys.end())
    {
        shortcut.key = it->keycode;
    }
    return it != named_keys.end();
}

auto try_parse_key_with_modifier(MenuToggleShortcut &shortcut, std::string_view name) -> bool
{
    bool is_modifier_key = false;
    if (name == "ctrl")
    {
        is_modifier_key = true;
        shortcut.ctrl   = true;
    }
    else if (name == "shift")
    {
        is_modifier_key = true;
        shortcut.shift  = true;
    }
    else if (name == "alt")
    {
        is_modifier_key = true;
        shortcut.alt    = true;
    }
    return is_modifier_key;
}

auto parse_shortcut(std::string_view shortcut_strv) -> std::optional<MenuToggleShortcut>
{
    MenuToggleShortcut shortcut;
    bool               parseComplete = false;
    // shortcut can't be only modifier key, must have a normal key. e.g. "Ctrl+Shift" is invalid, but "Ctrl+F1" is valid.
    bool               onlyModifier  = true;

    std::string shortcutErasedSpace;
    shortcutErasedSpace.reserve(shortcut_strv.size());
    for (const char c : shortcut_strv)
    {
        if (std::isspace(c) == 0)
        {
            shortcutErasedSpace.push_back(static_cast<char>(std::tolower(c)));
        }
    }

    size_t                 startPos              = 0;
    size_t                 partPos               = 0;
    const std::string_view shortcutErasedSpaceSv = shortcutErasedSpace;
    while (startPos < shortcutErasedSpace.size() && partPos != std::string::npos)
    {
        partPos = shortcutErasedSpaceSv.find_first_of('+', startPos);
        const auto key_name =
            (partPos == std::string::npos) ? shortcutErasedSpaceSv.substr(startPos) : shortcutErasedSpaceSv.substr(startPos, partPos - startPos);
        if (try_parse_key_with_modifier(shortcut, key_name))
        {
            startPos      = partPos + 1;
            parseComplete = (partPos == std::string::npos);
        }
        else if (try_parse_named_key(shortcut, key_name))
        {
            if (!onlyModifier) // already contain a named key!
            {
                parseComplete = false;
                break;
            }
            startPos      = partPos + 1;
            onlyModifier  = false;
            parseComplete = (partPos == std::string::npos);
        }
    }
    if (parseComplete && !onlyModifier)
    {
        return shortcut;
    }
    return std::nullopt;
}
} // namespace

auto load() -> void
{
    constexpr std::string_view settings_file = "sosgui_settings.toml";
    Settings                  &settings      = Settings::get_instance();
    const auto                 settings_path = utils::GetPluginInterfaceDir() / settings_file;
    if (std::filesystem::exists(settings_path))
    {
        try
        {
            const auto table   = toml::parse_file(settings_path.generic_string());
            settings.language  = table["language"].value_or("english");
            settings.debug_log = table["debug"].value_or(false);
            if (const auto shortcut_opt = parse_shortcut(table["shortcut"].value_or("ctrl+o")); shortcut_opt)
            {
                settings.menu_toggle_shortcut = shortcut_opt.value();
            }
            else
            {
                settings.menu_toggle_shortcut =
                    MenuToggleShortcut{.key = RE::BSWin32KeyboardDevice::Key::kO, .ctrl = true, .shift = false, .alt = false};
            }
        }
        catch (const std::exception &e)
        {
            logger::warn("SosGuiMenu: failed to settings: {}", e.what());
        }
    }
}

} // namespace SosGui::SettingsLoader

auto SosGui::Settings::get_instance() -> Settings &
{
    static Settings g_settings;
    return g_settings;
}