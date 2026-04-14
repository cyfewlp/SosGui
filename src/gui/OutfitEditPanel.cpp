#define IMGUI_DEFINE_MATH_OPERATORS

#include "gui/OutfitEditPanel.h"

#include "SosDataType.h"
#include "Translation.h"
#include "data/ArmorSource.h"
#include "data/SosUiData.h"
#include "data/SosUiOutfit.h"
#include "gui/icon.h"
#include "gui/popup/Popup.h"
#include "gui/widgets.h"
#include "i18n/translator_manager.h"
#include "imgui.h"
#include "imguiex/ImGuiEx.h"
#include "imguiex/imguiex_enum_wrap.h"
#include "tracy/Tracy.hpp"
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
#include <imgui_internal.h>
#include <memory>
#include <ranges>
#include <string>

namespace SosGui
{
namespace
{
constexpr int         SOS_SLOT_OFFSET                 = 30;
constexpr const char *Add_Armors_Progress_Popup_Title = "Add Armors";

auto get_slot_name_key(const SlotType slotPos) -> std::string
{
    return std::format("Panels.OutfitEdit.Slot{}", slotPos + SOS_SLOT_OFFSET);
}

void DrawSlotPolicyHelpPopup(const char *name)
{
    bool open = true;
    if (ImGui::BeginPopupModal(name, &open))
    {
        ImGui::TextWrapped("%s", Translate1("Panels.OutfitEdit.SlotPolicy.HelpText1"));
        ImGui::TextWrapped("%s", Translate1("Panels.OutfitEdit.SlotPolicy.HelpText2"));
        ImGui::TextWrapped("%s", Translate1("Panels.OutfitEdit.SlotPolicy.HelpText3"));

        (void)Popup::DrawActionButtons();
        ImGui::EndPopup();
    }
}

auto is_inventory_has_armor(RE::TESObjectREFR *objectRef) -> bool
{
    bool result = false;
    if (auto *invChanges = objectRef->GetInventoryChanges(); invChanges != nullptr && (invChanges->entryList != nullptr))
    {
        for (auto *entry : *invChanges->entryList)
        {
            if ((entry != nullptr) && entry->object != nullptr && entry->object->As<Armor>() != nullptr)
            {
                result = true;
                break;
            }
        }
    }

    if (auto *container = objectRef->GetContainer(); container)
    {
        container->ForEachContainerObject([&](RE::ContainerObject &a_entry) {
            auto *obj = a_entry.obj;
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

void OutfitEditPanel::on_refresh()
{
    view_item_count_     = -1;
    should_refresh_view_ = true;
    armor_view_.clear_all();
}

void OutfitEditPanel::draw(const OutfitContainer &outfit_container)
{
    ZoneScopedN(__FUNCTION__);
    if (ImGui::Begin(window_title_.c_str()))
    {
        if (should_refresh_view_)
        {
            should_refresh_view_ = false;
            armor_view_.reset_view(armor_source_, armor_source_refr_);
            view_item_count_ = -1;
        }

        outfit_list_table_.Draw(outfit_container.get_all(), *outfit_service_);
        auto &editingOutfit   = outfit_list_table_.get_editing_outfit();
        editingOutfit.invalid = outfit_container.find(editingOutfit.GetId()) == outfit_container.end();

        ImGui::SameLine();
        draw_filterers(editingOutfit);

        UpdateWindowTitle(editingOutfit);

        ImGui::SameLine();
        ImGui::BeginGroup();
        draw_outfit(editingOutfit);
        DrawArmorSourcesTabBar();
        draw_armor_view(editingOutfit);
        ImGui::EndGroup();
    }
    ImGui::End();
}

////////////////////////////////////////////////////////////////////////////////////////////////
// Outfit Panel
////////////////////////////////////////////////////////////////////////////////////////////////

void OutfitEditPanel::draw_outfit(EditingOutfit &editingOutfit)
{
    ZoneScopedN(__FUNCTION__);
    if (ImGui::CollapsingHeader(editingOutfit.get_name().data(), ImGuiEx::TreeNodeFlags().DefaultOpen()))
    {
        draw_outfit_armors(editingOutfit);
    }
}

void OutfitEditPanel::draw_filterers(const EditingOutfit &editingOutfit)
{
    ZoneScopedN(__FUNCTION__);
    if (!ImGui::BeginChild("##sidebar", {200, 0}, ImGuiEx::ChildFlags().Borders().ResizeX()))
    {
        ImGui::EndChild();
        return;
    }
    if (ImGui::Checkbox(Translate1("Panels.OutfitEdit.PreviewArmor"), &preview_armor_) && !preview_armor_)
    {
        RE::Inventory3DManager::GetSingleton()->UnloadInventoryItem();
    }

    ImGui::SeparatorText(Translate1("Panels.Outfit.ModList"));
    constexpr int maxChildItemCount = 10;
    const auto    itemHeight        = ImGui::GetTextLineHeight();
    const float   childHeight       = (itemHeight + ImGui::GetStyle().ItemInnerSpacing.y) * maxChildItemCount;

    if (ImGui::BeginChild("##ModNameListChild", {0, childHeight}, ImGuiEx::ChildFlags().Borders().ResizeY()))
    {
        DrawArmorViewModNameFilterer();
    }
    ImGui::EndChild();

    ImGuiUtil::Text(Translate1("Panels.Outfit.BodySlots"));
    ImGui::BeginDisabled(show_no_conflict_armors_); // disable modify slot filter if `ShowNoConflictArmors` checked
    if (ImGui::BeginChild("#SlotFilterChild", {0, childHeight}, ImGuiEx::ChildFlags().Borders().ResizeY()))
    {
        DrawArmorViewSlotFilterer();
    }
    ImGui::EndChild();
    ImGui::EndDisabled();
    bool needUpdateView = ImGui::Checkbox(Translate1("Panels.OutfitEdit.ContainNoPlayable"), &armor_view_.contain_non_playable_armor_);
    needUpdateView      = ImGui::Checkbox(Translate1("Panels.OutfitEdit.ContainTemplate"), &armor_view_.contain_template_armor_) || needUpdateView;

    const auto slot_mask     = editingOutfit.get_slot_mask();
    auto      &slot_filterer = armor_view_.slot_filterer_;
    if (ImGui::Checkbox(Translate1("Panels.OutfitEdit.ShowNoConflictArmors"), &show_no_conflict_armors_))
    {
        if (show_no_conflict_armors_)
        {
            last_selected_slot_mask_ = slot_filterer.get_selected_slots();
            slot_filterer.flags      = armor_view::SlotFilterer::Flags::Skip_Has_Any_Slots;
            slot_filterer.set_select_slots(slot_mask);
        }
        else
        {
            slot_filterer.flags = armor_view::SlotFilterer::Flags::Pass_Has_Any_Slots;
            slot_filterer.set_select_slots(last_selected_slot_mask_);
        }

        needUpdateView = true;
    }

    if (show_no_conflict_armors_)
    {
        if (last_outfit_slot_mask_ != slot_mask) needUpdateView = true;
        slot_filterer.set_select_slots(slot_mask);
    }
    last_outfit_slot_mask_ = slot_mask;

    if (needUpdateView)
    {
        view_item_count_ = -1;
    }
    ImGui::EndChild();
}

void OutfitEditPanel::UpdateWindowTitle(const EditingOutfit &editingOutfit)
{
    if (last_editing_outfit_id_ != editingOutfit.GetId())
    {
        window_title_ = std::format(
            "{} {} - {}###OutfitEditPanel", Translate("Panels.OutfitEdit.EditingHint"), editingOutfit.get_name(), Translate("Panels.OutfitEdit.Title")
        );
    }
    last_editing_outfit_id_ = editingOutfit.GetId();
}

void OutfitEditPanel::draw_outfit_armors(EditingOutfit &editingOutfit)
{
    constexpr const char *SLOT_POLICY_HELP_POPUP_TITLE = "What is Slot Policy?";
    constexpr auto        ERROR_COLOR                  = IM_COL32(255, 180, 171, 255); ///< m3 default dark mode error color
    constexpr auto        ERROR_TEXT_COLOR             = IM_COL32(105, 0, 5, 255);     ///< m3 default dark mode error text color

    if (editingOutfit.is_invalid())
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
    if (ImGui::Button(Translate1("Panels.OutfitEdit.SlotPolicy.Name")))
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
        ImGui::TableSetupColumn(Translate1("Panels.OutfitEdit.SlotPolicy.Name"), ImGuiEx::TableColumnFlags().WidthFixed().NoSort());
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
            const auto has_conflict = selected_armors_slot_mask_.any(util::ToSlot(slotIdx));
            if (has_conflict)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ERROR_TEXT_COLOR);
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ERROR_COLOR);
            }

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

            ImGui::TableNextColumn(); // armor name column
            ImGui::Text("%s", armor == nullptr ? "" : armor->GetName());

            ImGui::TableNextColumn(); // slot policy combo column
            SlotPolicyCombo(editingOutfit, slotIdx);

            ImGui::TableNextColumn(); // action column
            {
                ImGui::BeginDisabled(editingOutfit.is_invalid() || armor == nullptr);
                if (ImGui::Button(Translate1("Delete")))
                {
                    spawn([&] { return outfit_service_->RemoveArmor(editingOutfit.GetId(), editingOutfit.get_name_str(), armor); });
                }
                ImGui::EndDisabled();
            }

            if (has_conflict)
            {
                ImGui::PopStyleColor();
            }
            ImGui::PopID();
        }
        ImGui::EndTable();
    }
}

void OutfitEditPanel::SlotPolicyCombo(EditingOutfit &editingOutfit, const uint32_t &slotIdx) const
{
    auto &preview_policy = editingOutfit.slot_policies[slotIdx];
    if (ImGui::BeginCombo("##SlotPolicy", Translate1(slot_policy_token(preview_policy)), ImGuiComboFlags_WidthFitPreview))
    {
        for (const auto &policy : {SlotPolicy::Inherit, SlotPolicy::Passthrough, SlotPolicy::RequireEquipped, SlotPolicy::AlwaysUseOutfit})
        {
            if (ImGui::Selectable(Translate1(slot_policy_token(policy)), false))
            {
                if (!editingOutfit.is_invalid())
                {
                    preview_policy = policy;
                    spawn([&] {
                        return outfit_service_->SetSlotPolicy(editingOutfit.GetId(), std::string(editingOutfit.get_name()), slotIdx, policy);
                    });
                }
            }
            ImGui::SetItemTooltip("%s", Translate1(slot_policy_tooltip(policy)));
        }
        ImGui::EndCombo();
    }
}

void OutfitEditPanel::DrawArmorSourcesTabBar()
{
    ZoneScopedN(__FUNCTION__);
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
                        armor_view_.reset_view_data(armor_source_, armor_source_refr_);
                        view_item_count_ = -1;
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
                        armor_view_.reset_view_data(armor_source_, armor_source_refr_);
                        view_item_count_ = -1;
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
                char          *pEnd{};
                const uint32_t formId = strtoul(formIdBuf.data(), &pEnd, 16);
                if (pEnd != formIdBuf.data())
                {
                    armor_source_refr_ = RE::TESForm::LookupByID<RE::TESObjectREFR>(formId);
                    armor_view_.reset_view_data(armor_source_, armor_source_refr_);
                    view_item_count_ = -1;
                }
            }
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem(Translate1("Panels.OutfitEdit.ArmorSource.AllArmors")))
        {
            armor_source_      = ArmorSource::All;
            armor_source_refr_ = nullptr;
            ImGui::EndTabItem();
        }

        if (oldArmorSource != armor_source_)
        {
            show_no_conflict_armors_ = false;
            armor_view_.reset_view(armor_source_, armor_source_refr_);
            view_item_count_ = -1;
        }

        ImGui::EndTabBar();
    }
}

