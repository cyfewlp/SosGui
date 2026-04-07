//
// Created by jamie on 2026/4/5.
//

#pragma once

#include "id.h"

#include <unordered_map>

namespace SosGui
{

enum class AutoSwitch : uint32_t
{
    Combat  = 12,
    World   = 0,
    Town    = 1,
    Dungeon = 2,
    City    = 9,

    WorldSnowy   = 3,
    TownSnowy    = 4,
    DungeonSnowy = 5,
    CitySnowy    = 10,

    WorldRainy   = 6,
    TownRainy    = 7,
    DungeonRainy = 8,
    CityRainy    = 11,
    Count        = 13,
    None         = 14
};

struct AutoSwitchEntry
{
    AutoSwitch policy;
    OutfitId   outfit_id;
};

struct ActorPolicyContainer
{
    using container_type = std::unordered_map<RE::FormID, std::vector<AutoSwitchEntry>>;
    using const_iterator = container_type::const_iterator;

    container_type actor_policies;

    constexpr void reserve(size_t size) { actor_policies.reserve(size); }

    auto emplace_back(RE::FormID actorId, AutoSwitch policy, OutfitId outfitId)
    {
        auto it = actor_policies.try_emplace(actorId, std::vector<AutoSwitchEntry>{});
        it.first->second.emplace_back(policy, outfitId);
    }

    constexpr auto erase_actor(RE::FormID actorId) -> size_t { return actor_policies.erase(actorId); }

    [[nodiscard]] constexpr auto find(RE::FormID actorId, AutoSwitch policy) -> std::optional<AutoSwitchEntry *>
    {
        const auto it = actor_policies.find(actorId);

        if (it != actor_policies.end())
        {
            const auto entryIt = std::ranges::find_if(it->second, [&](const AutoSwitchEntry &entry) { return entry.policy == policy; });
            if (entryIt != it->second.end())
            {
                return &(*entryIt);
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] constexpr auto find(RE::FormID actorId, AutoSwitch policy) const -> std::optional<const AutoSwitchEntry *>
    {
        const auto it = actor_policies.find(actorId);

        if (it != actor_policies.end())
        {
            const auto entryIt = std::ranges::find_if(it->second, [&](const AutoSwitchEntry &entry) { return entry.policy == policy; });
            if (entryIt != it->second.end())
            {
                return &(*entryIt);
            }
        }
        return std::nullopt;
    }
};

} // namespace SosGui