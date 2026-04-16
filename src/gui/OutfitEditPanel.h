#pragma once

#include "data/ArmorSource.h"
#include "data/SosUiData.h"
#include "data/SosUiOutfit.h"
#include "gui/ArmorView.h"
#include "gui/OutfitListTable.h"
#include "popup/Popup.h"
#include "service/OutfitService.h"

#include <RE/B/BGSBipedObjectForm.h>
#include <RE/B/BipedObjects.h>
#include <RE/T/TESObjectARMO.h>
#include <string>

namespace SosGui
{

enum class MainMenuAction : std::uint8_t;

class OutfitEditPanel final
{
public:
    using Slot  = RE::BIPED_MODEL::BipedObjectSlot;
    using Armor = RE::TESObjectARMO;

    static constexpr int MAX_FILTER_ARMOR_NAME = 256;

public:
    explicit OutfitEditPanel(OutfitService &outfitService)
        : outfit_service_(&outfitService), last_editing_outfit_id_(std::numeric_limits<OutfitId>::max())
    {
        UpdateWindowTitle(outfit_list_table_.get_editing_outfit());
    }

    void toggle_showing() { showing_ = !showing_; }

    void on_refresh();

    void draw(const OutfitContainer &outfit_container);
    void draw_outfit(EditingOutfit &editingOutfit);

    void on_main_menu_action(MainMenuAction main_menu_action) { outfit_list_table_.on_main_menu_action(main_menu_action); }

private:
    void draw_filterers(const EditingOutfit &editingOutfit);
    void UpdateWindowTitle(const EditingOutfit &editingOutfit);

    void draw_outfit_armors(EditingOutfit &editingOutfit);
    void SlotPolicyCombo(EditingOutfit &editingOutfit, const uint32_t &slotIdx) const;

    void DrawArmorSourcesTabBar();
    void draw_armor_view(const EditingOutfit &editingOutfit);
    void draw_preview_armor_window(const Armor *to_preview_armor);
    void draw_armor_view_content(const EditingOutfit &editingOutfit);
    void draw_armor_view(const std::vector<ArmorEntry> &viewData, bool editing_invalid_outfit, const Armor *&to_preview_armor);
    void draw_armor_row(int row_index, const Armor *armor, bool editing_invalid_outfit, bool &want_add_armor, const Armor *&to_preview_armor);
    void draw_add_armors_popup(const EditingOutfit &outfit);
    void DrawArmorViewModNameFilterer();
    void DrawArmorViewSlotFilterer();

    void AddSelectArmors(const EditingOutfit &outfit);

    enum class ConflictSolution : std::uint8_t
    {
        none,
        Skip,         ///< skip current conflict armor
        SkipAll,      ///< skip all conflict armors
        Overwrite,    ///< (remove)overwrite old conflict armor
        OverwriteAll, ///< (remove)overwrite all old conflict armors
        Suspend,      ///< waiting user choose a solution
    };

    ArmorView                        armor_view_{};
    std::string                      window_title_;
    OutfitService                   *outfit_service_;
    OutfitListTable                  outfit_list_table_{};
    REX::EnumSet<Slot>               selected_armors_slot_mask_ = Slot::kNone;
    int                              waiting_add_armor_count_   = 0;
    OutfitId                         last_editing_outfit_id_;
    ConflictSolution                 conflict_solution_       = ConflictSolution::none;
    bool                             show_all_outfit_slots_   = false;
    bool                             should_refresh_view_     = true;
    bool                             armor_name_sort_ascend_  = true;
    ArmorSource                      armor_source_            = ArmorSource::None;
    bool                             show_no_conflict_armors_ = false;
    bool                             preview_armor_           = true;
    bool                             showing_                 = true;
    int                              view_item_count_         = -1;
    Armor                           *previewing_armor_        = nullptr;
    // be used to check is armor-view need to be reset.
    Slot                             last_outfit_slot_mask_   = Slot::kNone;
    // be used to restore armor-view slots filter when cancel checking 'show_no_conflict_armors'
    Slot                             last_selected_slot_mask_ = Slot::kNone;
    RE::TESObjectREFR               *armor_source_refr_       = nullptr;
    std::vector<RE::TESObjectREFR *> near_objects_;
};
} // namespace SosGui
