#include "data/SosUiOutfit.h"

#include "util/utils.h"

#include <RE/B/BGSBipedObjectForm.h>
#include <cstdint>

namespace SosGui
{

void SosUiOutfit::AddArmor(const Armor *armor)
{
    if (armor == nullptr)
    {
        return;
    }
    auto slot = armor->GetSlotMask();
    if (slot == Slot::kNone)
    {
        return;
    }
    for (uint32_t idx = 0; idx < m_armors.size(); ++idx)
    {
        if (armor->HasPartOf(util::ToSlot(idx)))
        {
            m_armors[idx] = armor;
        }
    }
    m_slotMask.set(slot.get());
}

void SosUiOutfit::RemoveArmor(const Armor *armor)
{
    if (armor == nullptr)
    {
        return;
    }
    auto slot = armor->GetSlotMask();
    if (slot == Slot::kNone)
    {
        return;
    }
    for (uint32_t idx = 0; idx < m_armors.size(); ++idx)
    {
        if (m_armors[idx] == armor)
        {
            m_armors[idx] = nullptr;
        }
    }
    m_slotMask.reset(slot.get());
}

auto SosUiOutfit::GetArmorAt(uint32_t slotPos) const -> const Armor *
{
    if (slotPos >= SLOT_COUNT)
    {
        return nullptr;
    }
    return m_armors.at(slotPos);
}

auto SosUiOutfit::HasArmor(const Armor *armor) const -> bool
{
    if (m_slotMask.none(armor->GetSlotMask().get()))
    {
        return false;
    }
    for (const auto &armor1 : m_armors)
    {
        if (armor == armor1)
        {
            return true;
        }
    }
    return false;
}

} // namespace SosGui
