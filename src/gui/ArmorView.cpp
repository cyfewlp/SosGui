//
// Created by jamie on 2025/5/7.
//

#include "gui/ArmorView.h"

#include "SosDataType.h"
#include "gui/icon.h"
#include "imgui.h"
#include "tracy/Tracy.hpp"
#include "util/utils.h"

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

auto armor_view::ArmorNameFilter::PassFilter(const Armor *armor) const -> bool
{
    ZoneScopedN(__FUNCTION__);
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

auto armor_view::ArmorNameFilter::Draw() -> bool
{
    ImGui::AlignTextToFramePadding();
    ImGuiUtil::IconButton(ICON_SEARCH);

    ImGui::SameLine();
    const bool needUpdate = DebounceInput::Draw("##ArmorFilter", Translation::Translate("$SkyOutSys_OEdit_AddFromList_Filter_Name").c_str());
    return needUpdate;
}

auto armor_view::SlotFilterer::pass_filter(const Armor *armor) const -> bool
{
    ZoneScopedN(__FUNCTION__);
    return enable_all_slot_ || util::IsArmorHasAnySlotOf(armor, get_selected_slots());
}

auto armor_view::ModFilterer::PassFilter(const Armor *armor) const -> bool
{
    ZoneScopedN(__FUNCTION__);
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

void ArmorView::clear_all()
{
    view_data_.clear();
    armor_count = 0;
    slot_counter_.fill(0);
    slot_filterer_.clear();
    multi_selection_.Clear();
    armor_name_filter_.Clear();
    mod_filterer_.clear();
}

void ArmorView::clear_view_data()
{
    view_data_.clear();
    multi_selection_.Clear();
}

auto ArmorView::add_armor(const Armor *armor) -> std::expected<void, error>
{
    ZoneScopedN(__FUNCTION__);
    if (filter(armor))
    {
        auto found = find(armor);

        while (found != view_data_.end() && (*found)->GetFormID() != armor->GetFormID() && (*found)->GetName() == armor->GetName())
        {
            ++found;
        }
        if (found != view_data_.end() && (*found)->GetFormID() == armor->GetFormID())
        {
            return std::unexpected{error::armor_already_exists};
        }
        view_data_.emplace(found, armor);
    }
    return {};
}

void ArmorView::add_armors_for_slots(ArmorSource source, RE::TESObjectREFR *source_ref, Slot slots)
{
    ZoneScopedN(__FUNCTION__);
    const auto oldSlots = slot_filterer_.get_selected_slots();
    slot_filterer_.set_select_slots(slots);
    switch (source)
    {
        case ArmorSource::None:
            break;
        case ArmorSource::Armor: {
            if (const auto *armor = source_ref != nullptr ? source_ref->As<Armor>() : nullptr; armor != nullptr)
            {
                try_add_armor(armor);
            }
            break;
        }
        case ArmorSource::Inventory: {
            if (source_ref == nullptr) break;
            for (const auto &object : source_ref->GetInventory() | std::views::keys)
            {
                try_add_armor(object->As<Armor>());
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
                    try_add_armor(armor);
                }
            }
            break;
        }
        case ArmorSource::All: {
            auto       *dataHandler = RE::TESDataHandler::GetSingleton();
            const auto &armorArray  = dataHandler->GetFormArray<RE::TESObjectARMO>();
            for (const auto &armor : armorArray)
            {
                try_add_armor(armor);
            }
            break;
        }
    }
    slot_filterer_.set_select_slots(oldSlots);
}

auto ArmorView::remove_armor(const Armor *armor) -> bool
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

void ArmorView::prune_view_by_slot_filter()
{
    ZoneScopedN(__FUNCTION__);
    multi_selection_.Clear();
    std::erase_if(view_data_, [this](const ArmorEntry &entry) { return !slot_filterer_.pass_filter(entry); });
}

void ArmorView::reset_counter()
{
    ZoneScopedN(__FUNCTION__);
    mod_filterer_.clear();
    slot_counter_.fill(0);
    for (const auto &armor : view_data_)
    {
        const std::string_view modName = util::GetFormModFileName(armor);
        auto                   it      = mod_filterer_.try_emplace(modName, 0, false);
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
    armor_count = 0;
    switch (source)
    {
        case ArmorSource::None:
            break;
        case ArmorSource::Armor: {
            if (const auto *armor = source_ref != nullptr ? source_ref->As<Armor>() : nullptr; filter(armor))
            {
                armor_count = 1;
                view_data_.emplace_back(armor);
            }
            break;
        }
        case ArmorSource::Inventory: {
            if (source_ref == nullptr) break;
            const auto objects = source_ref->GetInventory() | std::views::keys;
            for (const auto &object : objects)
            {
                if (const auto *armor = object->As<Armor>(); armor != nullptr)
                {
                    armor_count += 1;
                    if (filter(armor))
                    {
                        view_data_.emplace_back(armor);
                    }
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
                armor_count = visitor.armors.size();
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
            armor_count             = armorArray.size();
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
    armor_count = view_data_.size();
}

void ArmorView::reset_view(ArmorSource source, RE::TESObjectREFR *source_ref)
{
    ZoneScopedN(__FUNCTION__);
    mod_filterer_.clear();
    slot_filterer_.clear();
    contain_non_playable_armor_ = true;
    armor_name_filter_.filter.Clear();

    reset_view_data(source, source_ref);
    reset_counter();
}

auto ArmorView::filter(const Armor *armor) const -> bool
{
    ZoneScopedN(__FUNCTION__);
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

void ArmorView::set_enable_all_slots_filter(bool enable_all, ArmorSource source, RE::TESObjectREFR *source_ref)
{
    const auto oldIsAllSlotsEnabled = slot_filterer_.is_all_slots_enabled();
    slot_filterer_.enable_all_slots(enable_all);
    if (oldIsAllSlotsEnabled == slot_filterer_.is_all_slots_enabled()) return;

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
        prune_view_by_slot_filter();
    }
}

void ArmorView::set_slot_filter(SlotType slotPos, bool select, ArmorSource source, RE::TESObjectREFR *source_ref)
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
            add_armors_for_slots(source, source_ref, slot);
        }
        else
        {
            prune_view_by_slot_filter();
        }
    }
    else if (!slot_filterer_.is_all_slots_enabled())
    {
        reset_view_data(source, source_ref);
    }
}

auto ArmorView::find(const Armor *armor) const -> const_iterator
{
    const ArmorEntry armorEntry(armor);
    return std::ranges::lower_bound(view_data_, armorEntry, ArmorNameLessComparator());
}
} // namespace SosGui