void OutfitEditPanel::draw_armor_view(const EditingOutfit &editingOutfit)
{
    if (armor_view_.armor_name_filter_.Draw())
    {
        armor_view_.armor_name_filter_.dirty = false;
        view_item_count_                     = -1;
    }

    if (armor_view_.view_data_.empty())
    {
        return;
    }
    constexpr auto flags =
        ImGuiEx::TableFlags().RowBg().BordersInnerH().ScrollY().Resizable().SizingStretchProp().Sortable().Hideable().Reorderable();
    const auto styleGuard = ImGuiEx::StyleGuard().Style<ImGuiStyleVar_FramePadding>({3.0F, 3.0F});
    if (ImGui::BeginTable("##ArmorCandidates", 6, flags))
    {
        draw_armor_view_content(editingOutfit);
        ImGui::EndTable();
    }
}

void OutfitEditPanel::draw_preview_armor_window(const Armor *to_preview_armor)
{
    constexpr ImVec2 center_pivot{0.5F, 0.5F};
    constexpr auto   preview_window_name = "preview_armor";

    auto *inventory_manager = RE::Inventory3DManager::GetSingleton();
    if (inventory_manager == nullptr || !preview_armor_)
    {
        return;
    }
    ZoneScopedN(__FUNCTION__);
    if (to_preview_armor != nullptr && to_preview_armor != previewing_armor_)
    {
        ZoneScopedN("LoadInventoryItem");
        previewing_armor_ = const_cast<Armor *>(to_preview_armor);
        inventory_manager->LoadInventoryItem(previewing_armor_, nullptr);
    }

    inventory_manager->Render();
    if (auto &runtime_data = inventory_manager->GetRuntimeData(); !runtime_data.loadedModels.empty())
    {
        const RE::LoadedInventoryModel &loaded_model = runtime_data.loadedModels.back();
        if (const auto &loaded_sp_model = loaded_model.spModel; loaded_sp_model != nullptr)
        {
            auto        &translate     = loaded_sp_model->local.translate;
            const auto  &view_frustum  = RE::UI3DSceneManager::GetSingleton()->viewFrustum;
            const float  world_minx    = -view_frustum.fLeft * translate.y;
            const float  world_minz    = -view_frustum.fBottom * translate.y;
            const float  world_width   = -view_frustum.fRight * translate.y - world_minx;
            const float  world_height  = -view_frustum.fTop * translate.y - world_minz;
            const auto  &viewport_size = ImGui::GetMainViewport()->Size;
            const ImVec2 viewport_ratio(world_width / viewport_size.x, world_height / viewport_size.y);

            const auto   model_radius        = loaded_sp_model->worldBound.radius;
            const auto   scaled_model_radius = model_radius / viewport_ratio.x;
            const ImVec2 window_size{scaled_model_radius * 2.0F, scaled_model_radius * 2.0F};

            // relayout if window size will change: keep window position locate by center.
            if (const ImGuiWindow *preview_window = ImGui::FindWindowByName(preview_window_name); preview_window != nullptr)
            {
                if (preview_window->Size != window_size)
                {
                    const auto &window_center = preview_window->Pos + preview_window->Size * center_pivot;
                    ImGui::SetNextWindowPos(window_center, 0, center_pivot);
                }
            }
            ImGui::SetNextWindowSize(window_size);
            if (ImGui::Begin(preview_window_name, nullptr, ImGuiEx::WindowFlags().NoResize().NoDecoration().NoBackground()))
            {
                const auto &window_pos = ImGui::GetWindowPos();
                const auto  new_x      = -(viewport_ratio.x * window_pos.x + model_radius + world_minx);
                const auto  new_z      = -(viewport_ratio.y * window_pos.y + model_radius + world_minz);
                translate.x            = (translate.x - loaded_sp_model->worldBound.center.x) + new_x;
                translate.z            = (translate.z - loaded_sp_model->worldBound.center.z) + new_z;
            }
            ImGui::End();
        }
    }
}

