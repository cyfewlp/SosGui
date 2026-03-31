#pragma once

#include "data/id.h"
#include "imgui.h"

#include <RE/T/TESObjectARMO.h>
#include <string>

namespace SosGui::Popup
{

[[nodiscard]] auto DrawActionButtons() -> bool;

struct BasicPopup
{
    ImGuiPopupFlags popupFlags = 0;
};

void DrawSettingsPopup(std::string_view name);
void DrawAboutPopup(std::string_view name);

} // namespace SosGui::Popup
