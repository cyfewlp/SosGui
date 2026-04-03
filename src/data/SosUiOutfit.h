#pragma once

#include "SKSE/Impl/PCH.h"
#include "Translation.h"
#include "data/id.h"

#include <RE/B/BGSBipedObjectForm.h>
#include <RE/T/TESObjectARMO.h>
#include <array>
#include <string>
#include <unordered_set>
#include <utility>

namespace SosGui
{

static constexpr OutfitId UNTITLED_OUTFIT_ID = INVALID_OUTFIT_ID;

class SosUiOutfit
{
public:
    static constexpr int SLOT_COUNT = RE::BIPED_OBJECT::kEditorTotal;
    using Slot                      = RE::BIPED_MODEL::BipedObjectSlot;
    using Armor                     = RE::TESObjectARMO;
    using SlotPolicyArray           = std::array<std::string, SLOT_COUNT>;

private:
    SlotPolicyArray                       m_slotPolicies;
    std::array<const Armor *, SLOT_COUNT> m_armors{};
    std::string                           m_name       = "Untitled";
    REX::EnumSet<Slot, uint32_t>          m_slotMask   = Slot::kNone;
    OutfitId                              m_id         = UNTITLED_OUTFIT_ID;
    bool                                  m_isFavorite = false;

public:
    explicit constexpr SosUiOutfit() { m_armors.fill(nullptr); }

    explicit constexpr SosUiOutfit(const OutfitId id, std::string name, const bool favorite = false)
        : m_name(std::move(name)), m_id(id), m_isFavorite(favorite)
    {
        m_armors.fill(nullptr);
    }

    [[nodiscard]] constexpr auto GetId() const -> OutfitId { return m_id; }

    [[nodiscard]] auto GetArmorAt(uint32_t slotPos) const -> const Armor *;

    [[nodiscard]] auto HasArmor(const Armor *armor) const -> bool;

    [[nodiscard]] auto HasSlot(uint32_t slotPos) const -> bool { return m_slotMask.all(static_cast<Slot>(1 << slotPos)); }

    [[nodiscard]] auto IsConflictWith(const Armor *armor) const -> bool { return m_slotMask.any(armor->GetSlotMask().get()); }

    [[nodiscard]] constexpr auto IsFavorite() const -> bool { return m_isFavorite; }

    [[nodiscard]] auto GetName() const -> const std::string & { return m_name; }

    [[nodiscard]] auto IsEmpty() const -> bool { return m_slotMask.underlying() == 0; }

    [[nodiscard]] auto GetSlotPolicies() const -> const SlotPolicyArray & { return m_slotPolicies; }

    [[nodiscard]] auto GetUniqueArmors() const -> std::unordered_set<const Armor *>;

    void AddArmor(const Armor *armor);

    void RemoveArmor(const Armor *armor);

    void SetName(const std::string &newName) { m_name.assign(newName); }

    void SetFavorite(const bool isFavorite) { m_isFavorite = isFavorite; }

    void SetSlotPolicy(const uint32_t slotPos, const std::string &policy) { m_slotPolicies.at(slotPos) = policy; }
};

class EditingOutfit
{
    const SosUiOutfit *m_sourceOutfit;

public:
    explicit EditingOutfit(const SosUiOutfit &outfit) : m_sourceOutfit(&outfit) {}

    [[nodiscard]] auto GetSourceOutfit() const -> const SosUiOutfit * { return m_sourceOutfit; }

    [[nodiscard]] auto GetId() const -> OutfitId { return m_sourceOutfit->GetId(); }

    [[nodiscard]] auto IsUntitled() const -> bool { return GetId() == UNTITLED_OUTFIT_ID; }

    [[nodiscard]] auto GetName() const
    {
        if ((m_sourceOutfit == nullptr) || IsUntitled()) return Translation::Translate("$SosGui_Untitled");
        return m_sourceOutfit->GetName();
    }

    [[nodiscard]] auto IsConflictWith(const RE::TESObjectARMO *armor) const -> bool { return m_sourceOutfit->IsConflictWith(armor); }

    [[nodiscard]] auto IsEmpty() const -> bool { return m_sourceOutfit == nullptr || m_sourceOutfit->IsEmpty(); }

    [[nodiscard]] auto GetArmorAt(const uint32_t slotIdx) const -> const RE::TESObjectARMO * { return m_sourceOutfit->GetArmorAt(slotIdx); }

    [[nodiscard]] auto GetSlotPolicies() const -> const SosUiOutfit::SlotPolicyArray & { return m_sourceOutfit->GetSlotPolicies(); }

    auto operator->() const -> const SosUiOutfit * { return m_sourceOutfit; }
};

constexpr auto GetOutfitId(const SosUiOutfit &outfit)
{
    return outfit.GetId();
}

constexpr auto GetOutfitName(const SosUiOutfit &outfit)
{
    return outfit.GetName();
}
} // namespace SosGui
