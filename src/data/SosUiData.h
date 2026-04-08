//
// Created by jamie on 2025/4/3.
//

#pragma once

#include "Cleanable.h"
#include "OutfitContainer.h"
#include "data/ActorOutfitContainer.h"
#include "data/SosUiOutfit.h"
#include "data/id.h"
#include "imguiex/ErrorNotifier.h"
#include "log.h"

#include <RE/A/Actor.h>
#include <RE/T/TESObjectARMO.h>
#include <coroutine>
#include <exception>
#include <mutex>
#include <queue>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace SosGui
{
struct SosUiData
{
    using Armor         = RE::TESObjectARMO;
    using BodySlot      = int32_t;
    using BodySlotArmor = std::pair<BodySlot, Armor *>;

    std::vector<RE::Actor *> near_actors;
    bool                     enabled           = false;
    bool                     quick_slot_enabled = false;
    ActorOutfitContainer     actor_outfit_container;
    OutfitContainer          outfit_container{};
};
} // namespace SosGui
