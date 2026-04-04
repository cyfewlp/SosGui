#pragma once

#include <RE/B/BSContainer.h>
#include <RE/I/InventoryChanges.h>
#include <RE/T/TESObjectARMO.h>
#include <functional>
#include <vector>

inline RE::InventoryChanges::IItemChangeVisitor::~IItemChangeVisitor() {}

namespace SosGui
{
struct ArmorItemVisitor final : RE::InventoryChanges::IItemChangeVisitor
{
    std::vector<RE::TESObjectARMO *> armors{};

    ~ArmorItemVisitor() override {}

    constexpr auto Visit(RE::InventoryEntryData *a_entryData) -> RE::BSContainer::ForEachResult override
    {
        if (const auto form = a_entryData->object; form && form->formType == RE::FormType::Armor)
        {
            if (auto armor = form->As<RE::TESObjectARMO>(); armor != nullptr)
            {
                armors.push_back(armor);
            }
        }
        return RE::BSContainer::ForEachResult::kContinue;
    }
};

enum class ArmorSource : std::uint8_t
{
    None,
    Armor,     ///< Source is an armor
    Inventory, ///< Source is from object inventory
    Carried,   ///< Source from carried.
    All        ///< generate armors from TESDataHandler
};
} // namespace SosGui
