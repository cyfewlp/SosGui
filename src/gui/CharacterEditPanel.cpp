//
// Created by jamie on 2025/5/25.
//

#include "gui/CharacterEditPanel.h"

#include "data/SosUiData.h"
#include "gui/icon.h"
#include "gui/widgets.h"
#include "i18n/translator_manager.h"
#include "imgui.h"
#include "imguiex/imguiex_enum_wrap.h"
#include "service/SosDataCoordinator.h"
#include "task.h"
#include "util/utils.h"

namespace SosGui
{
namespace
{
constexpr auto POLICIES_DRAW_LIST = {
    AutoSwitch::Combat,
    AutoSwitch::World,
    AutoSwitch::WorldSnowy,
    AutoSwitch::WorldRainy,
    AutoSwitch::City,
    AutoSwitch::CitySnowy,
    AutoSwitch::CityRainy,
    AutoSwitch::Town,
    AutoSwitch::TownSnowy,
    AutoSwitch::TownRainy,
    AutoSwitch::Dungeon,
    AutoSwitch::DungeonSnowy,
    AutoSwitch::DungeonRainy
};

auto GetActorOutfitName(SosUiData &uiData, RE::Actor *actor) -> std::string
{
    const auto &actorOutfits = uiData.GetActorOutfitContainer();
    const auto &outfits      = uiData.GetOutfitContainer();
    if (const auto it = actorOutfits.find(actor->formID); it != actorOutfits.end())
    {
        if (const auto outfitIt = outfits.find(it->outfit_id); outfitIt != outfits.end())
        {
            return outfitIt->GetName();
        }
    }

    return "[No Outfit]";
}
} // namespace

void CharacterEditPanel::Focus()
{
    ImGui::SetWindowFocus("$SosGui_CharacterEditPanel"_T.c_str());
    BaseGui::Focus();
}

void CharacterEditPanel::Draw(SosUiData &uiData, const SosDataCoordinator &dataCoordinator, const OutfitService &outfitService)
{
    if (!IsShowing())
    {
        return;
    }
    ImGui::SetNextWindowPos(ImVec2(DEFAULT_MAIN_WINDOW_POS_X, DEFAULT_WINDOW_POS_Y), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("$SosGui_CharacterEditPanel"_T.c_str(), &m_show, ImGuiWindowFlags_NoNav))
    {
        DrawCharactersPanel(uiData, dataCoordinator, outfitService);
    }
    ImGui::End();
}

void CharacterEditPanel::DrawCharactersPanel(SosUiData &uiData, const SosDataCoordinator &dataCoordinator, const OutfitService &outfitService)
{
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    if (ImGuiUtil::IconButton(ICON_REFRESH_CW))
    {
        spawn([&] { return dataCoordinator.RequestActorList(); });
    }
    ImGui::PopStyleColor();
    ImGui::SetItemTooltip("%s", Translate1("Panels.Characters.Refresh"));

    ImGui::SameLine(0, 20);

    ImGuiUtil::Text(Translate("Add"));
    ImGui::SameLine();

    if (RE::Actor *selectedActor = nullptr; widgets::DrawNearActorsCombo(uiData.GetNearActors(), &selectedActor, RE::PlayerCharacter::GetSingleton()))
    {
        spawn([&] { return dataCoordinator.RequestAddActor(selectedActor); });
    }
    DrawCharactersInfo(uiData, dataCoordinator, outfitService);
}

void CharacterEditPanel::DrawCharactersInfo(SosUiData &uiData, const SosDataCoordinator &dataCoordinator, const OutfitService &outfitService)
{
    const auto &actors = uiData.GetActors();

    for (int index{0}; const auto &actor : actors)
    {
        ImGui::PushID(index++);
        if (ImGui::CollapsingHeader(actor->GetName()))
        {
            DrawOutfitsCombo(uiData, outfitService, actor);
            if (ImGui::Button(Translate1("Delete")))
            {
                spawn([&] { return dataCoordinator.RequestRemoveActor(actor); });
            }
            if (ImGui::TreeNode(Translate1("Panels.Characters.AutoSwitch.Title")))
            {
                draw_auto_switch(actor, uiData, dataCoordinator, outfitService);
                ImGui::TreePop();
            }
        }
        ImGui::PopID();
    }
}

void CharacterEditPanel::DrawOutfitsCombo(SosUiData &uiData, const OutfitService &outfitService, RE::Actor *actor)
{
    const auto activeOutfit = GetActorOutfitName(uiData, actor);
    if (ImGui::BeginCombo(Translate1("Panels.Characters.ActiveOutfit"), activeOutfit.c_str()))
    {
        const auto      &outfits = uiData.GetOutfitContainer().get_all();
        ImGuiListClipper clipper;
        clipper.Begin(static_cast<int>(outfits.size()));
        while (clipper.Step())
        {
            const auto start = static_cast<size_t>(clipper.DisplayStart);
            const auto end   = static_cast<size_t>(clipper.DisplayEnd);
            for (size_t index = start; index < end; ++index)
            {
                const auto &outfit = outfits[index];
                ImGui::PushID(static_cast<int>(index));
                if (ImGui::Selectable(outfit.GetName().c_str(), false))
                {
                    spawn([&] { return outfitService.SetActorOutfit(actor, outfit.GetId(), outfit.GetName()); });
                    util::RefreshActorArmor(actor);
                }
                ImGui::PopID();
            }
        }
        ImGui::EndCombo();
    }
}

std::string_view CharacterEditPanel::get_outfit_display_name(const RE::Actor *currentActor, AutoSwitch policy, SosUiData &uiData)
{
    const auto      &actor_outfit_container = uiData.GetActorOutfitContainer();
    std::string_view name                   = Translate("Panels.Characters.AutoSwitch.Empty");

    if (const auto auto_switch_outfit_opt = actor_outfit_container.find_auto_switch_outfit(currentActor->formID, policy); auto_switch_outfit_opt)
    {
        const auto &outfitList = uiData.GetOutfitContainer();
        if (const auto outfitIt = outfitList.find(auto_switch_outfit_opt.value()->outfit_id); outfitIt != outfitList.end())
        {
            name = outfitIt->GetName();
        }
    }
    return name;
}

void CharacterEditPanel::draw_auto_switch(
    const RE::Actor *currentActor, SosUiData &uiData, const SosDataCoordinator &dataCoordinator, const OutfitService &outfitService
)
{
    bool enabled = uiData.IsAutoSwitchEnabled(currentActor->GetFormID());
    if (ImGui::Checkbox(Translate1("Panels.Characters.AutoSwitch.Enable"), &enabled))
    {
        spawn([&] { return dataCoordinator.RequestSetActorAutoSwitchState(currentActor, enabled); });
    }

    AutoSwitch oldSelectedPolicy = selected_policy_;
    if (ImGui::BeginTable("##AutoSwitchStateList", 2, ImGuiEx::TableFlags().Resizable().SizingStretchProp().RowBg().Borders()))
    {
        ImGui::TableSetupColumn(Translate1("Panels.Characters.AutoSwitch.Location"));
        ImGui::TableSetupColumn(Translate1("Panels.Characters.AutoSwitch.State"));
        ImGui::TableHeadersRow();
        for (AutoSwitch policy : POLICIES_DRAW_LIST)
        {
            int policyId = static_cast<int>(policy);

            ImGui::PushID(policyId);
            ImGui::TableNextRow();
            if (ImGui::TableNextColumn())
            {
                const auto     key        = std::format("Panels.Characters.AutoSwitch.Policy{}", policyId);
                const bool     isSelected = selected_policy_ == policy;
                constexpr auto flags      = ImGuiEx::SelectableFlags().AllowOverlap().SpanAllColumns();
                if (ImGui::Selectable(Translate1(key), isSelected, flags))
                {
                    selected_policy_           = policy;
                    outfit_popup_target_actor_ = currentActor;
                }
                if (isSelected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }

            if (ImGui::TableNextColumn())
            {
                const auto outfitName = get_outfit_display_name(currentActor, policy, uiData);
                ImGuiUtil::Text(outfitName);
            }
            ImGui::PopID();
        }
        ImGui::EndTable();
    }

    if (oldSelectedPolicy != selected_policy_)
    {
        ImGui::OpenPopup("Outfit List");
    }

    if (ImGui::BeginPopup("Outfit List"))
    {
        ImGui::AlignTextToFramePadding();
        ImGuiUtil::IconButton(ICON_SEARCH);
        ImGui::SameLine(0.F, 0.F);
        debounce_input_.Draw("##filter", "filter outfit");

        ImGuiListClipper clipper;
        const auto      &outfits = uiData.GetOutfitContainer().get_all();
        clipper.Begin(static_cast<int>(outfits.size()));
        while (clipper.Step())
        {
            const auto start = static_cast<size_t>(clipper.DisplayStart);
            const auto end   = static_cast<size_t>(clipper.DisplayEnd);
            for (size_t index = start; index < end; ++index)
            {
                ImGui::PushID(static_cast<int>(index));
                const auto &outfit = outfits[index];
                if (ImGui::Selectable(outfit.GetName().c_str(), false))
                {
                    spawn([&] { return outfitService.SetActorStateOutfit(outfit_popup_target_actor_, selected_policy_, outfit.GetId()); });
                    ImGui::CloseCurrentPopup();
                }
                ImGui::PopID();
            }
        }
        ImGui::EndPopup();
    }
}
} // namespace SosGui