void OutfitEditPanel::draw_armor_view_content(const EditingOutfit &editingOutfit)
{
    ImGui::TableSetupScrollFreeze(1, 1);
    ImGui::TableSetupColumn("##Number", ImGuiEx::TableColumnFlags().NoSort().WidthFixed());
    ImGui::TableSetupColumn(Translate1("Armor"), ImGuiEx::TableColumnFlags().DefaultSort().NoHide());
    ImGui::TableSetupColumn("FormID", ImGuiEx::TableColumnFlags().DefaultHide().NoSort());
    ImGui::TableSetupColumn(Translate1("Panels.OutfitEdit.ModName"), ImGuiEx::TableColumnFlags().DefaultHide().NoSort());
    ImGui::TableSetupColumn(Translate1("Panels.OutfitEdit.Playable"), ImGuiEx::TableColumnFlags().DefaultHide().NoSort());
    ImGui::TableSetupColumn(Translate1("##Add"), ImGuiEx::TableColumnFlags().NoSort());
    ImGui::TableHeadersRow();

    if (auto *sortSpecs = ImGui::TableGetSortSpecs(); sortSpecs != nullptr)
    {
        if (sortSpecs->SpecsDirty && sortSpecs->SpecsCount > 0)
        {
            const auto direction    = sortSpecs->Specs[0].SortDirection;
            armor_name_sort_ascend_ = direction == ImGuiSortDirection_Ascending;
            sortSpecs->SpecsDirty   = false;
        }
    }

    selected_armors_slot_mask_    = Slot::kNone;
    const Armor *to_preview_armor = nullptr;

    draw_armor_view(armor_view_.view_data_, editingOutfit.is_invalid(), to_preview_armor);

    draw_preview_armor_window(to_preview_armor);

    draw_add_armors_popup(editingOutfit);
}

