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
    return is_all_slots_enabled()                                                                            //
           || (flags == Flags::Pass_Has_Any_Slots && util::IsArmorHasAnySlotOf(armor, get_selected_slots())) //
           || (flags == Flags::Skip_Has_Any_Slots && util::IsArmorNotHasSlotOf(armor, get_selected_slots()));
}

auto armor_view::ModFilterer::pass_filter(const Armor *armor) const -> bool
{
    ZoneScopedN(__FUNCTION__);
    if (mod_list.empty()) return true;

    const auto mod_name    = util::GetFormModFileName(armor);
    bool       any_checked = false;
    bool       pass        = false;
    for (const auto &mod_entry : mod_list)
    {
        if (mod_entry.checked)
        {
            any_checked = true;
            if (mod_entry.name == mod_name)
            {
                pass = true;
                break;
            }
        }
    }
    return !any_checked || pass;
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

void ArmorView::reset_counter()
{
    ZoneScopedN(__FUNCTION__);
    mod_filterer_.clear();
    slot_counter_.fill(0);
    for (const auto &armor : view_data_)
    {
        const std::string_view modName   = util::GetFormModFileName(armor);
        auto                  &mod_entry = mod_filterer_.try_emplace(modName, 0, false);
        mod_entry.count += 1;

        for (auto [index, slot_count] : slot_counter_ | std::views::enumerate)
        {
            if (armor->HasPartOf(static_cast<Slot>(1 << index)))
            {
                slot_count++;
            }
        }
    }
    std::ranges::sort(mod_filterer_.mod_list, std::less<>(), &armor_view::ModFilterer::ModEntry::name);
}

void ArmorView::reset_view_data(ArmorSource source, RE::TESObjectREFR *source_ref)
{
    ZoneScopedN(__FUNCTION__);
    clear_view_data();
    armor_count = 0;
    switch (source)
    {
        case ArmorSource::None:
            break;
        case ArmorSource::Armor: {
            ZoneScopedN("reset view data emplace armor");
            if (const auto *armor = source_ref != nullptr ? source_ref->As<Armor>() : nullptr; filter(armor))
            {
                armor_count = 1;
                view_data_.emplace_back(armor);
            }
            break;
        }
        case ArmorSource::Inventory: {
            ZoneScopedN("reset view data emplace inventory");
            if (source_ref == nullptr) break;
            const auto objects = source_ref->GetInventory() | std::views::keys;
            view_data_.reserve(objects.size());
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
            ZoneScopedN("reset view data emplace Carried");
            if (source_ref == nullptr) break;
            if (auto *changes = source_ref->GetInventoryChanges(); changes != nullptr)
            {
                ArmorItemVisitor visitor;
                changes->VisitWornItems(visitor);
                armor_count = visitor.armors.size();
                view_data_.reserve(armor_count);
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
            ZoneScopedN("reset view data emplace all");
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
    ZoneValue(view_data_.size());
    {
        ZoneScopedN("reset view data sort/shrink");
        view_data_.shrink_to_fit();
        std::ranges::sort(view_data_, ArmorNameLessComparator());
        armor_count = view_data_.size();
    }
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
    if (!is_armor_can_display(armor))
    {
        return false;
    }

    if (!mod_filterer_.pass_filter(armor))
    {
        return false;
    }
    if (!slot_filterer_.pass_filter(armor))
    {
        return false;
    }

    if (contain_non_playable_armor_ || util::IsArmorPlayable(armor))
    {
        if (!armor_name_filter_.PassFilter(armor))
        {
            return false;
        }
    }
    return true;
}

auto ArmorView::find(const Armor *armor) const -> const_iterator
{
    const ArmorEntry armorEntry(armor);
    return std::ranges::lower_bound(view_data_, armorEntry, ArmorNameLessComparator());
}
} // namespace SosGui
