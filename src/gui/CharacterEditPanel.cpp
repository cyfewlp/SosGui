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
#include "tracy/Tracy.hpp"
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

using EditingActor = ActorOutfitContainer::Entry;

auto get_actor_outfit_name(const EditingActor &editing_actor, const OutfitContainer &outfit_container) -> std::string
{
    if (const auto outfitIt = outfit_container.find(editing_actor.outfit_id); outfitIt != outfit_container.end())
    {
        return outfitIt->GetName();
    }

    return "[No Outfit]";
}

auto draw_outfits(const std::vector<SosUiOutfit> &outfits, const std::string_view selected_name, const ImGuiTextFilter &filter) -> const SosUiOutfit *
{
    const SosUiOutfit *selected_outfit = nullptr;

    for (const auto [index, outfit] : outfits | std::views::enumerate)
    {
        ImGui::PushID(static_cast<int>(index));
        const auto &name     = outfit.GetName();
        const auto  selected = selected_name == name;
        if (filter.PassFilter(name.c_str(), name.end()._Ptr) && ImGui::Selectable(name.c_str(), selected))
        {
            selected_outfit = &outfit;
        }
        if (selected)
        {
            ImGui::SetItemDefaultFocus();
        }
        ImGui::PopID();
    }
    return selected_outfit;
}

auto get_outfit_display_name(const EditingActor &editing_actor, AutoSwitch policy, const OutfitContainer &outfit_container) -> std::string_view
{
    std::string_view name = Translate("Panels.Characters.AutoSwitch.Empty");

    if (const auto auto_switch_outfit_opt = ActorOutfitContainer::find_auto_switch_outfit(editing_actor, policy); auto_switch_outfit_opt)
    {
        if (const auto outfitIt = outfit_container.find(auto_switch_outfit_opt.value()->outfit_id); outfitIt != outfit_container.end())
        {
            name = outfitIt->GetName();
        }
    }
    return name;
}
} // namespace

void CharacterEditPanel::Draw(SosUiData &uiData, const SosDataCoordinator &dataCoordinator, const OutfitService &outfitService)
{
    ZoneScopedN(__FUNCTION__);
    ImGui::SetNextWindowPos({200.F, 200.F}, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize({280.F, 300.F}, ImGuiCond_FirstUseEver);
    if (ImGui::Begin(Translate1("Panels.Characters.Title"), nullptr, ImGuiEx::WindowFlags()))
    {
        DrawCharactersPanel(uiData, dataCoordinator, outfitService);
    }
    ImGui::End();
}

void CharacterEditPanel::DrawCharactersPanel(SosUiData &uiData, const SosDataCoordinator &dataCoordinator, const OutfitService &outfitService)
{
    if (ImGui::BeginCombo("##NearObjects", Translate1("Panels.Characters.Add"), ImGuiEx::ComboFlags().WidthFitPreview().HeightRegular()))
    {
        for (const auto &actor : uiData.near_actors)
        {
            ImGui::PushID(static_cast<int>(actor->formID));
            if (ImGui::Selectable(actor->GetName(), false))
            {
                spawn([&] { return dataCoordinator.RequestAddActor(actor); });
            }
            ImGui::PopID();
        }
        ImGui::EndCombo();
    }
    DrawCharactersInfo(uiData, dataCoordinator, outfitService);
}

void CharacterEditPanel::DrawCharactersInfo(SosUiData &ui_data, const SosDataCoordinator &data_coordinator, const OutfitService &outfit_service)
{
    for (int index{0}; const auto &actor_outfit_entry : ui_data.actor_outfit_container.container)
    {
        ImGui::PushID(index++);
        auto *actor = actor_outfit_entry.actor;
        if (ImGui::CollapsingHeader(actor->GetName()))
        {
            const auto &outfit_container = ui_data.outfit_container;
            const auto  active_outfit    = get_actor_outfit_name(actor_outfit_entry, outfit_container);
            if (ImGui::BeginCombo(Translate1("Panels.Characters.ActiveOutfit"), active_outfit.c_str()))
            {
                outfit_name_filter_.Draw("##filter", -FLT_MIN);
                const SosUiOutfit *selected = draw_outfits(outfit_container.get_all(), active_outfit, outfit_name_filter_);
                if (selected != nullptr)
                {
                    spawn([&] { return outfit_service.SetActorOutfit(actor, selected->GetId(), selected->GetName()); });
                    util::RefreshActorArmor(actor);
                }
                ImGui::EndCombo();
            }
            if (ImGui::Button(Translate1("Delete")))
            {
                spawn([&] { return data_coordinator.RequestRemoveActor(actor); });
            }
            if (ImGui::TreeNode(Translate1("Panels.Characters.AutoSwitch.Title")))
            {
                draw_auto_switch(actor_outfit_entry, outfit_container, data_coordinator, outfit_service);
                ImGui::TreePop();
            }
        }
        ImGui::PopID();
    }
}

void CharacterEditPanel::draw_auto_switch(
    const EditingActor &editing_actor, const OutfitContainer &outfit_container, const SosDataCoordinator &data_coordinator,
    const OutfitService &outfit_service
)
{
    bool enabled = editing_actor.auto_switch_enabled;
    if (ImGui::Checkbox(Translate1("Panels.Characters.AutoSwitch.Enable"), &enabled))
    {
        spawn([&] { return data_coordinator.RequestSetActorAutoSwitchState(editing_actor.actor, enabled); });
    }

    if (ImGui::BeginTable("##AutoSwitchStateList", 2, ImGuiEx::TableFlags().Resizable().SizingStretchProp().RowBg().Borders()))
    {
        ImGui::TableSetupColumn(Translate1("Panels.Characters.AutoSwitch.Location"));
        ImGui::TableSetupColumn(Translate1("Panels.Characters.AutoSwitch.State"));
        ImGui::TableHeadersRow();
        for (const auto policy : POLICIES_DRAW_LIST)
        {
            int policyId = static_cast<int>(policy);

            ImGui::PushID(policyId);
            ImGui::TableNextRow();
            if (ImGui::TableNextColumn())
            {
                const auto key = std::format("Panels.Characters.AutoSwitch.Policy{}", policyId);
                ImGuiUtil::Text(Translate(key));
            }

            if (ImGui::TableNextColumn())
            {
                const auto outfitName = get_outfit_display_name(editing_actor, policy, outfit_container);
                if (ImGui::BeginCombo(Translate1("Panels.Characters.ActiveOutfit"), outfitName.data()))
                {
                    outfit_name_filter_.Draw("##filter", -FLT_MIN);
                    const SosUiOutfit *selected = draw_outfits(outfit_container.get_all(), outfitName, outfit_name_filter_);
                    if (selected != nullptr)
                    {
                        spawn([&] { return outfit_service.SetActorStateOutfit(editing_actor.actor, policy, selected->GetId()); });
                    }
                    ImGui::EndCombo();
                }
            }
            ImGui::PopID();
        }
        ImGui::EndTable();
    }
}
} // namespace SosGui
