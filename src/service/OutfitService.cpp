#include "service/OutfitService.h"

#include "SosDataType.h"
#include "SosNativeCaller.h"
#include "autoswitch/ActorPolicyContainer.h"
#include "data/SosUiData.h"
#include "data/SosUiOutfit.h"
#include "data/id.h"
#include "task.h"

#include <RE/A/Actor.h>
#include <RE/P/PlayerCharacter.h>
#include <RE/T/TESObjectARMO.h>
#include <RE/V/Variable.h>
#include <format>
#include <string>
#include <type_traits>
#include <vector>

namespace SosGui
{
auto OutfitService::CreateOutfit(std::string outfitName) const -> Task
{
    if (const Variable isAlreadyExist = co_await SosNativeCaller::IsOutfitExisting(std::string(outfitName));
        isAlreadyExist.IsBool() && isAlreadyExist.GetBool())
    {
        co_return;
    }
    co_await SosNativeCaller::CreateOutfit(std::string(outfitName));
    if (const Variable existVar = co_await SosNativeCaller::IsOutfitExisting(std::string(outfitName)); !existVar.IsBool() || !existVar.GetBool())
    {
        ErrorNotifier::GetInstance().Error("Can't get outfit list");
    }
    else
    {
        outfit_container_.Add(std::move(outfitName));
    }
}

auto OutfitService::CreateOutfitFromWorn(std::string outfitName) const -> Task
{
    std::string errorMessage("Can't create outfit from worn: ");
    if (auto *player = RE::PlayerCharacter::GetSingleton(); player != nullptr)
    {
        if (const Variable wornArmorsVar = co_await SosNativeCaller::GetWornItems(player); wornArmorsVar.IsObjectArray())
        {
            const auto                       array = wornArmorsVar.GetArray();
            std::vector<RE::TESObjectARMO *> armors;
            for (const auto *iter = array->begin(); iter != array->end(); ++iter)
            {
                const RE::BSScript::Variable var = *iter;
                armors.emplace_back(var.Unpack<RE::TESObjectARMO *>());
            }
            co_await SosNativeCaller::OverwriteOutfit(std::string(outfitName), armors);
            if (const Variable existVar = co_await SosNativeCaller::IsOutfitExisting(std::string(outfitName));
                existVar.IsBool() && existVar.GetBool())
            {
                outfit_container_.Add(std::move(outfitName));
                co_return;
            }
            errorMessage.append("create outfit fail.");
        }
        else
        {
            errorMessage.append("can't get player worn armors.");
        }
    }
    else
    {
        errorMessage.append("can't get player");
    }

    ErrorNotifier::GetInstance().Error(std::move(errorMessage));
}

auto OutfitService::GetOutfitList() const -> Task
{
    const RE::BSScript::Variable outfitListVar = co_await SosNativeCaller::GetOutfitList();
    if (!outfitListVar.IsLiteralArray())
    {
        ErrorNotifier::GetInstance().Error("Can't get outfit list");
        co_return;
    }
    const auto array = outfitListVar.GetArray();

    outfit_container_.get_all().clear();
    for (const auto *iter = array->begin(); iter != array->end(); ++iter)
    {
        const RE::BSScript::Variable var = *iter;
        outfit_container_.Add(var.Unpack<std::string>());
    }
}

auto OutfitService::GetAllFavoriteOutfits() const -> Task
{
    const RE::BSScript::Variable outfitListVar = co_await SosNativeCaller::GetOutfitList(true);
    if (!outfitListVar.IsLiteralArray())
    {
        ErrorNotifier::GetInstance().Error("Can't get outfit list");
        co_return;
    }
    const auto array = outfitListVar.GetArray();

    for (const auto *iter = array->begin(); iter != array->end(); ++iter)
    {
        const auto &outfitName = iter->Unpack<std::string>();
        if (auto it = outfit_container_.find(outfitName); it != outfit_container_.get_all().end())
        {
            it->SetFavorite(true);
        }
        else
        {
            ErrorNotifier::GetInstance().Error(std::format("Can't found outfit {}", outfitName));
        }
    }
}

auto OutfitService::SetOutfitIsFavorite(const OutfitId id, std::string outfitName, const bool isFavorite) const -> Task
{
    if (auto it = outfit_container_.find(id); it != outfit_container_.end())
    {
        it->SetFavorite(isFavorite);
        co_await SosNativeCaller::SetOutfitFavoriteStatus(std::string(outfitName), isFavorite);
    }
}

auto OutfitService::SetActorOutfit(RE::Actor *actor, const OutfitId id, std::string outfitName) const -> Task
{
    if (outfit_container_.exists(id))
    {
        m_uiData.GetActorOutfitMap().SetOutfit(actor, id);
        co_await SosNativeCaller::ActiveOutfit(actor, std::string(outfitName));
    }
}

auto OutfitService::GetActorOutfit(RE::Actor *actor) const -> Task
{
    const Variable outfitNameVar = co_await SosNativeCaller::GetSelectedOutfit(actor);
    if (!outfitNameVar.IsString())
    {
        ErrorNotifier::GetInstance().Error("Can't get actor's active outfit");
    }
    if (const auto it = outfit_container_.find(outfitNameVar.Unpack<std::string>()); it != outfit_container_.end())
    {
        m_uiData.GetActorOutfitMap().SetOutfit(actor, it->GetId());
    }
}

auto OutfitService::RenameOutfit(const OutfitId id, std::string outfitName, std::string newName) const -> Task
{
    if (auto it = outfit_container_.find(id); it != outfit_container_.end())
    {
        if (const auto successVar = co_await SosNativeCaller::RenameOutfit(std::move(outfitName), std::string(newName));
            !successVar.IsBool() || !successVar.GetBool())
        {
            ErrorNotifier::GetInstance().Error("Can't rename outfit");
            co_return;
        }

        it->SetName(std::move(newName));
    }
}

auto OutfitService::DeleteOutfit(const OutfitId id, std::string outfitName) const -> Task
{
    if (outfit_container_.erase(id))
    {
        co_await SosNativeCaller::DeleteOutfit(std::move(outfitName));
    }
}

auto OutfitService::AddArmor(const OutfitId id, std::string outfitName, const Armor *armor) const -> Task
{
    if (armor == nullptr)
    {
        co_return;
    }
    if (auto it = outfit_container_.find(id); it != outfit_container_.end())
    {
        it->AddArmor(armor);
        co_await SosNativeCaller::AddArmorToOutfit(std::move(outfitName), armor);
    }
}

auto OutfitService::DeleteConflictArmors(std::string outfitName, const Armor *armor) -> Task
{
    if (armor == nullptr)
    {
        co_return;
    }
    co_await SosNativeCaller::RemoveConflictingArmorsFrom(armor, std::move(outfitName));
    // We no need update UI data because we default directly override armor in slot
}

auto OutfitService::RemoveArmor(const OutfitId id, std::string outfitName, const Armor *armor) const -> Task
{
    if (armor == nullptr)
    {
        co_return;
    }
    if (auto it = outfit_container_.find(id); it != outfit_container_.end())
    {
        it->RemoveArmor(armor);
        co_await SosNativeCaller::RemoveArmorFromOutfit(std::move(outfitName), armor);
    }
}

auto OutfitService::GetOutfitArmors(const OutfitId id, std::string outfitName) const -> Task
{
    auto it = outfit_container_.find(id);
    if (it == outfit_container_.end())
    {
        co_return;
    }
    co_await SosNativeCaller::PrepOutfitBodySlotListing(std::move(outfitName));
    const Variable armorsVar = co_await SosNativeCaller::GetOutfitBodySlotListingArmorForms();
    if (!armorsVar.IsObjectArray())
    {
        ErrorNotifier::GetInstance().Error("Can't get outfit armors");
        co_return;
    }
    const auto array = armorsVar.GetArray();
    for (const auto *iter = array->begin(); iter != array->end(); ++iter)
    {
        const RE::BSScript::Variable var = *iter;
        it->AddArmor(var.Unpack<RE::TESObjectARMO *>());
    }
}

auto OutfitService::SetSlotPolicy(const OutfitId id, std::string outfitName, const uint32_t slotPos, const SlotPolicy policy) const -> Task
{
    if (auto it = outfit_container_.find(id); it != outfit_container_.end())
    {
        it->SetSlotPolicy(slotPos, SlotPolicyToUiString(policy));
        co_await SosNativeCaller::SetBodySlotPoliciesForOutfit(std::move(outfitName), slotPos, SlotPolicyToCode(policy));
    }
}

auto OutfitService::GetSlotPolicy(const OutfitId id, std::string outfitName) const -> Task
{
    auto it = outfit_container_.find(id);
    if (it == outfit_container_.end())
    {
        co_return;
    }
    const Variable variable = co_await SosNativeCaller::BodySlotPolicyNamesForOutfit(std::move(outfitName));
    if (!variable.IsLiteralArray())
    {
        ErrorNotifier::GetInstance().Error("Can't get outfit slot policies");
        co_return;
    }
    const auto array = variable.GetArray();
    if (array->size() < SosUiOutfit::SLOT_COUNT)
    {
        ErrorNotifier::GetInstance().Error("Invalid outfit slot policies: slot count incorrect.");
        co_return;
    }
    SlotType slotPos = 0;
    for (const auto *iter = array->begin(); iter != array->end(); ++iter, ++slotPos)
    {
        const RE::BSScript::Variable var = *iter;

        it->SetSlotPolicy(slotPos, std::string(var.GetString()));
    }
}

auto OutfitService::GetActorStateOutfit(RE::Actor *actor, uint32_t policyId) const -> Task
{
    const Variable outfitVar = co_await SosNativeCaller::GetStateOutfit(actor, std::move(policyId));
    if (!outfitVar.IsString())
    {
        ErrorNotifier::GetInstance().Error("Can't get actor state outfit");
        co_return;
    }

    if (auto it = outfit_container_.find(outfitVar.Unpack<std::string>()); it != outfit_container_.end())
    {
        m_uiData.GetAutoSwitchPolicyContainer().emplace(actor->GetFormID(), policyId, it->GetId());
    }
}

auto OutfitService::GetActorAllStateOutfit(RE::Actor *actor) const -> Task
{
    auto &view = m_uiData.GetAutoSwitchPolicyContainer();
    view.erase(actor->GetFormID());
    using Policy = AutoSwitch::Policy;
    for (uint32_t policyId = 0; policyId < static_cast<uint32_t>(Policy::Count); ++policyId)
    {
        Variable outfitVar = co_await SosNativeCaller::GetStateOutfit(actor, std::move(policyId));
        if (!outfitVar.IsString())
        {
            continue;
        }
        if (auto it = outfit_container_.find(outfitVar.Unpack<std::string>()); it != outfit_container_.end())
        {
            view.emplace(actor->GetFormID(), policyId, it->GetId());
        }
    }
}

auto OutfitService::SetActorStateOutfit(const RE::Actor *actor, uint32_t policyId, const OutfitId outfitId) const -> Task
{
    if (policyId >= static_cast<uint32_t>(AutoSwitch::Policy::Count))
    {
        ErrorNotifier::GetInstance().Error(std::format("Invalid outfit policy: {}", policyId));
        co_return;
    }
    auto &view = m_uiData.GetAutoSwitchPolicyContainer();

    if (auto it = outfit_container_.find(outfitId); it != outfit_container_.end())
    {
        co_await SosNativeCaller::SetStateOutfit(actor, std::move(policyId), it->GetName().c_str());
        view.emplace_or_replace(actor->GetFormID(), policyId, outfitId);
    }
    else
    {
        co_await SosNativeCaller::SetStateOutfit(actor, std::move(policyId), "");
        view.erase(actor->GetFormID(), policyId);
    }
}
} // namespace SosGui
