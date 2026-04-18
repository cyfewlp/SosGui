#pragma once

#include "SosDataType.h"

#include <RE/A/Actor.h>
#include <coroutine>
#include <string>
#include <vector>

namespace SosGui
{
class SosNativeCaller
{
    using Armor = RE::TESObjectARMO;

public:
    using Awaitable = RE::BSScript::IVirtualMachine::Awaitable;

    static auto ActorNearPC() -> Awaitable;
    static auto AddActor(RE::Actor *actor) -> Awaitable;
    static auto RemoveActor(RE::Actor *actor) -> Awaitable;
    static auto ListActor() -> Awaitable;

    static auto GetCarriedArmor(RE::Actor *actor) -> Awaitable;
    static auto GetWornItems(RE::Actor *actor) -> Awaitable;

    static auto CreateOutfit(std::string outfitName) -> Awaitable;
    static auto SetActorActiveOutfit(RE::Actor *actor, std::string outfitName) -> Awaitable;
    static auto GetSelectedOutfit(RE::Actor *actor) -> Awaitable;
    static auto RenameOutfit(std::string outfitName, std::string newName) -> Awaitable;
    static auto DeleteOutfit(std::string outfitName) -> Awaitable;
    static auto AddArmorToOutfit(std::string outfitName, const Armor *armor) -> Awaitable;
    static auto RemoveArmorFromOutfit(std::string outfitName, const Armor *armor) -> Awaitable;
    static auto RemoveConflictingArmorsFrom(const Armor *armor, std::string outfitName) -> Awaitable;
    static auto IsOutfitExisting(std::string outfitName) -> Awaitable;
    static auto GetOutfitList(bool favoritesOnly = false) -> Awaitable;
    static auto GetOutfitFavoriteStatus(std::string outfitName) -> Awaitable;
    static auto SetOutfitFavoriteStatus(std::string outfitName, bool favoritesOnly = false) -> Awaitable;
    static auto OverwriteOutfit(std::string outfitName, std::vector<Armor *> &armors) -> Awaitable;
    static auto PrepOutfitBodySlotListing(std::string outfitName) -> Awaitable;
    static auto GetOutfitBodySlotListingArmorForms() -> Awaitable;
    static auto GetOutfitNameMaxLength() -> Awaitable;
    static auto BodySlotPolicyNamesForOutfit(std::string outfitName) -> Awaitable;
    static auto SetBodySlotPoliciesForOutfit(std::string outfitName, uint32_t slotPos, std::string policyCode) -> Awaitable;
    static auto RefreshAllActorsAutoSwitchOutfit() -> Awaitable;
    static auto RefreshAllActorsActiveOutfit() -> Awaitable;

    // auto-switch
    static auto IsActorAutoSwitchEnabled(RE::Actor *actor) -> Awaitable;
    static auto SetActorAutoSwitchEnabled(const RE::Actor *actor, bool &enabled) -> Awaitable;
    static auto SetStateOutfit(const RE::Actor *actor, uint32_t location, std::string outfitName) -> Awaitable;
    static auto ClearStateOutfit(const RE::Actor *actor, uint32_t location) -> Awaitable;
    static auto GetStateOutfit(RE::Actor *actor, uint32_t location) -> Awaitable;

    // Enable SkyrimOutfitSystem?
    static auto Enable(bool &isEnable) -> Awaitable;
    static auto IsEnabled() -> Awaitable;
    static auto ImportSettings() -> Awaitable;
    static auto ExportSettings() -> Awaitable;

private:
    static auto GetVM() -> RE::BSTSmartPointer<RE::BSScript::IVirtualMachine>
    {
        auto *skyrimVm = RE::SkyrimVM::GetSingleton();
        return skyrimVm != nullptr ? skyrimVm->impl : nullptr;
    }

    static auto StaticCall(const RE::BSFixedString &a_fnName, RE::BSScript::IFunctionArguments *a_args = RE::MakeFunctionArguments()) -> Awaitable
    {
        if (const auto &vm = GetVM(); vm != nullptr)
        {
            return vm->ADispatchStaticCall(SOS_NATIVE_CLASS_NAME, a_fnName, a_args);
        }
        return {};
    }
};
} // namespace SosGui
