#pragma once

#include <i18n/translator_manager.h>
#include <utility>

namespace SosGui::util
{
constexpr auto reverse_range(size_t start, size_t end, size_t itemCount) -> std::pair<size_t, size_t>
{
    start             = itemCount - start;
    const auto length = end - start;
    end               = start < length ? 0 : start - length;
    return {start, end};
}

constexpr auto IsArmorHasAnySlotOf(const RE::TESObjectARMO *armor, RE::BIPED_MODEL::BipedObjectSlot slot)
{
    return armor->bipedModelData.bipedObjectSlots.any(slot);
}

constexpr auto IsArmorNotHasSlotOf(const RE::TESObjectARMO *armor, RE::BIPED_MODEL::BipedObjectSlot slot)
{
    return armor->bipedModelData.bipedObjectSlots.none(slot);
}

constexpr auto IsArmorNonPlayable(const RE::TESObjectARMO *armor) -> bool
{
    return (armor->formFlags & RE::TESObjectARMO::RecordFlags::kNonPlayable) != 0;
}

constexpr auto IsArmorPlayable(const RE::TESObjectARMO *armor) -> bool
{
    return !IsArmorNonPlayable(armor);
}

auto GetFormModFileName(const RE::TESForm *form) -> std::string_view;

constexpr void RefreshActorArmor(RE::Actor *selectedActor)
{
    if (selectedActor != nullptr)
    {
        if (auto *currentProcess = selectedActor->GetActorRuntimeData().currentProcess; currentProcess != nullptr)
        {
            currentProcess->Set3DUpdateFlag(RE::RESET_3D_FLAGS::kModel);
            selectedActor->Update3DModel();
        }
    }
}

constexpr auto ToSlot(uint32_t slotPos) -> RE::BipedObjectSlot
{
    return slotPos >= RE::BIPED_OBJECT::kEditorTotal ? RE::BipedObjectSlot::kNone : static_cast<RE::BipedObjectSlot>(1 << slotPos);
}

constexpr auto ToSlot(const RE::BIPED_OBJECT equipIndex) -> RE::BipedObjectSlot
{
    if (equipIndex >= RE::BIPED_OBJECT::kEditorTotal)
    {
        return RE::BipedObjectSlot::kNone;
    }
    return static_cast<RE::BipedObjectSlot>(1 << equipIndex);
}

/// @brief Supports only one replacement translate key, and the template string must be "{}"
template <typename Arg>
constexpr auto TranslateEx(std::string_view key, Arg arg) -> std::string
{
    constexpr std::string_view placeholder = "{}";
    std::string                message(Translate(key));
    if (auto off = message.find(placeholder); off != std::string::npos)
    {
        message.replace(off, placeholder.size(), std::format("{}", arg));
    }
    return message;
}

} // namespace SosGui::util
