#include "data/ArmorContainer.h"

namespace SosGui
{
void ArmorContainer::Insert(Armor *armor)
{
    if (armor == nullptr)
    {
        return;
    }
    const auto it = std::ranges::lower_bound(m_container, armor, NameComparator());
    m_container.insert(it, armor);
}

void ArmorContainer::Init()
{
    Clear();
    auto *dataHandler = RE::TESDataHandler::GetSingleton();
    if (dataHandler == nullptr)
    {
        return;
    }
    const auto &armorArray = dataHandler->GetFormArray<RE::TESObjectARMO>();

    auto comparator = NameComparator();
    m_container.reserve(armorArray.size());
    for (const auto &armor : armorArray)
    {
        if (util::IsArmorCanDisplay(armor))
        {
            const auto it = std::ranges::lower_bound(m_container, armor, comparator);
            m_container.insert(it, armor);
        }
    }
}

auto ArmorContainer::FindArmor(const char *armorName, RE::FormID formId) const -> const_iterator
{
    if (armorName == nullptr)
    {
        return m_container.end();
    }
    auto foundIt = std::lower_bound(m_container.begin(), m_container.end(), armorName, [](const Armor *armor, const char *searchName) {
        return util::StrEqual(armor->GetName(), searchName);
    });

    if (foundIt == m_container.end() || (*foundIt)->formID == formId)
    {
        return foundIt;
    }
    while (++foundIt != m_container.end())
    {
        if ((*foundIt)->GetName() != armorName)
        {
            break;
        }
        if (formId == (*foundIt)->GetFormID())
        {
            break;
        }
    }

    return foundIt;
}
} // namespace SosGui
