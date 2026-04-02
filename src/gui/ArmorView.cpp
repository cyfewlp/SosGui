//
// Created by jamie on 2025/5/7.
//

#include "gui/ArmorView.h"

#include "SosDataType.h"
#include "gui/icon.h"
#include "imgui.h"

#include <ranges>

namespace SosGui
{

bool armor_view::ArmorNameFilter::PassFilter(const Armor *armor) const
{
    if (!filter.IsActive())
    {
        return true;
    }
    if (DebounceInput::PassFilter(armor->GetName()))
    {
        return true;
    }
    return false;
}

bool armor_view::ArmorNameFilter::Draw()
{
    ImGui::AlignTextToFramePadding();
    ImGuiUtil::IconButton(ICON_SEARCH);

    ImGui::SameLine();
    bool needUpdate = DebounceInput::Draw("##ArmorFilter", Translation::Translate("$SkyOutSys_OEdit_AddFromList_Filter_Name").c_str());
    return needUpdate;
}

auto armor_view::SlotFilterer::pass_filter(const Armor *armor) const -> bool
{
    if (enable_all_slot_ || armor->HasPartOf(static_cast<Slot>(selected_slots_.to_ulong())))
    {
        return true;
    }
    return false;
}

bool armor_view::ModFilterer::PassFilter(const Armor *armor) const
{
    bool result = true;
    if (mod_entry_map.empty()) return result;
    for (const auto &[modName, modEntry] : mod_entry_map)
    {
        if (modEntry.checked)
        {
            result = false;
            if (util::GetFormModFileName(armor) == modName)
            {
                return true;
            }
        }
    }
    return result;
}

void ArmorView::init()
{
    if (armor_container_.Size() != 0)
    {
        return;
    }
    armor_container_.Init();
}

void ArmorView::on_refresh()
{
    view_data_.clear();
    slot_counter_.fill(0);
    slot_filterer_.clear();
    multi_selection_.Clear();
    armor_name_filter_.Clear();
    mod_filterer_.clear();
}

void ArmorView::clear()
{
    on_refresh();
    armor_container_.Clear();
}

void ArmorView::clear_view_data()
{
    view_data_.clear();
    multi_selection_.Clear();
}

auto ArmorView::add_armor(const Armor *armor) -> std::expected<void, error>
{
    if (filter(armor))
    {
        const auto armorRank = armor_container_.GetRank(armor->GetName(), armor->formID);
        if (armorRank >= armor_container_.Size())
        {
            return std::unexpected{error::unknown_armor};
        }
        const auto ranked = RankedArmor(armor, armorRank);
        const auto found  = std::lower_bound(view_data_.begin(), view_data_.end(), ranked);

        if (found != view_data_.end())
        {
            if (found->get_rank() == armorRank)
            {
                return std::unexpected{error::armor_already_exists};
            }
            if (found->get_rank() >= armor_container_.Size())
            {
                return std::unexpected{error::unassociated_armor};
            }
        }
        view_data_.insert(found, ranked);
    }
    return {};
}

void ArmorView::add_armors_has_slot(ArmorGenerator *generator, const SosUiOutfit *editing_outfit, Slot slots)
{
    generator->ForEach([&](const Armor *armor) {
        if (!editing_outfit->HasArmor(armor) && armor->HasPartOf(slots))
        {
            if (const auto result = add_armor(armor); !result.has_value())
            {
                if (result.error() != error::armor_already_exists)
                {
                    // TODO: show error in UI
                    logger::error("Can't add armor[{}]: {}", armor->formID, to_error_message(result.error()));
                }
            }
        }
    });
}

bool ArmorView::remove_armor(const Armor *armor)
{
    if (auto found = find_armor(armor); found.has_value() && found.value() != view_data_.end())
    {
        view_data_.erase(found.value());
        for (SlotType slotPos = 0; slotPos < RE::BIPED_OBJECT::kEditorTotal; slotPos++)
        {
            if (armor->HasPartOf(util::ToSlot(slotPos)))
            {
                slot_counter_[slotPos] -= 1;
            }
        }
        return true;
    }
    return false;
}

void ArmorView::remove_armors_has_slot(Slot slots)
{
    multi_selection_.Clear();
    for (auto itBegin = view_data_.begin(); itBegin != view_data_.end();)
    {
        const auto &rankedArmor = *itBegin;
        if (rankedArmor->HasPartOf(slots) && util::IsArmorNotHasSlotOf(rankedArmor.data(), slot_filterer_.get_selected_slots()))
        {
            itBegin = view_data_.erase(itBegin);
        }
        else
        {
            ++itBegin;
        }
    }
}

void ArmorView::remove_armors_no_has_slots(Slot slots)
{
    multi_selection_.Clear();
    const auto selectedSlots = slot_filterer_.get_selected_slots();
    for (auto itBegin = view_data_.begin(); itBegin != view_data_.end();)
    {
        const auto &rankedArmor = *itBegin;
        if (util::IsArmorNotHasSlotOf(rankedArmor.data(), slots) && util::IsArmorNotHasSlotOf(rankedArmor.data(), selectedSlots))
        {
            itBegin = view_data_.erase(itBegin);
        }
        else
        {
            ++itBegin;
        }
    }
}

void ArmorView::reset_counter()
{
    mod_filterer_.clear();
    slot_counter_.fill(0);
    for (const auto &rankedArmor : view_data_)
    {
        std::string_view modName = util::GetFormModFileName(rankedArmor.data());
        auto             it      = mod_filterer_.try_emplace(modName, 0, false);
        it.first->second.count += 1;

        for (SlotType slotPos = 0; slotPos < RE::BIPED_OBJECT::kEditorTotal; slotPos++)
        {
            if (rankedArmor->HasPartOf(util::ToSlot(slotPos)))
            {
                slot_counter_[slotPos]++;
            }
        }
    }
}

void ArmorView::reset_view_data(ArmorGenerator *generator)
{
    clear_view_data();
    generator->ForEach([&](const Armor *armor) {
        if (const auto result = add_armor(armor); !result.has_value())
        {
            // TODO: show error in UI
            logger::error("Can't add armor[{}]: {}", armor->formID, to_error_message(result.error()));
        }
    });
}

void ArmorView::reset_view(ArmorGenerator *generator)
{
    mod_filterer_.clear();
    slot_filterer_.clear();
    contain_non_playable_armor_ = true;
    armor_name_filter_.filter.Clear();

    reset_view_data(generator);
    reset_counter();
}

bool ArmorView::filter(const Armor *armor) const
{
    if (!mod_filterer_.PassFilter(armor))
    {
        return false;
    }
    if (!slot_filterer_.pass_filter(armor))
    {
        return false;
    }
    // filter by armor name
    if (contain_non_playable_armor_ || util::IsArmorPlayable(armor))
    {
        if (!armor_name_filter_.PassFilter(armor))
        {
            return false;
        }
    }
    return true;
}

void ArmorView::filterer_enable_all_slots(bool enable_all, ArmorGenerator *generator, const SosUiOutfit *editing_outfit)
{
    const auto old = slot_filterer_.is_enable_all_slots();
    if (old == enable_all) return;
    slot_filterer_.enable_all_slots(enable_all);

    if (slot_filterer_.is_enable_all_slots())
    {
        reset_view_data(generator);
    }
    else if (slot_filterer_.is_all_slots_disabled())
    {
        clear_view_data();
    }
    else
    {
        remove_armors_no_has_slots(slot_filterer_.get_selected_slots());
    }
}

void ArmorView::filterer_select_slot(SlotType slotPos, bool select, ArmorGenerator *generator, const SosUiOutfit *editing_outfit)
{
    if (const auto old = slot_filterer_.is_slot_selected(slotPos); old == select)
    {
        return;
    }
    slot_filterer_.select_slot(slotPos, select);

    // cancel enable all slots when select/unselect any slot
    const auto oldIsEnableAllSlots = slot_filterer_.is_enable_all_slots();
    if (oldIsEnableAllSlots)
    {
        slot_filterer_.enable_all_slots(false);
    }

    if (slot_filterer_.is_all_slots_disabled())
    {
        clear_view_data();
    }
    else if (!oldIsEnableAllSlots)
    {
        const Slot slot = util::ToSlot(slotPos);
        if (select)
        {
            add_armors_has_slot(generator, editing_outfit, slot);
        }
        else
        {
            remove_armors_has_slot(slot);
        }
    }
    else if (!slot_filterer_.is_all_slots_enabled())
    {
        reset_view_data(generator);
    }
}

auto ArmorView::find_armor(const Armor *armor) const -> std::expected<const_iterator, error>
{
    const auto armorRank = armor_container_.GetRank(armor->GetName(), armor->formID);
    if (armorRank >= armor_container_.Size())
    {
        return std::unexpected{error::unknown_armor};
    }

    const RankedArmor ranked(armor, armorRank);
    const auto        found = std::lower_bound(view_data_.begin(), view_data_.end(), ranked);

    if (found != view_data_.end())
    {
        if (found->get_rank() >= armor_container_.Size())
        {
            return std::unexpected{error::unassociated_armor};
        }
    }
    return found;
}
} // namespace SosGui
