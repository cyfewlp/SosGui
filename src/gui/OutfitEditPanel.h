#pragma once

#include "BaseGui.h"
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
class OutfitEditPanel final : public BaseGui
{
public:
    using Slot  = RE::BIPED_MODEL::BipedObjectSlot;
    using Armor = RE::TESObjectARMO;

    static constexpr int MAX_FILTER_ARMOR_NAME = 256;

private:
    static auto IsArmorNonPlayable(const Armor *armor) -> bool { return (armor->formFlags & Armor::RecordFlags::kNonPlayable) != 0; }

public:
    explicit OutfitEditPanel(SosUiData &uiData, OutfitService &outfitService) : m_uiData(uiData), m_outfitService(outfitService)
    {
        last_editing_outfit_id_ = std::numeric_limits<OutfitId>::max();
        UpdateWindowTitle(outfit_list_table_.GetEditingOutfit());
    }

    void OnRefresh() override;
    void Cleanup() override;
    void Focus() override;

    void Draw();
    void DrawOutfitPanel(EditingOutfit &editingOutfit);

private:
    enum class Error
    {
        delete_armor_from_unknown_outfit_id,
        add_armor_to_unknown_outfit_id,
    };

    static void PushError(Error error);

    void draw_filterers();
    void UpdateWindowTitle(const EditingOutfit &editingOutfit);

    void DrawOutfitArmors(EditingOutfit &editingOutfit);
    void HighlightConflictSlot(Slot slot) const;
    void SlotPolicyCombo(EditingOutfit &editingOutfit, const uint32_t &slotIdx) const;

    using DrawArmorEntry = std::function<void(const Armor *armor, ImGuiID index)>;
    void DrawArmorSourcesTabBar();
    void DrawArmorView(const EditingOutfit &editingOutfit);
    void DrawArmorViewContent(const EditingOutfit &editingOutfit, const std::vector<ArmorEntry> &viewData);
    void DrawArmorViewTableContent(const std::vector<ArmorEntry> &viewData, const DrawArmorEntry &drawArmorEntry);
    void DrawArmorViewModNameFilterer();
    void DrawArmorViewSlotFilterer();

    void AddSelectArmors(OutfitId id);
    void DeleteArmor(OutfitId id, const Armor *armor);

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
    SosUiData                       &m_uiData;
    OutfitService                   &m_outfitService;
    OutfitListTable                  outfit_list_table_{};
    int                              waiting_add_armor_count_ = 0;
    OutfitId                         last_editing_outfit_id_;
    ConflictSolution                 conflict_solution_     = ConflictSolution::none;
    bool                             show_all_outfit_slots_ = false;
    bool                             should_refresh_view_   = true;
    ArmorSource                      armor_source_          = ArmorSource::None;
    RE::TESObjectREFR               *armor_source_refr_     = nullptr;
    std::vector<RE::TESObjectREFR *> near_objects_;
};
} // namespace SosGui
