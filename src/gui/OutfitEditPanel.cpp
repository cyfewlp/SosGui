#include "gui/OutfitEditPanel.h"

#include "SosDataType.h"
#include "Translation.h"
#include "data/ArmorSource.h"
#include "data/SosUiData.h"
#include "data/SosUiOutfit.h"
#include "gui/UiSettings.h"
#include "gui/icon.h"
#include "gui/popup/Popup.h"
#include "gui/widgets.h"
#include "i18n/translator_manager.h"
#include "imgui.h"
#include "imguiex/ImGuiEx.h"
#include "imguiex/imguiex_enum_wrap.h"
#include "util/ImGuiUtil.h"
#include "util/utils.h"

#include <RE/B/BGSBipedObjectForm.h>
#include <RE/B/BipedObjects.h>
#include <RE/P/PlayerCharacter.h>
#include <RE/T/TESForm.h>
#include <RE/T/TESObjectARMO.h>
#include <array>
#include <cassert>
#include <expected>
#include <format>
#include <memory>
#include <ranges>
#include <string>

namespace SosGui
{
namespace
{
constexpr int SOS_SLOT_OFFSET = 30;

auto get_slot_name_key(const uint32_t slotPos) -> std::string
{
    return std::format("Panels.OutfitEdit.Slot{}", slotPos + SOS_SLOT_OFFSET);
}

void DrawSlotPolicyHelpPopup(const char *name)
{
    if (ImGui::BeginPopupModal(name))
    {
        ImGui::TextWrapped("%s", Translate1("Panels.OutfitEdit.SlotPolicyHelpText1"));
        ImGui::TextWrapped("%s", Translate1("Panels.OutfitEdit.SlotPolicyHelpText2"));
        ImGui::TextWrapped("%s", Translate1("Panels.OutfitEdit.SlotPolicyHelpText3"));
        ImGui::EndPopup();
    }
}

auto is_inventory_has_armor(RE::TESObjectREFR *objectRef) -> bool
{
    bool result = false;
    if (auto invChanges = objectRef->GetInventoryChanges(); invChanges && invChanges->entryList)
    {
        for (auto &entry : *invChanges->entryList)
        {
            if (entry && entry->object && entry->object->As<Armor>() != nullptr)
            {
                result = true;
                break;
            }
        }
    }

    if (auto container = objectRef->GetContainer(); container)
    {
        container->ForEachContainerObject([&](RE::ContainerObject &a_entry) {
            auto obj = a_entry.obj;
            if (obj && obj->As<Armor>() != nullptr)
            {
                result = true;
                return RE::BSContainer::ForEachResult::kStop;
            }
            return RE::BSContainer::ForEachResult::kContinue;
        });
    }

    return result;
}

auto get_near_objects_has_armor() -> std::vector<RE::TESObjectREFR *>
{
    const RE::PlayerCharacter *player = RE::PlayerCharacter::GetSingleton();
    const RE::TESObjectCELL   *cell   = player->GetParentCell();

    std::vector<RE::TESObjectREFR *> objectRefs;
    cell->ForEachReference([&](RE::TESObjectREFR *objectRef) {
        if (const std::string_view nameSv(objectRef->GetName()); nameSv.empty())
        {
            return RE::BSContainer::ForEachResult::kContinue;
        }
        if (is_inventory_has_armor(objectRef))
        {
            objectRefs.push_back(objectRef);
        }
        return RE::BSContainer::ForEachResult::kContinue;
    });
    return objectRefs;
}

} // namespace

void OutfitEditPanel::OnRefresh()
{
    should_refresh_view_ = true;
    m_armorView.on_refresh();
}

void OutfitEditPanel::Cleanup()
{
    should_refresh_view_ = true;
    m_armorView.clear();
}

void OutfitEditPanel::Focus()
{
    ImGui::SetWindowFocus(m_windowTitle.c_str());
    BaseGui::Focus();
}

void OutfitEditPanel::OnSelectOutfit(const EditingOutfit &lastEdit, const EditingOutfit &editing)
{
    UpdateWindowTitle(editing.GetName());
    // m_armorView.multi_selection_.Clear();
}

void OutfitEditPanel::PushError(const Error error)
{
    switch (error)
    {
        case Error::delete_armor_from_unknown_outfit_id:
            ErrorNotifier::GetInstance().Error("Can't delete a invalid or has been removed outfit.");
            break;
        case Error::add_armor_to_unknown_outfit_id:
            ErrorNotifier::GetInstance().Error("Can't add armor to a invalid or has been removed outfit.");
            break;
    }
}

void OutfitEditPanel::Draw(const EditingOutfit &editingOutfit)
{
    if (!IsShowing())
    {
        return;
    }
    ImGui::SetNextWindowPos(ImVec2(DEFAULT_OUTFIT_EDIT_WINDOW_POS_X, DEFAULT_WINDOW_POS_Y), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize({DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT}, ImGuiCond_FirstUseEver);
    if (ImGui::Begin(m_windowTitle.c_str(), &m_show))
    {
        if (should_refresh_view_)
        {
            should_refresh_view_ = false;
            m_armorView.reset_view(armor_source_, armor_source_refr_);
        }
        if (ImGui::BeginChild("##OutfitEditPanelSideBar", {200, 0}, ImGuiEx::ChildFlags().Borders().ResizeX()))
        {
            DrawSideBar();
        }
        ImGui::EndChild();

        ImGui::SameLine();
        ImGui::BeginGroup();
        DrawOutfitPanel(editingOutfit);
        DrawArmorSourcesTabBar();
        DrawArmorViewFilter();
        DrawArmorView(editingOutfit);
        ImGui::EndGroup();
    }
    ImGui::End();
}

////////////////////////////////////////////////////////////////////////////////////////////////
// Outfit Panel
////////////////////////////////////////////////////////////////////////////////////////////////

void OutfitEditPanel::DrawOutfitPanel(const EditingOutfit &editingOutfit)
{
    ImGui::TextUnformatted(editingOutfit.GetName().c_str());
    if (ImGui::BeginTabBar("##OutfitTabBarView"))
    {
        if (ImGui::BeginTabItem(Translate1("Armor"), nullptr, ImGuiTabItemFlags_Leading))
        {
            DrawOutfitArmors(editingOutfit);
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}

void OutfitEditPanel::DrawSideBar()
{
    ImGui::TextUnformatted(Translate1("Panels.Outfit.ModList"));
    constexpr int maxChildItemCount = 10;
    const auto    itemHeight        = ImGui::GetTextLineHeight();
    float         childHeight       = (itemHeight + ImGui::GetStyle().ItemInnerSpacing.y) * maxChildItemCount;

    if (ImGui::BeginChild("##ModNameListChild", {0, childHeight}, ImGuiEx::ChildFlags().Borders().ResizeY()))
    {
        DrawArmorViewModNameFilterer();
    }
    ImGui::EndChild();

    ImGuiUtil::Text(Translate1("Panels.Outfit.BodySlots"));
    if (ImGui::BeginChild("#SlotFilterChild", {0, childHeight}, ImGuiEx::ChildFlags().Borders().ResizeY()))
    {
        DrawArmorViewSlotFilterer();
    }
    ImGui::EndChild();
    bool needUpdateView = ImGui::Checkbox("Contain non playable", &m_armorView.contain_non_playable_armor_);
    needUpdateView      = ImGui::Checkbox("Contain template armors", &m_armorView.contain_template_armor_) || needUpdateView;
    if (needUpdateView)
    {
        m_armorView.reset_view_data(armor_source_, armor_source_refr_);
    }
}

void OutfitEditPanel::UpdateWindowTitle(const std::string &outfitName)
{
    m_windowTitle =
        std::format("{} {} - {}###OutfitEditPanel", Translate("Panels.OutfitEdit.EditingHint"), outfitName, Translate("Panels.OutfitEdit.Title"));
}

void OutfitEditPanel::DrawOutfitArmors(const EditingOutfit &editingOutfit)
{
    constexpr const char *SLOT_POLICY_HELP_POPUP_TITLE = "What is Slot Policy?";

    if (editingOutfit.IsUntitled())
    {
        ImGuiUtil::Text(Translate("Panels.OutfitEdit.MissingOutfitHint"));
        return;
    }
    if (editingOutfit.IsEmpty())
    {
        ImGuiUtil::Text(Translate("Panels.OutfitEdit.EmptyHint"));
        return;
    }

    ImGui::Checkbox(Translate1("Panels.OutfitEdit.ShowAllSlots"), &show_all_outfit_slots_);
    ImGui::SameLine();
    if (ImGui::Button(Translate1("Panels.OutfitEdit.SlotPolicies")))
    {
        ImGui::OpenPopup(SLOT_POLICY_HELP_POPUP_TITLE);
    }
    DrawSlotPolicyHelpPopup(SLOT_POLICY_HELP_POPUP_TITLE);

    static uint32_t selectedIdx = RE::BIPED_OBJECT::kEditorTotal;
    if (ImGui::BeginTable("##OutfitArmors", 5, ImGuiEx::TableFlags().Resizable().SizingStretchProp()))
    {
        ImGui::TableSetupColumn("##Number", ImGuiEx::TableColumnFlags().NoHide());
        ImGui::TableSetupColumn(Translate1("Panels.OutfitEdit.Slot"), ImGuiEx::TableColumnFlags().NoSort());
        ImGui::TableSetupColumn(Translate1("Armor"), ImGuiEx::TableColumnFlags().NoSort());
        ImGui::TableSetupColumn(Translate1("Panels.OutfitEdit.SlotPolicies"), ImGuiEx::TableColumnFlags().WidthFixed().NoSort());
        ImGui::TableSetupColumn(Translate1("Delete"), ImGuiEx::TableColumnFlags().WidthFixed().NoSort());
        ImGui::TableHeadersRow();

        for (SlotType slotIdx = 0; slotIdx < static_cast<SlotType>(RE::BIPED_OBJECT::kEditorTotal); ++slotIdx)
        {
            const auto *armor = editingOutfit.GetArmorAt(slotIdx);
            if (!show_all_outfit_slots_ && armor == nullptr)
            {
                continue;
            }

            ImGui::PushID(static_cast<int>(slotIdx));

            ImGui::TableNextRow();
            ImGui::TableNextColumn(); // number column
            ImGui::Text("%d", slotIdx + 1);

            if (ImGui::TableNextColumn()) // slot name column
            {
                const bool isSelected = selectedIdx == slotIdx;
                if (ImGui::Selectable(Translate1(get_slot_name_key(slotIdx)), isSelected, ImGuiEx::SelectableFlags().AllowOverlap().SpanAllColumns()))
                {
                    selectedIdx = isSelected ? RE::BIPED_OBJECT::kEditorTotal : slotIdx;
                }
            }
            if (armor != nullptr)
            {
                HighlightConflictSlot(util::ToSlot(slotIdx));
            }

            ImGui::TableNextColumn(); // armor name column
            ImGui::Text("%s", armor == nullptr ? "" : armor->GetName());

            ImGui::TableNextColumn(); // slot policy combo column
            SlotPolicyCombo(editingOutfit, slotIdx);

            ImGui::TableNextColumn(); // action column
            {
                ImGui::BeginDisabled(armor == nullptr);
                if (ImGui::Button(Translate1("Delete")))
                {
                    DeleteArmor(editingOutfit.GetId(), armor);
                }
                ImGui::EndDisabled();
            }
            ImGui::PopID();
        }
        ImGui::EndTable();
    }
}

void OutfitEditPanel::HighlightConflictSlot(const Slot slot) const
{
    if (m_armorView.multi_selection_.is_select_slot(slot))
    {
        auto *drawList = ImGui::GetWindowDrawList();
        drawList->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), ImColor(255, 0, 0, 255), 0, ImDrawFlags_None, 2.0F);
    }
}

void OutfitEditPanel::SlotPolicyCombo(const EditingOutfit &editingOutfit, const uint32_t &slotIdx) const
{
    auto       &policyNameKey = editingOutfit.GetSlotPolicies().at(slotIdx);
    std::string policyName;

    if (ImGui::BeginCombo("##SlotPolicy", Translation::Translate(policyNameKey).c_str(), ImGuiComboFlags_WidthFitPreview))
    {
        for (const auto &policy : {SlotPolicy::Inherit, SlotPolicy::Passthrough, SlotPolicy::RequireEquipped, SlotPolicy::AlwaysUseOutfit})
        {
            policyName = SlotPolicyToUiString(policy);
            if (ImGui::Selectable(policyName.c_str(), false))
            {
                if (!editingOutfit.IsUntitled())
                {
                    +[&] {
                        return m_outfitService.SetSlotPolicy(editingOutfit.GetId(), editingOutfit.GetName(), slotIdx, policy);
                    };
                }
            }
            ImGuiUtil::SetItemTooltip(SlotPolicyToTooltipString(policy));
        }
        ImGui::EndCombo();
    }
}

void OutfitEditPanel::DrawArmorSourcesTabBar()
{
    ImGui::Separator();
    if (ImGui::BeginTabBar("ArmorSourceTabBar", ImGuiEx::TabBarFlags().DrawSelectedOverline().Reorderable()))
    {
        auto      *player         = RE::PlayerCharacter::GetSingleton();
        const auto oldArmorSource = armor_source_;
        if (ImGui::BeginTabItem(Translate1("Panels.OutfitEdit.ArmorSource.NearObjectInventory")))
        {
            armor_source_ = ArmorSource::Inventory;
            if (oldArmorSource != armor_source_)
            {
                near_objects_      = get_near_objects_has_armor();
                armor_source_refr_ = near_objects_.empty() ? player : near_objects_[0];
            }

            if (ImGui::BeginCombo("##NearObjects", armor_source_refr_->GetDisplayFullName(), ImGuiEx::ComboFlags().WidthFitPreview().HeightRegular()))
            {
                for (const auto &object : near_objects_)
                {
                    ImGui::PushID(static_cast<int>(object->formID));
                    if (ImGui::Selectable(object->GetName(), armor_source_refr_->GetFormID() == object->GetFormID()))
                    {
                        armor_source_refr_ = object;
                        m_armorView.reset_view(armor_source_, armor_source_refr_);
                    }
                    ImGui::PopID();
                }
                ImGui::EndCombo();
            }
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem(Translate1("Panels.OutfitEdit.ArmorSource.NearNpcCarried")))
        {
            armor_source_ = ArmorSource::Carried;
            if (oldArmorSource != armor_source_)
            {
                near_objects_      = get_near_objects_has_armor();
                armor_source_refr_ = player;
            }

            if (ImGui::BeginCombo("##NearActors", armor_source_refr_->GetDisplayFullName(), ImGuiEx::ComboFlags().WidthFitPreview().HeightRegular()))
            {
                for (const auto &object : near_objects_)
                {
                    if (!object->IsHumanoid()) continue;
                    ImGui::PushID(static_cast<int>(object->formID));
                    if (ImGui::Selectable(object->GetName(), armor_source_refr_->GetFormID() == object->GetFormID()))
                    {
                        armor_source_refr_ = object;
                        m_armorView.reset_view(armor_source_, armor_source_refr_);
                    }
                    ImGui::PopID();
                }
                ImGui::EndCombo();
            }
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem(Translate1("Panels.OutfitEdit.ArmorSource.ByFormID")))
        {
            armor_source_ = ArmorSource::Armor;
            if (oldArmorSource != armor_source_)
            {
                armor_source_refr_ = nullptr;
            }
            ImGui::Text("0x");
            ImGui::SameLine();
            static std::array<char, 9> formIdBuf;

            if (ImGui::InputText("##FormIdInput", formIdBuf.data(), formIdBuf.size(), ImGuiEx::InputTextFlags().CharsUppercase().CharsHexadecimal()))
            {
                m_armorView.clear_view_data();
                char          *pEnd{};
                const uint32_t formId = strtoul(formIdBuf.data(), &pEnd, 16);
                if (pEnd != formIdBuf.data())
                {
                    armor_source_refr_ = RE::TESForm::LookupByID<RE::TESObjectREFR>(formId);
                    m_armorView.reset_view(armor_source_, armor_source_refr_);
                }
            }
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem(Translate1("Panels.OutfitEdit.ArmorSource.AllArmors")))
        {
            armor_source_ = ArmorSource::All;
            if (oldArmorSource != armor_source_)
            {
                armor_source_refr_ = nullptr;
            }
            ImGui::EndTabItem();
        }

        if (oldArmorSource != armor_source_)
        {
            m_armorView.reset_view(armor_source_, armor_source_refr_);
        }

        ImGui::EndTabBar();
    }
}

void OutfitEditPanel::DrawArmorViewFilter()
{
    if (m_armorView.armor_name_filter_.Draw())
    {
        m_armorView.armor_name_filter_.dirty = false;
        m_armorView.reset_view_data(armor_source_, armor_source_refr_);
    }
}

void OutfitEditPanel::DrawArmorView(const EditingOutfit &editingOutfit)
{
    if (m_armorView.view_data_.empty())
    {
        return;
    }
    constexpr auto flags =
        ImGuiEx::TableFlags().RowBg().BordersInnerH().ScrollY().Resizable().SizingStretchProp().Sortable().Hideable().Reorderable();
    const auto styleGuard = ImGuiEx::StyleGuard().Style<ImGuiStyleVar_FramePadding>({3.0F, 3.0F});
    if (ImGui::BeginTable("##ArmorCandidates", 6, flags))
    {
        DrawArmorViewContent(editingOutfit, m_armorView.view_data_);
        ImGui::EndTable();
    }
}

void OutfitEditPanel::DrawArmorViewContent(const EditingOutfit &editingOutfit, const std::vector<ArmorEntry> &viewData)
{
    ImGui::TableSetupScrollFreeze(1, 1);
    ImGui::TableSetupColumn("##Number", ImGuiEx::TableColumnFlags().NoSort().WidthFixed());
    ImGui::TableSetupColumn(Translate1("Armor"), ImGuiEx::TableColumnFlags().DefaultSort().NoHide());
    ImGui::TableSetupColumn("FormID", ImGuiEx::TableColumnFlags().DefaultHide().NoSort());
    ImGui::TableSetupColumn(Translate1("Panels.OutfitEdit.ModName"), ImGuiEx::TableColumnFlags().DefaultHide().NoSort());
    ImGui::TableSetupColumn(Translate1("Panels.OutfitEdit.Playable"), ImGuiEx::TableColumnFlags().DefaultHide().NoSort());
    ImGui::TableSetupColumn(Translate1("##Add"), ImGuiEx::TableColumnFlags().NoSort());
    ImGui::TableHeadersRow();

    bool wantAddArmor   = false;
    auto drawArmorEntry = [&](const Armor *armor, const ImGuiID index) {
        ImGui::PushID(static_cast<int>(index));
        ImGui::TableNextRow();
        if (ImGui::TableNextColumn()) // number column
        {
            auto      &multiSelection = m_armorView.multi_selection_;
            const bool isSelected     = multiSelection.update_selected(armor, index);
            ImGui::SetNextItemSelectionUserData(index);
            ImGui::Selectable(std::format("{}", index + 1).c_str(), isSelected, ImGuiEx::SelectableFlags().AllowOverlap().SpanAllColumns());

            if (ImGui::BeginPopupContextItem("Context"))
            {
                ImGui::BeginDisabled(editingOutfit.IsUntitled() || multiSelection.Size <= 0);
                if (ImGui::MenuItem(Translate1("Panels.OutfitEdit.AddAll")))
                {
                    wantAddArmor = true;
                }
                ImGui::EndDisabled();
                ImGui::EndPopup();
            }
        }

        if (ImGui::TableNextColumn()) // armor name column
        {
            ImGuiUtil::Text(armor->GetName());
        }

        if (ImGui::TableNextColumn()) // formId column
        {
            ImGuiUtil::Text(std::format("{:#010X}", armor->GetFormID()));
        }

        if (ImGui::TableNextColumn()) // mod name column
        {
            ImGuiUtil::Text(util::GetFormModFileName(armor));
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip))
            {
                ImGui::SetItemTooltip("%s", util::GetFormModFileName(armor).data());
            }
        }

        if (ImGui::TableNextColumn()) // column playable
        {
            ImGuiUtil::IconButton(IsArmorNonPlayable(armor) ? std::string_view(ICON_X) : ICON_CHECK);
        }

        ImGui::BeginDisabled(editingOutfit.IsUntitled());
        if (ImGui::TableNextColumn() && ImGui::Button(Translate1("Add"))) // column Action
        {
            m_armorView.multi_selection_.Clear();
            m_armorView.multi_selection_.SetItemSelected(index, true);
            wantAddArmor = true;
        }
        ImGui::EndDisabled();
        ImGui::PopID();
    };

