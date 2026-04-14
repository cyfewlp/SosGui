//
// Created by jamie on 2025/4/15.
//

#pragma once

#include "imgui.h"

#include <vector>

namespace SosGui::widgets
{
bool DrawNearActorsCombo(const std::vector<RE::Actor *> &nearActors, RE::Actor **selectedActor, RE::Actor *defaultActor);
}
