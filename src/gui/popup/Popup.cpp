#include "gui/popup/Popup.h"

#include "Translation.h"
#include "i18n/translator_manager.h"
#include "imgui.h"
#include "imguiex/ErrorNotifier.h"
#include "path_utils.h"
#include "util/ImGuiUtil.h"
#include "util/utils.h"

#include <ranges>
#include <string>

namespace SosGui
{
namespace
{

void ConfirmAndClose(bool &confirmed)
{
    confirmed = true;
    ImGui::CloseCurrentPopup();
}
} // namespace

auto Popup::DrawActionButtons() -> bool
{
    bool        confirm      = false;
    const float contentWidth = ImGui::GetContentRegionAvail().x;
    const auto  quarterWidth = contentWidth * 0.25F;
    ImGui::SetCursorPosX(quarterWidth * 0.5F);
    if (ImGui::Button(Translate1("Yes"), ImVec2(quarterWidth, 0.0F)))
    {
        confirm = true;
        ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();

    ImGui::SetCursorPosX(quarterWidth * 2.5F);
    if (ImGui::Button(Translate1("No"), ImVec2(quarterWidth, 0.0F)))
    {
        ImGui::CloseCurrentPopup();
    }
    return confirm;
}

void Popup::DrawAboutPopup(std::string_view name)
{
    if (ImGui::BeginPopupModal(name.data()))
    {
        ImGuiUtil::Text(name);

        const auto *plugin  = SKSE::PluginDeclaration::GetSingleton();
        const auto &version = plugin->GetVersion();
        ImGuiUtil::Text(std::format("{} v{}.{}.{}", plugin->GetName(), version.major(), version.minor(), version.patch()));

        ImGui::Spacing();
        ImGuiUtil::Text(std::format("{:10} {}", "Build Date", __DATE__));
        ImGuiUtil::Text(std::format("{:10} {}", "Author Date", plugin->GetAuthor()));
        ImGuiUtil::Text(std::format("{:10}", "Github"));
        ImGui::SameLine();
        ImGui::TextLinkOpenURL("https://github.com/cyfewlp/JamieMods");
        ImGuiUtil::Text(std::format("{:10}", "Nexus Mods"));
        ImGui::SameLine();
        ImGui::TextLinkOpenURL("https://next.nexusmods.com/profile/JamieYin101");
        ImGui::EndPopup();
    }
}
} // namespace SosGui
