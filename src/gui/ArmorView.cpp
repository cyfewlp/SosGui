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
namespace
{
struct ArmorNameLessComparator
{
    auto operator()(const ArmorEntry &lhs, const ArmorEntry &rhs) -> bool { return util::StrLess(lhs.name, rhs.name); }
};
} // namespace

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
        auto found = find(armor);

        if (found != view_data_.end() && (*found)->GetFormID() == armor->GetFormID())
        {
            return std::unexpected{error::armor_already_exists};
        }
        view_data_.emplace(found, armor);
    }
    return {};
}

void ArmorView::add_armors_has_slot(ArmorSource source, RE::TESObjectREFR *source_ref, Slot slots)
{
    const auto oldSlots = slot_filterer_.get_selected_slots();
    slot_filterer_.select_slots(Slot::kNone);
    slot_filterer_.select_slots(slots);
    switch (source)
    {
        case ArmorSource::None:
            break;
        case ArmorSource::Armor: {
            if (const auto armor = source_ref != nullptr ? source_ref->As<Armor>() : nullptr; armor != nullptr)
            {
                add_armor_log_error(armor);
            }
            break;
        }
        case ArmorSource::Inventory: {
            if (source_ref == nullptr) break;
            for (const auto &object : source_ref->GetInventory() | std::views::keys)
            {
                add_armor_log_error(object->As<Armor>());
            }
            break;
        }
        case ArmorSource::Carried: {
            if (source_ref == nullptr) break;
            if (auto *changes = source_ref->GetInventoryChanges(); changes != nullptr)
            {
                ArmorItemVisitor visitor;
                changes->VisitWornItems(visitor);
                for (const auto &armor : visitor.armors)
                {
                    add_armor_log_error(armor);
                }
            }
            break;
        }
        case ArmorSource::All: {
            auto       *dataHandler = RE::TESDataHandler::GetSingleton();
            const auto &armorArray  = dataHandler->GetFormArray<RE::TESObjectARMO>();
            for (const auto &armor : armorArray)
            {
                add_armor_log_error(armor);
            }
            break;
        }
    }
    slot_filterer_.select_slots(Slot::kNone);
    slot_filterer_.select_slots(oldSlots);
}

bool ArmorView::remove_armor(const Armor *armor)
{
    if (auto found = find(armor); found != view_data_.end())
    {
        view_data_.erase(found);
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
        const auto &armor = *itBegin;
        if (armor->HasPartOf(slots) && util::IsArmorNotHasSlotOf(armor, slot_filterer_.get_selected_slots()))
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
        const auto &armor = *itBegin;
        if (util::IsArmorNotHasSlotOf(armor, slots) && util::IsArmorNotHasSlotOf(armor, selectedSlots))
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
    for (const auto &armor : view_data_)
    {
        std::string_view modName = util::GetFormModFileName(armor);
        auto             it      = mod_filterer_.try_emplace(modName, 0, false);
        it.first->second.count += 1;

        for (SlotType slotPos = 0; slotPos < RE::BIPED_OBJECT::kEditorTotal; slotPos++)
        {
            if (armor->HasPartOf(util::ToSlot(slotPos)))
            {
                slot_counter_[slotPos]++;
            }
        }
    }
}

void ArmorView::reset_view_data(ArmorSource source, RE::TESObjectREFR *source_ref)
{
    clear_view_data();
    switch (source)
    {
        case ArmorSource::None:
            break;
        case ArmorSource::Armor: {
            if (const auto armor = source_ref != nullptr ? source_ref->As<Armor>() : nullptr; filter(armor))
            {
                view_data_.emplace_back(armor);
            }
            break;
        }
        case ArmorSource::Inventory: {
            if (source_ref == nullptr) break;
            for (const auto &pair : source_ref->GetInventory())
            {
                if (const auto *armor = pair.first->As<Armor>(); filter(armor))
                {
                    view_data_.emplace_back(armor);
                }
            }
            break;
        }
        case ArmorSource::Carried: {
            if (source_ref == nullptr) break;
            if (auto *changes = source_ref->GetInventoryChanges(); changes != nullptr)
            {
                ArmorItemVisitor visitor;
                changes->VisitWornItems(visitor);
                for (const auto &armor : visitor.armors)
                {
                    if (filter(armor))
                    {
                        view_data_.emplace_back(armor);
                    }
                }
            }
            break;
        }
        case ArmorSource::All: {
            auto       *dataHandler = RE::TESDataHandler::GetSingleton();
            const auto &armorArray  = dataHandler->GetFormArray<RE::TESObjectARMO>();
            for (const auto &armor : armorArray)
            {
                if (filter(armor))
                {
                    view_data_.emplace_back(armor);
                }
            }
            break;
        }
    }
    std::ranges::sort(view_data_, ArmorNameLessComparator());
    armor_count    = view_data_.size();
}

void ArmorView::reset_view(ArmorSource source, RE::TESObjectREFR *source_ref)
{
    mod_filterer_.clear();
    slot_filterer_.clear();
    contain_non_playable_armor_ = true;
    armor_name_filter_.filter.Clear();

    reset_view_data(source, source_ref);
    reset_counter();
}

auto ArmorView::filter(const Armor *armor) const -> bool
{
    if (armor == nullptr || !is_armor_can_display(armor))
    {
        return false;
    }

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

void ArmorView::filterer_enable_all_slots(bool enable_all, ArmorSource source, RE::TESObjectREFR *source_ref)
{
    const auto old = slot_filterer_.is_enable_all_slots();
    if (old == enable_all) return;
    slot_filterer_.enable_all_slots(enable_all);

    if (slot_filterer_.is_enable_all_slots())
    {
        reset_view_data(source, source_ref);
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

void ArmorView::filterer_select_slot(SlotType slotPos, bool select, ArmorSource source, RE::TESObjectREFR *source_ref)
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
            add_armors_has_slot(source, source_ref, slot);
        }
        else
        {
            remove_armors_has_slot(slot);
        }
    }
    else if (!slot_filterer_.is_all_slots_enabled())
    {
        reset_view_data(source, source_ref);
    }
}

auto ArmorView::find(const Armor *armor) const -> const_iterator
{
    ArmorEntry armorEntry(armor);
    return std::ranges::lower_bound(view_data_, armorEntry, ArmorNameLessComparator());
}
} // namespace SosGui
