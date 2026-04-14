//
// Created by jamie on 2025/4/29.
//

#include "util/utils.h"

namespace SosGui
{

auto util::GetFormModFileName(const RE::TESForm *form) -> std::string_view
{
    if (auto *modFile = form->GetFile(); modFile != nullptr)
    {
        return modFile->GetFilename();
    }
    return "";
}

} // namespace SosGui
