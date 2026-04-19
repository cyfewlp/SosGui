//
// Created by jamie on 2025/4/3.
//

#pragma once

#include "RE/B/BipedObjects.h"

#include <array>
#include <cstdint>
#include <string>

namespace SosGui
{
namespace SosFunction
{
// TODO: try RE::BSFixedString
static constexpr auto GetOutfitNameMaxLength                    = "GetOutfitNameMaxLength";
static constexpr auto GetCarriedArmor                           = "GetCarriedArmor";
static constexpr auto GetWornItems                              = "GetWornItems";
static constexpr auto RefreshArmorFor                           = "RefreshArmorFor";
static constexpr auto RefreshArmorForAllConfiguredActors        = "RefreshArmorForAllConfiguredActors";
static constexpr auto ActorNearPC                               = "ActorNearPC";
static constexpr auto PrepArmorSearch                           = "PrepArmorSearch";
static constexpr auto GetArmorSearchResultForms                 = "GetArmorSearchResultForms";
static constexpr auto GetArmorSearchResultNames                 = "GetArmorSearchResultNames";
static constexpr auto ClearArmorSearch                          = "ClearArmorSearch";
static constexpr auto PrepOutfitBodySlotListing                 = "PrepOutfitBodySlotListing";
static constexpr auto GetOutfitBodySlotListingArmorForms        = "GetOutfitBodySlotListingArmorForms";
static constexpr auto GetOutfitBodySlotListingArmorNames        = "GetOutfitBodySlotListingArmorNames";
static constexpr auto GetOutfitBodySlotListingSlotIndices       = "GetOutfitBodySlotListingSlotIndices";
static constexpr auto ClearOutfitBodySlotListing                = "ClearOutfitBodySlotListing";
static constexpr auto NaturalSort_ASCII                         = "NaturalSort_ASCII";
static constexpr auto NaturalSortPairArmor_ASCII                = "NaturalSortPairArmor_ASCII";
static constexpr auto HexToInt32                                = "HexToInt32";
static constexpr auto ToHex                                     = "ToHex";
static constexpr auto AddArmorToOutfit                          = "AddArmorToOutfit";
static constexpr auto ArmorConflictsWithOutfit                  = "ArmorConflictsWithOutfit";
static constexpr auto CreateOutfit                              = "CreateOutfit";
static constexpr auto DeleteOutfit                              = "DeleteOutfit";
static constexpr auto GetOutfitContents                         = "GetOutfitContents";
static constexpr auto GetOutfitFavoriteStatus                   = "GetOutfitFavoriteStatus";
static constexpr auto SetOutfitFavoriteStatus                   = "SetOutfitFavoriteStatus";
static constexpr auto BodySlotPolicyNamesForOutfit              = "BodySlotPolicyNamesForOutfit";
static constexpr auto SetBodySlotPoliciesForOutfit              = "SetBodySlotPoliciesForOutfit";
static constexpr auto SetAllBodySlotPoliciesForOutfit           = "SetAllBodySlotPoliciesForOutfit";
static constexpr auto SetBodySlotPolicyToDefaultForOutfit       = "SetBodySlotPolicyToDefaultForOutfit";
static constexpr auto GetAvailablePolicyNames                   = "GetAvailablePolicyNames";
static constexpr auto GetAvailablePolicyCodes                   = "GetAvailablePolicyCodes";
static constexpr auto GetSelectedOutfit                         = "GetSelectedOutfit";
static constexpr auto IsEnabled                                 = "IsEnabled";
static constexpr auto ListOutfits                               = "ListOutfits";
static constexpr auto RemoveArmorFromOutfit                     = "RemoveArmorFromOutfit";
static constexpr auto RemoveConflictingArmorsFrom               = "RemoveConflictingArmorsFrom";
static constexpr auto RenameOutfit                              = "RenameOutfit";
static constexpr auto OutfitExists                              = "OutfitExists";
static constexpr auto OverwriteOutfit                           = "OverwriteOutfit";
static constexpr auto SetEnabled                                = "SetEnabled";
static constexpr auto SetSelectedOutfit                         = "SetSelectedOutfit";
static constexpr auto AddActor                                  = "AddActor";
static constexpr auto RemoveActor                               = "RemoveActor";
static constexpr auto ListActors                                = "ListActors";
static constexpr auto SetLocationBasedAutoSwitchEnabled         = "SetLocationBasedAutoSwitchEnabled";
static constexpr auto GetLocationBasedAutoSwitchEnabled         = "GetLocationBasedAutoSwitchEnabled";
static constexpr auto GetAutoSwitchStateArray                   = "GetAutoSwitchStateArray";
static constexpr auto IdentifyStateType                         = "IdentifyStateType";
static constexpr auto SetOutfitUsingState                       = "SetOutfitUsingState";
static constexpr auto SetOutfitUsingStateForAllConfiguredActors = "SetOutfitUsingStateForAllConfiguredActors";
static constexpr auto SetStateOutfit                            = "SetStateOutfit";
static constexpr auto UnsetStateOutfit                          = "UnsetStateOutfit";
static constexpr auto GetStateOutfit                            = "GetStateOutfit";
static constexpr auto ExportSettings                            = "ExportSettings";
static constexpr auto ImportSettings                            = "ImportSettings";
static constexpr auto NotifyCombatStateChanged                  = "NotifyCombatStateChanged";
}; // namespace SosFunction

static constexpr std::array ARMOR_SLOT_NAMES = {
    "None",
    "Head",
    "Hair",
    "Body",
    "Hands",
    "Forearms",
    "Amulet",
    "Ring",
    "Feet",
    "Calves",
    "Shield",
    "Tail",
    "LongHair",
    "Circlet",
    "Ears",
    "ModMouth",
    "ModNeck",
    "ModChestPrimary",
    "ModBack",
    "ModMisc1",
    "ModPelvisPrimary",
    "DecapitateHead",
    "Decapitate",
    "ModPelvisSecondary",
    "ModLegRight",
    "ModLegLeft",
    "ModFaceJewelry",
    "ModChestSecondary",
    "ModShoulder",
    "ModArmLeft",
    "ModArmRight",
    "ModMisc2",
    "FX01",
};

static constexpr std::array SLOT_POLICY_NAMES = {
    "XXXX", // not available advanced: true
    "XXXE", // not available advanced: true
    "XXXO", //     available advanced: false  // name: Require equipped
    "XXOX", // not available advanced: true
    "XXOE", // not available advanced: true
    "XXOO", //     available advanced: false  // name: Always use outfit
    "XEXX", // not available advanced: true
    "XEXE", // not available advanced: true
    "XEXO", //     available advanced: false  // name: Passthrough
    "XEOX", // not available advanced: true
    "XEOE", // not available advanced: true
    "XEOO", // not available advanced: true
};

static constexpr std::array AVAILABLE_SLOT_POLICY = {
    "XXXO", //     available advanced: false  // name: Require equipped
    "XXOO", //     available advanced: false  // name: Always use outfit
    "XEXO", //     available advanced: false  // name: Passthrough
};

enum class SlotPolicy : uint8_t
{
    None,
    Inherit,
    RequireEquipped,
    AlwaysUseOutfit,
    Passthrough,
};

// ReSharper disable all
constexpr auto slot_policy_from_string(std::string_view str_policy) -> SlotPolicy
{
    SlotPolicy policy = SlotPolicy::None;
    if (str_policy == "$SkyOutSys_Desc_PolicyName_INHERIT")
    {
        policy = SlotPolicy::Inherit;
    }
    else if (str_policy == "$SkyOutSys_Desc_EasyPolicyName_XXXO")
    {
        policy = SlotPolicy::RequireEquipped;
    }
    else if (str_policy == "$SkyOutSys_Desc_EasyPolicyName_XXOO")
    {
        policy = SlotPolicy::AlwaysUseOutfit;
    }
    else if (str_policy == "$SkyOutSys_Desc_EasyPolicyName_XEXO")
    {
        policy = SlotPolicy::Passthrough;
    }
    return policy;
}

inline auto slot_policy_code(const SlotPolicy policy) -> std::string
{
    switch (policy)
    {
        case SlotPolicy::Inherit:
            return "";
        case SlotPolicy::RequireEquipped:
            return "XXXO";
        case SlotPolicy::AlwaysUseOutfit:
            return "XXOO";
        case SlotPolicy::Passthrough:
            return "XEXO";
        default:
            return "UNKNOWN";
    }
}

// ReSharper Restore all

constexpr auto slot_policy_token(SlotPolicy policy) -> const char *
{
    switch (policy)
    {
        case SlotPolicy::Inherit:
            return "Panels.OutfitEdit.SlotPolicy.Inherit";
        case SlotPolicy::RequireEquipped:
            return "Panels.OutfitEdit.SlotPolicy.RequireEquipped";
        case SlotPolicy::AlwaysUseOutfit:
            return "Panels.OutfitEdit.SlotPolicy.AlwaysUseOutfit";
        case SlotPolicy::Passthrough:
            return "Panels.OutfitEdit.SlotPolicy.Passthrough";
        default:
            return "Panels.OutfitEdit.SlotPolicy.Unknown";
    }
}

constexpr auto slot_policy_tooltip(SlotPolicy policy) -> const char *
{
    switch (policy)
    {
        case SlotPolicy::Inherit:
            return "Panels.OutfitEdit.SlotPolicy.InheritTooltip";
        case SlotPolicy::RequireEquipped:
            return "Panels.OutfitEdit.SlotPolicy.RequireEquippedTooltip";
        case SlotPolicy::AlwaysUseOutfit:
            return "Panels.OutfitEdit.SlotPolicy.AlwaysUseOutfitTooltip";
        case SlotPolicy::Passthrough:
            return "Panels.OutfitEdit.SlotPolicy.PassthroughTooltip";
        default:
            return "Panels.OutfitEdit.SlotPolicy.UnknownTooltip";
    }
}

constexpr std::string_view SOS_SPELL_EDITOR_ID   = "SkyrimOutfitSystemQuickslotSpell";
constexpr std::string_view SOS_NATIVE_CLASS_NAME = "SkyrimOutfitSystemNativeFuncs";
constexpr uint8_t          MAX_ERROR_COUNT       = 64;
constexpr uint8_t          MAX_ERROR_SHOW_COUNT  = 5;

using Slot     = RE::BIPED_MODEL::BipedObjectSlot;
using Armor    = RE::TESObjectARMO;
using SlotType = std::underlying_type_t<RE::BIPED_OBJECT>;
} // namespace SosGui
