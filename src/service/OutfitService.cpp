#include "service/OutfitService.h"

#include "SosDataType.h"
#include "SosNativeCaller.h"
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
auto OutfitService::CreateOutfit(const std::string &outfitName) const -> Task
{
    if (const Variable isAlreadyExist = co_await SosNativeCaller::IsOutfitExisting(outfitName); isAlreadyExist.IsBool() && isAlreadyExist.GetBool())
    {
        co_return;
    }
    co_await SosNativeCaller::CreateOutfit(outfitName);
    if (const Variable existVar = co_await SosNativeCaller::IsOutfitExisting(outfitName); !existVar.IsBool() || !existVar.GetBool())
    {
        ErrorNotifier::GetInstance().Error("Can't get outfit list");
    }
    else
    {
        outfit_container_.try_emplace(outfitName);
    }
}

auto OutfitService::CreateOutfitFromWorn(const std::string &outfitName) const -> Task
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
            co_await SosNativeCaller::OverwriteOutfit(outfitName, armors);
            if (const Variable existVar = co_await SosNativeCaller::IsOutfitExisting(outfitName); existVar.IsBool() && existVar.GetBool())
            {
                outfit_container_.try_emplace(outfitName);
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
        outfit_container_.try_emplace(var.Unpack<std::string>());
    }
    outfit_container_.sort();
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

auto OutfitService::SetOutfitIsFavorite(const OutfitId id, const std::string &outfitName, const bool isFavorite) const -> Task
{
    if (auto it = outfit_container_.find(id); it != outfit_container_.end())
    {
        it->SetFavorite(isFavorite);
        co_await SosNativeCaller::SetOutfitFavoriteStatus(outfitName, isFavorite);
    }
}

auto OutfitService::SetActorOutfit(RE::Actor *actor, const OutfitId id, const std::string &outfitName) const -> Task
{
    if (outfit_container_.exists(id))
    {
        m_uiData.actor_outfit_container.set(actor, id);
        co_await SosNativeCaller::ActiveOutfit(actor, outfitName);
    }
}

auto OutfitService::GetActorOutfit(RE::Actor *actor) const -> Task
{
    const Variable outfitNameVar = co_await SosNativeCaller::GetSelectedOutfit(actor);
    if (!outfitNameVar.IsString())
    {
        ErrorNotifier::GetInstance().Error("Can't get actor's active outfit");
        co_return;
    }
    if (const auto it = outfit_container_.find(outfitNameVar.Unpack<std::string>()); it != outfit_container_.end())
    {
        m_uiData.actor_outfit_container.set(actor, it->GetId());
    }
}

auto OutfitService::RenameOutfit(const OutfitId id, const std::string &outfitName, const std::string &newName) const -> Task
{
    if (const auto successVar = co_await SosNativeCaller::RenameOutfit(outfitName, newName); !successVar.IsBool() || !successVar.GetBool())
    {
        ErrorNotifier::GetInstance().Error("Can't rename outfit");
        co_return;
    }

    if (auto it = outfit_container_.find(id); it != outfit_container_.end())
    {
        it->SetName(newName);
    }
}

auto OutfitService::DeleteOutfit(const OutfitId id, const std::string &outfitName) const -> Task
{
    co_await SosNativeCaller::DeleteOutfit(outfitName);
    outfit_container_.erase(id);
}

auto OutfitService::AddArmor(const OutfitId id, const std::string &outfitName, const Armor *armor) const -> Task
{
    if (armor == nullptr)
    {
        co_return;
    }
    if (auto it = outfit_container_.find(id); it != outfit_container_.end())
    {
        it->AddArmor(armor);
        co_await SosNativeCaller::AddArmorToOutfit(outfitName, armor);
    }
}

auto OutfitService::DeleteConflictArmors(const std::string &outfitName, const Armor *armor) -> Task
{
    if (armor == nullptr)
    {
        co_return;
    }
    co_await SosNativeCaller::RemoveConflictingArmorsFrom(armor, outfitName);
    // We don't need update UI data because we default directly override armor in slot
}

auto OutfitService::RemoveArmor(const OutfitId id, const std::string &outfitName, const Armor *armor) const -> Task
{
    if (armor == nullptr)
    {
        co_return;
    }
    if (auto it = outfit_container_.find(id); it != outfit_container_.end())
    {
        it->RemoveArmor(armor);
        co_await SosNativeCaller::RemoveArmorFromOutfit(outfitName, armor);
    }
}

