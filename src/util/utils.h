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

constexpr bool IsArmorNonPlayable(const RE::TESObjectARMO *armor)
{
    return (armor->formFlags & RE::TESObjectARMO::RecordFlags::kNonPlayable) != 0;
}

constexpr bool IsArmorPlayable(const RE::TESObjectARMO *armor)
{
    return !IsArmorNonPlayable(armor);
}

auto IsArmorCanDisplay(const RE::TESObjectARMO *armor) -> bool;

auto GetFormModFileName(const RE::TESForm *form) -> std::string_view;

void RefreshActorArmor(RE::Actor *const selectedActor);

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