    DrawArmorViewTableContent(viewData, drawArmorEntry);

    constexpr const char *Add_Armors_Progress_Popup_Title = "Add Armors";
    if (wantAddArmor)
    {
        waiting_add_armor_count_ = m_armorView.multi_selection_.Size;
        conflict_solution_       = ConflictSolution::none;
        ImGui::OpenPopup(Add_Armors_Progress_Popup_Title);
    }

    bool open = true;
    if (ImGui::BeginPopupModal(Add_Armors_Progress_Popup_Title, &open))
    {
        const int   remaining = waiting_add_armor_count_ - m_armorView.multi_selection_.Size;
        const float fraction  = static_cast<float>(remaining) / static_cast<float>(waiting_add_armor_count_);
        ImGui::ProgressBar(fraction);

        void   *it     = nullptr;
        ImGuiID nextId = 0;
        if (m_armorView.multi_selection_.GetNextSelectedItem(&it, &nextId))
        {
            const auto &rankedArmor = m_armorView.view_data_.at(nextId);
            ImGuiUtil::Text(std::format("Add armor {}...", rankedArmor->GetName()));
        }

        if (conflict_solution_ == ConflictSolution::Suspend)
        {
            ImGuiUtil::Text("Conflict with exist armors, do you want to:");
            if (ImGui::Button("Skip"))
            {
                conflict_solution_ = ConflictSolution::Skip;
            }
            ImGui::SameLine();
            if (ImGui::Button("Skip All"))
            {
                conflict_solution_ = ConflictSolution::SkipAll;
            }
            ImGui::Separator();
            if (ImGui::Button("Overwrite"))
            {
                conflict_solution_ = ConflictSolution::Overwrite;
            }
            ImGui::SameLine();
            if (ImGui::Button("Overwrite All"))
            {
                conflict_solution_ = ConflictSolution::OverwriteAll;
            }
        }
        AddSelectArmors(editingOutfit.GetId());
        if (m_armorView.multi_selection_.Size == 0)
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void OutfitEditPanel::DrawArmorViewTableContent(const std::vector<ArmorEntry> &viewData, const DrawArmorEntry &drawArmorEntry)
{
    static bool ascend = true;
    ImGuiUtil::may_update_table_sort_dir(ascend);
    auto &multiSelection = m_armorView.multi_selection_;
    auto *msIO           = multiSelection.Begin(
        ImGuiEx::MultiSelectFlags().NoSelectAll().BoxSelect1d().ClearOnEscape().ClearOnClickVoid(), static_cast<int>(viewData.size())
    );
    multiSelection.ApplyRequests(msIO);
    ImGuiListClipper clipper;
    clipper.Begin(static_cast<int>(viewData.size()));
    if (msIO->RangeSrcItem != -1)
    {
        clipper.IncludeItemByIndex(static_cast<int>(msIO->RangeSrcItem));
    }

    while (clipper.Step())
    {
        const int itemCount = clipper.DisplayEnd - clipper.DisplayStart;
        auto      index     = static_cast<ImGuiID>(clipper.DisplayStart);
        if (ascend)
        {
            for (const auto &armor : viewData | std::views::drop(clipper.DisplayStart) | std::views::take(itemCount))
            {
                drawArmorEntry(armor, index++);
            }
        }
        else
        {
            for (const auto &armor : viewData | std::views::reverse | std::views::drop(clipper.DisplayStart) | std::views::take(itemCount))
            {
                drawArmorEntry(armor, index++);
            }
        }
    }
    multiSelection.ApplyRequests(ImGui::EndMultiSelect());
}

void OutfitEditPanel::DrawArmorViewModNameFilterer()
{
    bool needResetView = false;
    for (auto &modEntry : m_armorView.mod_filterer_.mod_entry_map | std::views::values)
    {
        auto label = std::format("({}) {}", modEntry.count, modEntry.name);

        if (ImGui::Checkbox(label.c_str(), &modEntry.checked))
        {
            needResetView = true;
        }
    }
    if (needResetView)
    {
        m_armorView.reset_view_data(armor_source_, armor_source_refr_);
    }
}

void OutfitEditPanel::DrawArmorViewSlotFilterer()
{
    const std::string name = std::format("All({}/{})##AllSlot", m_armorView.view_data_.size(), m_armorView.armor_count);

    const auto &slotFilterer = m_armorView.slot_filterer_;
    bool        checkedAll   = slotFilterer.is_enable_all_slots();
    if (ImGui::Checkbox(name.c_str(), &checkedAll))
    {
        m_armorView.filterer_enable_all_slots(checkedAll, armor_source_, armor_source_refr_);
    }

    for (SlotType idx = 0; idx < RE::BIPED_OBJECT::kEditorTotal; ++idx)
    {
        ImGui::PushID(static_cast<int>(idx));

        auto slotLabel = std::format("({}) {}", m_armorView.get_armor_count(idx), Translate(get_slot_name_key(idx)));
        bool checked   = slotFilterer.is_slot_selected(idx);
        if (ImGui::Checkbox(slotLabel.c_str(), &checked))
        {
            m_armorView.filterer_select_slot(idx, checked, armor_source_, armor_source_refr_);
        }
        ImGui::PopID();
    }
}

void OutfitEditPanel::AddSelectArmors(const OutfitId id)
{
    if (m_armorView.multi_selection_.Size <= 0 || conflict_solution_ == ConflictSolution::Suspend)
    {
        return;
    }

    const auto outfitIt = m_uiData.GetOutfitContainer().find(id);
    if (outfitIt == m_uiData.GetOutfitContainer().end())
    {
        PushError(Error::add_armor_to_unknown_outfit_id);
        m_armorView.multi_selection_.Clear();
        return;
    }
    REX::EnumSet<Slot> usedSlot;
    void              *it     = nullptr;
    ImGuiID            nextId = 0;
    for (; m_armorView.multi_selection_.GetNextSelectedItem(&it, &nextId); m_armorView.multi_selection_.SetItemSelected(nextId, false))
    {
        const auto &armor = m_armorView.view_data_.at(nextId);
        if (usedSlot.all(armor->GetSlotMask().get()) || outfitIt->IsConflictWith(armor))
        {
            if (conflict_solution_ == ConflictSolution::none)
            {
                conflict_solution_ = ConflictSolution::Suspend;
                break;
            }
            if (conflict_solution_ == ConflictSolution::Skip)
            {
                conflict_solution_ = ConflictSolution::none;
                continue;
            }
            else if (conflict_solution_ == ConflictSolution::SkipAll)
            {
                continue;
            }

            // overwrite or overwrite-all
            if (conflict_solution_ == ConflictSolution::Overwrite)
            {
                conflict_solution_ = ConflictSolution::none;
            }

            +[&] {
                return m_outfitService.DeleteConflictArmors(outfitIt->GetName(), armor);
            };
        }

        usedSlot.set(armor->GetSlotMask().get());
        +[&] {
            return m_outfitService.AddArmor(outfitIt->GetId(), outfitIt->GetName(), armor);
        };
    }

    +[&] {
        return m_outfitService.GetOutfitArmors(outfitIt->GetId(), outfitIt->GetName());
    };
}

void OutfitEditPanel::DeleteArmor(const OutfitId id, const Armor *armor)
{
    if (const auto it = m_uiData.GetOutfitContainer().find(id); it != m_uiData.GetOutfitContainer().end())
    {
        +[&] {
            return m_outfitService.RemoveArmor(it->GetId(), it->GetName(), armor);
        };
    }
    else
    {
        PushError(Error::delete_armor_from_unknown_outfit_id);
    }
}
} // namespace SosGui
