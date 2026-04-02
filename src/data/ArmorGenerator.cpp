#include "data/ArmorGenerator.h"

#include "log.h"
#include "util/utils.h"

#include <functional>

namespace SosGui
{

namespace
{

auto CanDisplayArmorInventoryFilter(RE::TESBoundObject &object) -> bool
{
    const auto *armor = object.As<RE::TESObjectARMO>();
    return util::IsArmorCanDisplay(armor);
}
} // namespace

auto ArmorItemVisitor::Visit(RE::InventoryEntryData *a_entryData) -> RE::BSContainer::ForEachResult
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

void FormIdArmorGenerator::ForEach(std::function<void(RE::TESObjectARMO *armor)> &&action)
{
    if (const auto foundArmor = RE::TESForm::LookupByID<RE::TESObjectARMO>(armorFormId); foundArmor != nullptr)
    {
        action(foundArmor);
    }
}

void NearObjectsInventoryArmorGenerator::ForEach(std::function<void(RE::TESObjectARMO *armor)> &&action)
{
    if (wantVisitIndex >= nearObjects.size())
    {
        return;
    }
    for (const auto &entry : nearObjects[wantVisitIndex]->GetInventory(CanDisplayArmorInventoryFilter, false))
    {
        if (const auto armor = skyrim_cast<RE::TESObjectARMO *>(entry.first); armor != nullptr)
        {
            action(armor);
        }
    }
}

void NearObjectsInventoryArmorGenerator::Update()
{
    nearObjects.clear();
    const RE::PlayerCharacter *player = RE::PlayerCharacter::GetSingleton();
    const RE::TESObjectCELL   *cell   = player->GetParentCell();

    cell->ForEachReference([this](RE::TESObjectREFR *objectRef) {
        if (const std::string_view nameSv(objectRef->GetName()); nameSv.empty())
        {
            return RE::BSContainer::ForEachResult::kContinue;
        }
        if (!objectRef->GetInventory(CanDisplayArmorInventoryFilter, false).empty())
        {
            nearObjects.push_back(objectRef);
        }
        return RE::BSContainer::ForEachResult::kContinue;
    });
}

void InventoryArmorGenerator::ForEach(std::function<void(RE::TESObjectARMO *armor)> &&action)
{
    if (actor == nullptr)
    {
        return;
    }

    ArmorItemVisitor visitor;
    actor->GetInventoryChanges()->VisitInventory(visitor);

    for (const auto &armor : visitor.armors)
    {
        if (util::IsArmorCanDisplay(armor))
        {
            action(armor);
        }
    }
}

void CarriedArmorGenerator::ForEach(std::function<void(RE::TESObjectARMO *armor)> &&action)
{
    if (actor == nullptr)
    {
        return;
    }

    ArmorItemVisitor visitor;
    actor->GetInventoryChanges()->VisitWornItems(visitor);

    for (const auto &armor : visitor.armors)
    {
        if (util::IsArmorCanDisplay(armor))
        {
            action(armor);
        }
    }
}

void BasicArmorGenerator::ForEach(std::function<void(RE::TESObjectARMO *armor)> &&action)
{
    auto       *dataHandler = RE::TESDataHandler::GetSingleton();
    const auto &armorArray  = dataHandler->GetFormArray<RE::TESObjectARMO>();

    for (const auto &armor : armorArray)
    {
        if (util::IsArmorCanDisplay(armor))
        {
            action(armor);
        }
    }
}
} // namespace SosGui
