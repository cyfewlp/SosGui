//
// Created by jamie on 2025/5/7.
//

#pragma once

#include "SosDataType.h"
#include "data/ArmorContainer.h"
#include "data/ArmorGenerator.h"
#include "data/SosUiData.h"
#include "gui/widgets.h"
#include "util/ImGuiUtil.h"

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

class ArmorMultiSelection : public MultiSelection
{
    REX::EnumSet<Slot> slot_mask_ = Slot::kNone;

public:
    auto update_selected(const Armor *armor, const ImGuiID index) -> bool
    {
        const bool has = Contains(index);
        slot_mask_.set(has, armor->GetSlotMask().get());
        return has;
    }

    [[nodiscard]] constexpr auto is_select_slot(const Slot slot) const -> bool { return slot_mask_.all(slot); }

    void Clear()
    {
        MultiSelection::Clear();
        slot_mask_ = Slot::kNone;
    }
};
} // namespace armor_view

// A wrap class. Be used to cache armor rank(sort by name).
class RankedArmor;

class ArmorView final
{
public:
    using SlotCounter = std::array<uint16_t, SLOT_COUNT>;

    std::vector<RankedArmor>        view_data_{};
    ArmorContainer                  armor_container_{};
    SlotCounter                     slot_counter_;
    armor_view::ArmorNameFilter     armor_name_filter_;
    armor_view::ModFilterer         mod_filterer_;
    armor_view::SlotFilterer        slot_filterer_;
    armor_view::ArmorMultiSelection multi_selection_;
    bool                            contain_non_playable_armor_ = true;
    bool                            contain_template_armor_    = false;

    using const_iterator = std::vector<RankedArmor>::const_iterator;

    enum class error : uint8_t
    {
        unknown_armor,
        unassociated_armor,
        armor_already_exists,
        armor_not_exist,
    };

    ////////////////////////////////////////////////////////////////////
    // mod filterer -> slot-filterer -> armor-name filter
    void               init();
    void               on_refresh();
    void               clear();
    void               clear_view_data();
    [[nodiscard]] auto add_armor(const Armor *armor) -> std::expected<void, error>;
    void               add_armors_has_slot(ArmorGenerator *generator, const SosUiOutfit *editing_outfit, Slot slots);
    bool               remove_armor(const Armor *armor);
    void               remove_armors_has_slot(Slot slots);
    void               remove_armors_no_has_slots(Slot slots);
    void               reset_counter();
    void               reset_view_data(ArmorGenerator *generator);
    void               reset_view(ArmorGenerator *generator);
    bool               filter(const Armor *armor) const;
    void               filterer_enable_all_slots(bool enable_all, ArmorGenerator *generator, const SosUiOutfit *editing_outfit);
    void               filterer_select_slot(SlotType slotPos, bool select, ArmorGenerator *generator, const SosUiOutfit *editing_outfit);

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
    auto find_armor(const Armor *armor) const -> std::expected<const_iterator, error>;

public:
    [[nodiscard]] auto get_view_data() const -> const std::vector<RankedArmor> & { return view_data_; }

    auto swap_view_data(std::vector<RankedArmor> &other) -> void { view_data_.swap(other); }
};

class RankedArmor
{
    const Armor *armor;
    size_t       rank = std::numeric_limits<size_t>::max();

public:
    explicit RankedArmor(const Armor *const armor, const size_t rank) : armor(armor), rank(rank) {}

    [[nodiscard]] auto get_rank() const -> size_t { return rank; }

    [[nodiscard]] auto data() const -> const Armor * { return armor; }

    auto operator->() const -> const Armor * { return armor; }

    friend bool operator<(const RankedArmor &lhs, const RankedArmor &rhs) { return lhs.rank < rhs.rank; }
};

} // namespace SosGui

template <>
struct std::less<SosGui::RankedArmor>
{
    bool operator()(const SosGui::RankedArmor &lhs, const SosGui::RankedArmor &rhs) const noexcept { return lhs.get_rank() < rhs.get_rank(); }
};