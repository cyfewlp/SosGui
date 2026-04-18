#pragma once

#include "SosDataType.h"
#include "data/SosUiData.h"
#include "data/id.h"
#include "task.h"

#include <RE/T/TESObjectARMO.h>
#include <RE/V/Variable.h>
#include <string>

namespace SosGui
{
class OutfitService
{
    using Armor    = RE::TESObjectARMO;
    using Variable = RE::BSScript::Variable;
    SosUiData       &m_uiData;
    OutfitContainer &outfit_container_;

public:
    OutfitService(SosUiData &uiData) : m_uiData(uiData), outfit_container_(m_uiData.outfit_container) {}

    auto CreateOutfit(std::string outfitName) const -> Task;

    auto CreateOutfitCopy(std::string outfitName, OutfitId src_outfit_id, const EditingOutfit::SlotPolicyArray slot_policies) const -> Task;

    auto CreateOutfitFromWorn(std::string outfitName) const -> Task;

    auto GetOutfitList() const -> Task;

    auto GetAllFavoriteOutfits() const -> Task;

    auto SetOutfitIsFavorite(OutfitId id, std::string outfitName, bool isFavorite) const -> Task;

    auto SetActorOutfit(RE::Actor *actor, OutfitId id, std::string outfitName) const -> Task;

    auto GetActorOutfit(RE::Actor *actor) const -> Task;

    auto RenameOutfit(OutfitId id, std::string outfitName, std::string newName) const -> Task;

    auto DeleteOutfit(OutfitId id, std::string outfitName) const -> Task;

    auto AddArmor(OutfitId id, std::string outfitName, const Armor *armor) const -> Task;

    static auto DeleteConflictArmors(std::string outfitName, const Armor *armor) -> Task;

    auto RemoveArmor(OutfitId id, std::string outfitName, const Armor *armor) const -> Task;

    auto GetOutfitArmors(OutfitId id, std::string outfitName) const -> Task;

    auto SetSlotPolicy(OutfitId id, std::string name, uint32_t slotPos, SlotPolicy policy) const -> Task;

    auto GetSlotPolicy(EditingOutfit &editingOutfit) const -> Task;

    auto GetActorStateOutfit(RE::Actor *actor, uint32_t policyId) const -> Task;

    auto GetActorAllStateOutfit(RE::Actor *actor) const -> Task;

    auto SetActorStateOutfit(RE::Actor *actor, AutoSwitch policy, OutfitId outfitId) const -> Task;
    auto ClearActorStateOutfit(RE::Actor *actor, AutoSwitch policy) const -> Task;

    static auto RefreshAllActorsAutoSwitchOutfit() -> Task;
    static auto RefreshAllActorsActiveOutfit() -> Task;
};
} // namespace SosGui
