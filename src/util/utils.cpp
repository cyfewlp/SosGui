//
// Created by jamie on 2025/4/29.
//

#include "util/utils.h"

#include "gui/UiSettings.h"

namespace SosGui
{
auto util::IsArmorCanDisplay(const RE::TESObjectARMO *armor) -> bool
{
    bool canDisplay = false;
    if (armor != nullptr)
    {
        if (!Settings::UiSettings::GetInstance()->includeTemplateArmor && armor->templateArmor != nullptr)
        {
            return canDisplay;
        }
        if (const std::string_view name = armor->GetName(); !name.empty())
        {
            canDisplay = true;
        }
    }
    return canDisplay;
}

auto util::GetFormModFileName(const RE::TESForm *form) -> std::string_view
{
    if (auto *modFile = form->GetFile(); modFile != nullptr)
    {
        return modFile->GetFilename();
    }
    return "";
}

} // namespace SosGui