auto OutfitService::GetOutfitArmors(const OutfitId id, const std::string &outfitName) const -> Task
{
    auto it = outfit_container_.find(id);
    if (it == outfit_container_.end())
    {
        co_return;
    }
    co_await SosNativeCaller::PrepOutfitBodySlotListing(outfitName);
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

auto OutfitService::SetSlotPolicy(EditingOutfit &editingOutfit, const uint32_t slotPos, const SlotPolicy policy) const -> Task
{
    if (outfit_container_.exists(editingOutfit.GetId()))
    {
        editingOutfit.slot_policies[slotPos] = policy;
        co_await SosNativeCaller::SetBodySlotPoliciesForOutfit(std::string(editingOutfit.GetName()), slotPos, slot_policy_code(policy));
    }
}

auto OutfitService::GetSlotPolicy(EditingOutfit &editingOutfit) const -> Task
{
    if (!outfit_container_.exists(editingOutfit.GetId()))
    {
        co_return;
    }
    const Variable variable = co_await SosNativeCaller::BodySlotPolicyNamesForOutfit(std::string(editingOutfit.GetName()));
    if (!variable.IsLiteralArray())
    {
        ErrorNotifier::GetInstance().Error("Can't get outfit slot policies");
        co_return;
    }
    const auto array = variable.GetArray();
    if (array == nullptr || array->size() < SosUiOutfit::SLOT_COUNT)
    {
        ErrorNotifier::GetInstance().Error("Invalid outfit slot policies: slot count incorrect.");
        co_return;
    }
    for (SlotType slotPos = 0; slotPos < RE::BIPED_OBJECT::kEditorTotal; ++slotPos)
    {
        const auto &var                      = array->operator[](slotPos);
        editingOutfit.slot_policies[slotPos] = slot_policy_from_string(var.GetString());
    }
}

auto OutfitService::GetActorStateOutfit(RE::Actor *actor, uint32_t policyId) const -> Task
{
    auto &actorOutfitContainer = m_uiData.actor_outfit_container;
    if (actor == nullptr || !actorOutfitContainer.exists(actor))
    {
        co_return;
    }

    const Variable outfitVar = co_await SosNativeCaller::GetStateOutfit(actor, std::move(policyId));
    if (!outfitVar.IsString())
    {
        ErrorNotifier::GetInstance().Error("Can't get actor state outfit");
        co_return;
    }

    if (auto it = outfit_container_.find(outfitVar.Unpack<std::string>()); it != outfit_container_.end())
    {
        actorOutfitContainer.set_auto_switch_outfit(actor, static_cast<AutoSwitch>(policyId), it->GetId());
    }
}

auto OutfitService::GetActorAllStateOutfit(RE::Actor *actor) const -> Task
{
    if (actor == nullptr)
    {
        co_return;
    }
    auto &actorOutfitContainer = m_uiData.actor_outfit_container;
    auto  it                   = actorOutfitContainer.find(actor);
    if (it == actorOutfitContainer.end())
    {
        co_return;
    }

    it->auto_switch_outfits.clear();
    for (uint32_t policyId = 0; policyId < static_cast<uint32_t>(AutoSwitch::Count); ++policyId)
    {
        Variable outfitVar = co_await SosNativeCaller::GetStateOutfit(actor, std::move(policyId));
        if (!outfitVar.IsString())
        {
            continue;
        }
        if (auto outfit_it = outfit_container_.find(outfitVar.Unpack<std::string>()); outfit_it != outfit_container_.end())
        {
            it->auto_switch_outfits.emplace_back(static_cast<AutoSwitch>(policyId), outfit_it->GetId());
        }
    }
}

auto OutfitService::SetActorStateOutfit(RE::Actor *actor, AutoSwitch policy, const OutfitId outfitId) const -> Task
{
    auto &actorOutfitContainer = m_uiData.actor_outfit_container;
    if (actor == nullptr || !actorOutfitContainer.exists(actor) || policy >= AutoSwitch::Count)
    {
        ErrorNotifier::GetInstance().Error(std::format("Invalid outfit policy: {}", static_cast<uint32_t>(policy)));
        co_return;
    }
    OutfitId newOutfitId = INVALID_OUTFIT_ID;
    if (auto it = outfit_container_.find(outfitId); it != outfit_container_.end())
    {
        co_await SosNativeCaller::SetStateOutfit(actor, static_cast<uint32_t>(policy), it->GetName().c_str());
        newOutfitId = outfitId;
    }
    actorOutfitContainer.set_auto_switch_outfit(actor, policy, newOutfitId);
}
} // namespace SosGui