void OutfitEditPanel::draw_armor_view(const std::vector<ArmorEntry> &viewData, const bool editing_invalid_outfit, const Armor *&to_preview_armor)
{
    auto          &multi_selection = armor_view_.multi_selection_;
    constexpr auto ms_flags        = ImGuiEx::MultiSelectFlags().NoSelectAll().BoxSelect1d().ClearOnEscape().ClearOnClickVoid();
    auto          *msIO            = ImGui::BeginMultiSelect(ms_flags, multi_selection.Size, static_cast<int>(viewData.size()));
    multi_selection.ApplyRequests(msIO);
    ImGuiListClipper clipper;
    const bool       item_count_known = view_item_count_ != -1;
    clipper.Begin(item_count_known ? view_item_count_ : INT_MAX);
    if (msIO->RangeSrcItem != -1)
    {
        clipper.IncludeItemByIndex(static_cast<int>(msIO->RangeSrcItem));
    }

    bool      want_add_armor = false;
    const int step           = armor_name_sort_ascend_ ? -1 : 1;
    const int start          = armor_name_sort_ascend_ ? static_cast<int>(viewData.size()) - 1 : 0;
    const int end            = armor_name_sort_ascend_ ? -1 : static_cast<int>(viewData.size());
    int       index          = start;
    while (clipper.Step())
    {
        const auto start_index = clipper.UserIndex;
        while (index != end && clipper.UserIndex < clipper.DisplayEnd)
        {
            const auto  uIndex = static_cast<ImGuiID>(index);
            const auto &armor  = viewData[uIndex];
            if (armor_view_.filter(armor))
            {
                if (clipper.UserIndex >= clipper.DisplayStart)
                {
                    draw_armor_row(uIndex, armor, editing_invalid_outfit, want_add_armor, to_preview_armor);
                }
                clipper.UserIndex++;
            }
            index += step;
        }
        if (clipper.UserIndex == start_index) // no anymore
        {
            clipper.End();
            break;
        }
    }
    multi_selection.ApplyRequests(ImGui::EndMultiSelect());
    if (!item_count_known)
    {
        while (index != end)
        {
            const auto  uIndex = static_cast<ImGuiID>(index);
            const auto &armor  = viewData[uIndex];
            if (armor_view_.filter(armor))
            {
                clipper.UserIndex++;
            }
            index += step;
        }
        view_item_count_ = clipper.UserIndex;
        clipper.SeekCursorForItem(view_item_count_);
    }
    if (ImGui::IsAnyItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Right))
    {
        ImGui::OpenPopup("Context");
    }

    if (ImGui::BeginPopup("Context"))
    {
        ImGui::BeginDisabled(editing_invalid_outfit || armor_view_.multi_selection_.Size <= 0);
        if (ImGui::MenuItem(Translate1("Panels.OutfitEdit.AddAll")))
        {
            want_add_armor = true;
        }
        ImGui::EndDisabled();
        ImGui::EndPopup();
    }
    if (want_add_armor)
    {
        waiting_add_armor_count_ = armor_view_.multi_selection_.Size;
        conflict_solution_       = ConflictSolution::none;
        ImGui::OpenPopup(Add_Armors_Progress_Popup_Title);
    }
}

