#include "service/SosDataCoordinator.h"

#include "SosDataType.h"
#include "SosNativeCaller.h"
#include "data/SosUiData.h"
#include "service/OutfitService.h"

#include <vector>

namespace SosGui
{
auto SosDataCoordinator::RequestActorList() const -> Task
{
    const RE::BSScript::Variable actorListVar = co_await SosNativeCaller::ListActor();
    if (!actorListVar.IsObjectArray())
    {
        ErrorNotifier::GetInstance().Error("Can't get actor list");
        co_return;
    }

    const auto array = actorListVar.GetArray();
    ui_data_.actor_outfit_container.container.clear();
    for (const auto &var : *array)
    {
        auto *actor = var.Unpack<RE::Actor *>();

        const RE::BSScript::Variable isEnabledVar = co_await SosNativeCaller::IsActorAutoSwitchEnabled(actor);
        if (isEnabledVar.IsBool() && isEnabledVar.GetBool())
        {
            co_await outfit_service_.GetActorAllStateOutfit(actor);
        }
        co_await outfit_service_.GetActorOutfit(actor);
        co_await RequestUpdateActorAutoSwitchState(actor);
    }
}

auto SosDataCoordinator::RequestAddActor(RE::Actor *actor) const -> Task
{
    co_await SosNativeCaller::AddActor(actor);
    (void)ui_data_.actor_outfit_container.try_emplace(actor);
}

auto SosDataCoordinator::RequestRemoveActor(RE::Actor *actor) const -> Task
{
    co_await SosNativeCaller::RemoveActor(actor);
    ui_data_.actor_outfit_container.remove(actor);
}

auto SosDataCoordinator::RequestNearActorList() const -> Task
{
    const Variable actorsVar = co_await SosNativeCaller::ActorNearPC();
    if (!actorsVar.IsObjectArray())
    {
        ErrorNotifier::GetInstance().Error("Can't get near actor list");
        co_return;
    }

    const auto array = actorsVar.GetArray();
    ui_data_.near_actors.clear();
    for (const auto &var : *array)
    {
        ui_data_.near_actors.emplace_back(var.Unpack<RE::Actor *>());
    }
    std::ranges::sort(ui_data_.near_actors, std::less<>(), &RE::Actor::formID);
}

auto SosDataCoordinator::RequestUpdateActorAutoSwitchState(RE::Actor *actor) const -> Task
{
    const RE::BSScript::Variable isEnabledVar = co_await SosNativeCaller::IsActorAutoSwitchEnabled(actor);
    if (!isEnabledVar.IsBool())
    {
        ErrorNotifier::GetInstance().Error("Can't get actor auto-switch enabled state");
        co_return;
    }
    const auto isEnabled = isEnabledVar.GetBool();
    ui_data_.actor_outfit_container.enable_auto_switch(actor, isEnabled);
}

auto SosDataCoordinator::RequestSetActorAutoSwitchState(RE::Actor *actor, bool enabled) const -> Task
{
    co_await SosNativeCaller::SetActorAutoSwitchEnabled(actor, enabled);
    ui_data_.actor_outfit_container.enable_auto_switch(actor, enabled);
}

auto SosDataCoordinator::RequestImportSettings() const -> Task
{
    if (const auto successVar = co_await SosNativeCaller::ImportSettings(); successVar.IsBool() && successVar.GetBool())
    {
        co_await Refresh();
    }
}

auto SosDataCoordinator::RequestExportSettings() const -> Task
{
    if (const RE::BSScript::Variable successVar = co_await SosNativeCaller::ExportSettings(); !successVar.IsBool() || !successVar.GetBool())
    {
        ErrorNotifier::GetInstance().Error("Can't export settings");
        co_return;
    }
}

auto SosDataCoordinator::RequestEnable(bool enable) const -> Task
{
    co_await SosNativeCaller::Enable(enable);
    if (const RE::BSScript::Variable isEnabledVar = co_await SosNativeCaller::IsEnabled(); isEnabledVar.IsBool() && isEnabledVar.GetBool() != enable)
    {
        ErrorNotifier::GetInstance().Error("Can't set SkyrimOutfitSystem enabled state");
    }
    else
    {
        ui_data_.enabled = enable;
    }
}

auto SosDataCoordinator::QueryIsEnable() const -> Task
{
    const RE::BSScript::Variable isEnabledVar = co_await SosNativeCaller::IsEnabled();
    if (!isEnabledVar.IsBool())
    {
        ErrorNotifier::GetInstance().Error("Can't set SkyrimOutfitSystem enabled state");
        co_return;
    }
    ui_data_.enabled = isEnabledVar.GetBool();
}

auto SosDataCoordinator::Refresh() const -> Task
{
    logger::info("start refresh in thread: {}", std::this_thread::get_id());
    const auto        start = std::chrono::high_resolution_clock::now();
    std::vector<Task> tasks;
    tasks.emplace_back(outfit_service_.GetOutfitList());
    tasks.emplace_back(QueryIsEnable());
    tasks.emplace_back(RequestActorList());
    tasks.emplace_back(RequestNearActorList());
    co_await RequestUpdateActorAutoSwitchState(RE::PlayerCharacter::GetSingleton());
    tasks.emplace_back(outfit_service_.GetActorOutfit(RE::PlayerCharacter::GetSingleton()));
    tasks.emplace_back(outfit_service_.GetAllFavoriteOutfits());
    ui_data_.quick_slot_enabled = HasQuickSlotSpell();
    for (const auto &task : tasks)
    {
        co_await task;
    }

    auto end = std::chrono::high_resolution_clock::now();

    const auto nano = std::chrono::nanoseconds(end - start);

    logger::info("refresh spent: {}ns", nano.count());
}

auto SosDataCoordinator::HasQuickSlotSpell() -> bool
{
    const auto &player = RE::PlayerCharacter::GetSingleton();
    if (auto *spell = RE::TESForm::LookupByEditorID<RE::SpellItem>(SOS_SPELL_EDITOR_ID); spell != nullptr)
    {
        return player->HasSpell(spell);
    }
    return false;
}
} // namespace SosGui
