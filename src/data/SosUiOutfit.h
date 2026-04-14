#pragma once

#include "SKSE/Impl/PCH.h"
#include "SosDataType.h"
#include "data/id.h"
#include "i18n/translator_manager.h"

#include <RE/B/BGSBipedObjectForm.h>
#include <RE/T/TESObjectARMO.h>
#include <array>
#include <string>
#include <unordered_set>
#include <utility>

namespace SosGui
{

static constexpr OutfitId UNTITLED_OUTFIT_ID = INVALID_OUTFIT_ID;
struct EditingOutfit;

class SosUiOutfit
{
public:
    static constexpr int SLOT_COUNT = RE::BIPED_OBJECT::kEditorTotal;
    using Slot                      = RE::BIPED_MODEL::BipedObjectSlot;
    using Armor                     = RE::TESObjectARMO;

private:
    std::array<const Armor *, SLOT_COUNT> m_armors{};
    std::string                           m_name       = "Untitled";
    REX::EnumSet<Slot, uint32_t>          m_slotMask   = Slot::kNone;
    OutfitId                              m_id         = UNTITLED_OUTFIT_ID;
    bool                                  m_isFavorite = false;
    friend struct EditingOutfit;

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

    void AddArmor(const Armor *armor);

    void RemoveArmor(const Armor *armor);

    void SetName(const std::string &newName) { m_name.assign(newName); }

    void SetFavorite(const bool isFavorite) { m_isFavorite = isFavorite; }
};

struct EditingOutfit
{
    using SlotPolicyArray = std::array<SlotPolicy, RE::BIPED_OBJECT::kEditorTotal>;

    const SosUiOutfit *source_outfit;
    SlotPolicyArray    slot_policies{};
    bool               invalid = false; ///< set to true if id is untitled or outfit is deleted.

    explicit EditingOutfit() : source_outfit(nullptr) {}

    explicit EditingOutfit(const SosUiOutfit &outfit) : source_outfit(&outfit) {}

    EditingOutfit(const EditingOutfit &other)                         = default;
    EditingOutfit(EditingOutfit &&other) noexcept                     = default;
    auto operator=(const EditingOutfit &other) -> EditingOutfit &     = default;
    auto operator=(EditingOutfit &&other) noexcept -> EditingOutfit & = default;

    auto operator=(const SosUiOutfit &outfit) -> EditingOutfit &
    {
        source_outfit = &outfit;
        slot_policies.fill(SlotPolicy::None);
        return *this;
    }

    [[nodiscard]] auto GetId() const -> OutfitId { return source_outfit == nullptr ? UNTITLED_OUTFIT_ID : source_outfit->GetId(); }

    [[nodiscard]] auto is_invalid() const -> bool { return invalid || GetId() == UNTITLED_OUTFIT_ID; }

    [[nodiscard]] auto get_name() const -> std::string_view
    {
        if (is_invalid()) return Translate("Panels.Outfit.Untitled");
        return source_outfit->GetName();
    }

    [[nodiscard]] auto get_name_str() const -> std::string
    {
        if (is_invalid()) return std::string(Translate("Panels.Outfit.Untitled"));
        return source_outfit->GetName();
    }

    [[nodiscard]] auto is_conflict_with(const RE::TESObjectARMO *armor) const -> bool
    {
        return source_outfit != nullptr && source_outfit->IsConflictWith(armor);
    }

    [[nodiscard]] auto IsEmpty() const -> bool { return source_outfit == nullptr || source_outfit->IsEmpty(); }

    [[nodiscard]] auto get_slot_mask() const -> Slot { return source_outfit == nullptr ? Slot::kNone : source_outfit->m_slotMask.get(); }

    [[nodiscard]] auto GetArmorAt(const uint32_t slotIdx) const -> const RE::TESObjectARMO *
    {
        return source_outfit != nullptr ? source_outfit->GetArmorAt(slotIdx) : nullptr;
    }

    operator bool() const { return source_outfit != nullptr; }
};
} // namespace SosGui
