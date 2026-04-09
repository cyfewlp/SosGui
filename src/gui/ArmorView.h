//
// Created by jamie on 2025/5/7.
//

#pragma once

#include "SosDataType.h"
#include "data/ArmorSource.h"
#include "data/SosUiData.h"
#include "gui/widgets.h"
#include "util/ImGuiUtil.h"

#include <expected>

namespace SosGui
{

static constexpr uint32_t SLOT_COUNT = RE::BIPED_OBJECT::kEditorTotal;

namespace armor_view
{

struct ArmorNameFilter final : ImGuiUtil::DebounceInput
{
    bool PassFilter(const Armor *armor) const;

    bool Draw();
};

class SlotFilterer
{
    std::bitset<SLOT_COUNT> selected_slots_{};
    bool                    enable_all_slot_ = true; ///< default shows all armor slot

public:
    [[nodiscard]] auto pass_filter(const Armor *armor) const -> bool;

    [[nodiscard]] auto get_selected_slots() const -> Slot { return static_cast<Slot>(selected_slots_.to_ulong()); }

    [[nodiscard]] auto is_enable_all_slots() const -> bool { return enable_all_slot_; }

    [[nodiscard]] auto is_all_slots_enabled() const -> bool { return enable_all_slot_ || selected_slots_.all(); }

    [[nodiscard]] auto is_all_slots_disabled() const -> bool { return !enable_all_slot_ && selected_slots_.none(); }

    [[nodiscard]] auto is_slot_selected(SlotType slotPos) const -> bool { return selected_slots_.test(slotPos); }

    void select_slot(SlotType slotPos, bool select = true) { selected_slots_.set(slotPos, select); }

    void set_select_slots(Slot slots) { selected_slots_ = static_cast<uint32_t>(slots); }

    void enable_all_slots(bool check = true) { enable_all_slot_ = check; }

    void clear()
    {
        selected_slots_.reset();
        enable_all_slot_ = true;
    }
};

struct ModFilterer
{
    struct ModEntry
    {
        std::string_view name;
        uint32_t         count   = 0;
        bool             checked = false;
    };

    using ModEntryMap    = std::unordered_map<std::string_view, ModEntry>;
    using iterator       = ModEntryMap::iterator;
    using const_iterator = ModEntryMap::const_iterator;

    ModEntryMap mod_entry_map;

    bool PassFilter(const Armor *armor) const;

    void clear() { mod_entry_map.clear(); }

    constexpr auto try_emplace(std::string_view name, uint32_t count, bool checked)
    {
        return mod_entry_map.try_emplace(name, ModEntry(name, count, checked));
    }
};

} // namespace armor_view

/**
 * @brief Wrap `Armor`. Cache utf16 name to speed up sort and compare.
 */
struct ArmorEntry
{
    const Armor *armor;
    std::wstring name;

    explicit ArmorEntry(const Armor *armor) : armor(armor) { name = SKSE::stl::utf8_to_utf16(armor->GetName()).value_or(L""); }

    auto operator->() const -> const Armor * { return armor; }

    operator const Armor *() const { return armor; }
};

class ArmorView final
{
public:
    using SlotCounter    = std::array<uint16_t, SLOT_COUNT>;
    using const_iterator = std::vector<ArmorEntry>::const_iterator;

    std::vector<ArmorEntry>     view_data_{};
    SlotCounter                 slot_counter_;
    armor_view::ArmorNameFilter armor_name_filter_;
    armor_view::ModFilterer     mod_filterer_;
    armor_view::SlotFilterer    slot_filterer_;
    MultiSelection              multi_selection_;
    size_t                      armor_count; ///< Available armors count
    bool                        contain_non_playable_armor_ = true;
    bool                        contain_template_armor_     = false;

    enum class error : uint8_t
    {
        unknown_armor,
        unassociated_armor,
        armor_already_exists,
        armor_not_exist,
    };

    ////////////////////////////////////////////////////////////////////
    // mod filterer -> slot-filterer -> armor-name filter
    void               clear_all();
    void               clear_view_data();
    [[nodiscard]] auto add_armor(const Armor *armor) -> std::expected<void, error>;
    void               add_armors_for_slots(ArmorSource source, RE::TESObjectREFR *source_ref, Slot slots);
    bool               remove_armor(const Armor *armor);
    void               prune_view_by_slot_filter();
    void               reset_counter();
    void               reset_view_data(ArmorSource source, RE::TESObjectREFR *source_ref);
    void               reset_view(ArmorSource source, RE::TESObjectREFR *source_ref);
    bool               filter(const Armor *armor) const;
    void               set_enable_all_slots_filter(bool enable_all, ArmorSource source, RE::TESObjectREFR *source_ref);
    void               set_slot_filter(SlotType slotPos, bool select, ArmorSource source, RE::TESObjectREFR *source_ref);

    [[nodiscard]] auto get_armor_count(SlotType slotPos) const -> std::uint16_t { return slot_counter_.at(slotPos); }

    static auto to_error_message(const error error) -> std::string
    {
        switch (error)
        {
            case error::unknown_armor:
                return "Unknown armor.";
            case error::unassociated_armor:
                return "Unassociated Armor: Missing in container, try reopen menu to resolve.";
            case error::armor_already_exists:
                return "Armor already exists in view";
            case error::armor_not_exist:
                return "Armor not exist in view";
            default:
                return "Unknown error";
        }
    }

private:
    auto find(const Armor *armor) const -> const_iterator;

    void try_add_armor(const Armor *armor)
    {
        if (const auto expected = add_armor(armor); !expected && expected.error() != error::armor_already_exists)
        {
            logger::error("Can't add armor[{}]: {}", armor->formID, to_error_message(expected.error()));
        }
    }

    auto is_armor_can_display(const Armor *armor) const -> bool
    {
        bool result = false;
        if (armor != nullptr && !std::string_view(armor->GetName()).empty())
        {
            if (armor->templateArmor == nullptr || contain_template_armor_)
            {
                result = true;
            }
        }
        return result;
    }
};
} // namespace SosGui