#include "util/ImGuiUtil.h"

namespace SosGui
{

namespace
{
/**
 * FIXME: Retrieve the icon directly from the @c ImFontAtlas::Fonts collection.
 * @note **Assumed Convention:**
 * This implementation relies on the following layout:
 * 1. The Icon Font is never merged.
 * 2. The Icon Font is always the second entry in the atlas index [1].
 * 3. Fallback: If the icon font is unavailable, the default font will be used.
 */
auto GetIconFont() -> ImFont *
{
    ImFontAtlas *fontAtlas = ImGui::GetIO().Fonts;
    if (fontAtlas->Fonts.size() > 1)
    {
        return fontAtlas->Fonts[1];
    }
    return ImGui::GetFont();
}

} // namespace

auto ImGuiUtil::Icon(std::string_view icon) -> void
{
    ImGui::PushFont(GetIconFont(), 0.0F);
    ImGui::TextUnformatted(ImGuiEx::TextStart(icon), ImGuiEx::TextEnd(icon));
    ImGui::PopFont();
}

auto ImGuiUtil::IconButton(std::string_view icon, const ImVec2 &size) -> bool
{
    ImGui::PushFont(GetIconFont(), 0.0F);
    const auto clicked = ImGui::Button(icon.data(), size);
    ImGui::PopFont();
    return clicked;
}

bool ImGuiUtil::DebounceInput::Draw(const char *label, const char *hintText)
{
    ImGui::SetNextItemShortcut(ImGuiMod_Ctrl | ImGuiKey_F, ImGuiInputFlags_Tooltip);
    ImGui::PushItemFlag(ImGuiItemFlags_NoNavDefaultFocus, true);
    if (ImGui::InputTextWithHint(label, hintText, filter.InputBuf, IM_ARRAYSIZE(filter.InputBuf), ImGuiInputTextFlags_EscapeClearsAll))
    {
        OnInput();
    }
    ImGui::PopItemFlag();
    return ImGui::IsItemDeactivatedAfterEdit() || //
           (dirty && std::chrono::system_clock::now() - prevEditTime > duration);
}

void ImGuiUtil::DebounceInput::Clear()
{
    dirty = true;
    filter.Clear();
}
} // namespace SosGui