void OutfitEditPanel::draw_armor_row(
    const ImGuiID index, const Armor *armor, const bool editing_invalid_outfit, bool &want_add_armor, const Armor *&to_preview_armor
)
{
    ImGui::PushID(static_cast<int>(index));
    ImGui::TableNextRow();
    if (ImGui::TableNextColumn()) // number column
    {
        auto &multiSelection = armor_view_.multi_selection_;
        ImGui::SetNextItemSelectionUserData(index);
        ImGui::Selectable(
            std::format("{}", index + 1).c_str(), multiSelection.Contains(index), ImGuiEx::SelectableFlags().AllowOverlap().SpanAllColumns()
        );
        if (multiSelection.Contains(index))
        {
            selected_armors_slot_mask_.set(armor->GetSlotMask().get());
        }
        if (ImGui::IsItemHovered())
        {
            to_preview_armor = armor;
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

    ImGui::BeginDisabled(editing_invalid_outfit);
    if (ImGui::TableNextColumn() && ImGui::Button(Translate1("Add"))) // column Action
    {
        armor_view_.multi_selection_.Clear();
        armor_view_.multi_selection_.SetItemSelected(index, true);
        want_add_armor = true;
    }
    ImGui::EndDisabled();
    ImGui::PopID();
}

void OutfitEditPanel::draw_add_armors_popup(const EditingOutfit &outfit)
{
    bool open = true;
    if (ImGui::BeginPopupModal(Add_Armors_Progress_Popup_Title, &open))
    {
        const int   remaining = waiting_add_armor_count_ - armor_view_.multi_selection_.Size;
        const float fraction  = static_cast<float>(remaining) / static_cast<float>(waiting_add_armor_count_);
        ImGui::ProgressBar(fraction);

        void   *it     = nullptr;
        ImGuiID nextId = 0;
        if (armor_view_.multi_selection_.GetNextSelectedItem(&it, &nextId))
        {
            const auto &rankedArmor = armor_view_.view_data_.at(nextId);
            ImGuiUtil::Text(std::format("Add armor {}...", rankedArmor->GetName()));
        }

        if (conflict_solution_ == ConflictSolution::Suspend)
        {
            ImGuiUtil::Text("Conflict with exist armors, do you want to:");
            if (ImGui::Button(Translate1("Panels.OutfitEdit.ConflictSolution.Skip")))
            {
                conflict_solution_ = ConflictSolution::Skip;
            }
            ImGui::SameLine();
            if (ImGui::Button(Translate1("Panels.OutfitEdit.ConflictSolution.SkipAll")))
            {
                conflict_solution_ = ConflictSolution::SkipAll;
            }
            ImGui::Separator();
            if (ImGui::Button(Translate1("Panels.OutfitEdit.ConflictSolution.Overwrite")))
            {
                conflict_solution_ = ConflictSolution::Overwrite;
            }
            ImGui::SameLine();
            if (ImGui::Button(Translate1("Panels.OutfitEdit.ConflictSolution.OverwriteAll")))
            {
                conflict_solution_ = ConflictSolution::OverwriteAll;
            }
        }
        AddSelectArmors(outfit);
        if (armor_view_.multi_selection_.Size == 0)
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void OutfitEditPanel::DrawArmorViewModNameFilterer()
{
    for (auto &modEntry : armor_view_.mod_filterer_.mod_list)
    {
        auto label = std::format("({}) {}", modEntry.count, modEntry.name);
        if (ImGui::Checkbox(label.c_str(), &modEntry.checked))
        {
            view_item_count_ = -1;
        }
    }
}

void OutfitEditPanel::DrawArmorViewSlotFilterer()
{
    const std::string name =
        std::format("{}({}/{})##AllSlot", Translate("Panels.OutfitEdit.PassAllSlots"), armor_view_.view_data_.size(), armor_view_.armor_count);

    auto &slot_filterer = armor_view_.slot_filterer_;
    bool  checked_all   = slot_filterer.is_pass_always();
    if (ImGui::Checkbox(name.c_str(), &checked_all))
    {
        // FIXME: may change api to receive Flags, now slots-filter will be disable when flag is 'Skip_has_Any_Slots'
        slot_filterer.flags = checked_all ? armor_view::SlotFilterer::Flags::Pass_Always : armor_view::SlotFilterer::Flags::Pass_Has_Any_Slots;
        view_item_count_    = -1;
    }

    for (auto [slot_idx, slot_count] : armor_view_.slot_counter_ | std::views::enumerate)
    {
        const auto slot = static_cast<SlotType>(slot_idx);
        ImGui::PushID(static_cast<int>(slot_idx));

        auto slotLabel = std::format("({}) {}", slot_count, Translate(get_slot_name_key(slot)));
        bool checked   = slot_filterer.is_slot_selected(slot);
        if (ImGui::Checkbox(slotLabel.c_str(), &checked))
        {
            slot_filterer.select_slot(slot, checked);
            view_item_count_ = -1;
        }
        ImGui::PopID();
    }
}

void OutfitEditPanel::AddSelectArmors(const EditingOutfit &outfit)
{
    if (armor_view_.multi_selection_.Size <= 0 || conflict_solution_ == ConflictSolution::Suspend)
    {
        return;
    }

    REX::EnumSet<Slot> usedSlot;
    void              *it     = nullptr;
    ImGuiID            nextId = 0;
    for (; armor_view_.multi_selection_.GetNextSelectedItem(&it, &nextId); armor_view_.multi_selection_.SetItemSelected(nextId, false))
    {
        const auto &armor = armor_view_.view_data_.at(nextId);
        if (usedSlot.all(armor->GetSlotMask().get()) || outfit.is_conflict_with(armor))
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

            if (conflict_solution_ == ConflictSolution::SkipAll)
            {
                continue;
            }

            // overwrite or overwrite-all
            if (conflict_solution_ == ConflictSolution::Overwrite)
            {
                conflict_solution_ = ConflictSolution::none;
            }

            spawn([&] { return OutfitService::DeleteConflictArmors(outfit.get_name_str(), armor); });
        }

        usedSlot.set(armor->GetSlotMask().get());
        spawn([&] { return outfit_service_->AddArmor(outfit.GetId(), outfit.get_name_str(), armor); });
    }

    spawn([&] { return outfit_service_->GetOutfitArmors(outfit.GetId(), outfit.get_name_str()); });
}

} // namespace SosGui
