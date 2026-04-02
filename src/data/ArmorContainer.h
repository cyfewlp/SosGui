#pragma once

#include "data/BaseContainer.h"
#include "util/StringUtil.h"

#include <RE/B/BSCoreTypes.h>
#include <RE/T/TESForm.h>
#include <RE/T/TESObjectARMO.h>

namespace SosGui
{
class ArmorContainer : BaseContainer
{
    using Armor = RE::TESObjectARMO;

    struct NameComparator
    {
        bool operator()(const Armor *lhs, const Armor *rhs) const { return util::StrEqual(lhs->GetName(), rhs->GetName()); }
    };

    std::vector<Armor *> m_container;

public:
    using size_type      = std::vector<Armor *>::size_type;
    using iterator       = std::vector<Armor *>::iterator;
    using const_iterator = std::vector<Armor *>::const_iterator;

    ArmorContainer()  = default;
    ~ArmorContainer() = default;

    void Insert(Armor *armor);

    // init container: sort all registered armor by name
    void Init();

    auto FindArmor(const char *armorName, RE::FormID formId) const -> const_iterator;

    /**
     * The formId be used ensure the expected armor
     * because some armor may have the same name.
     */
    constexpr auto GetRank(const char *armorName, RE::FormID formId) const -> size_t
    {
        return static_cast<size_t>(std::distance(m_container.begin(), FindArmor(armorName, formId)));
    }

    constexpr auto GetRank(const_iterator element) const -> size_t { return static_cast<size_t>(std::distance(m_container.begin(), element)); }

    constexpr void Clear() { m_container.clear(); }

    constexpr auto IsEmpty() const -> bool { return m_container.empty(); }

    constexpr auto Size() const -> size_t { return m_container.size(); }
};
} // namespace SosGui
