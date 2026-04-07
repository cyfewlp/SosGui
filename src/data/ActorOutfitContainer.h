#pragma once

#include "data/id.h"

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

struct AutoSwitchOutfit
{
    AutoSwitch policy;
    OutfitId   outfit_id;
};

struct ActorOutfitContainer
{
    struct Entry
    {
        RE::FormID                    actor_id;
        OutfitId                      outfit_id; ///< active outfit id
        std::vector<AutoSwitchOutfit> auto_switch_outfits;
    };

    using Container      = std::vector<Entry>;
    using iterator       = Container::iterator;
    using const_iterator = Container::const_iterator;

    Container container;

    [[nodiscard]] constexpr auto find(RE::FormID actor_id) -> iterator
    {
        const auto it = std::ranges::lower_bound(container, actor_id, std::less<RE::FormID>(), [](const Entry &entry) { return entry.actor_id; });
        return it != container.end() && it->actor_id == actor_id ? it : container.end();
    }

    [[nodiscard]] constexpr auto find(RE::FormID actor_id) const -> const_iterator
    {
        const auto it = std::ranges::lower_bound(container, actor_id, std::less<RE::FormID>(), [](const Entry &entry) { return entry.actor_id; });
        return it != container.end() && it->actor_id == actor_id ? it : container.end();
    }

    [[nodiscard]] constexpr auto find_auto_switch_outfit(RE::FormID actor_id, AutoSwitch policy) -> std::optional<AutoSwitchOutfit *>
    {
        if (auto it = find(actor_id); it != end())
        {
            const auto it1 =
                std::ranges::find_if(it->auto_switch_outfits, [policy](const AutoSwitchOutfit &entry) { return entry.policy == policy; });
            if (it1 != it->auto_switch_outfits.end())
            {
                return &(*it1);
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] constexpr auto find_auto_switch_outfit(RE::FormID actor_id, AutoSwitch policy) const -> std::optional<const AutoSwitchOutfit *>
    {
        if (auto it = find(actor_id); it != end())
        {
            const auto it1 =
                std::ranges::find_if(it->auto_switch_outfits, [policy](const AutoSwitchOutfit &entry) { return entry.policy == policy; });
            if (it1 != it->auto_switch_outfits.end())
            {
                return &(*it1);
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] constexpr auto end() const -> const_iterator { return container.end(); }

    [[nodiscard]] constexpr auto exists(RE::FormID actorId) const -> bool { return find(actorId) != container.end(); }

    [[nodiscard]] constexpr auto IsActorOutfit(RE::FormID actorId, OutfitId outfitId) const -> bool
    {
        if (auto it = find(actorId); it != container.end())
        {
            return it->outfit_id == outfitId;
        }
        return false;
    }

    constexpr void set(RE::FormID actorId, OutfitId outfitId)
    {
        auto it = find(actorId);
        if (it != container.end())
        {
            it->outfit_id = outfitId;
        }
        else
        {
            container.emplace(it, actorId, outfitId);
        }
    }

    /**
     * @brief Sets the auto switch outfit for the given actor and policy.
     * NOTE: if the actor does not exist, this function will not create a new entry for the actor.
     * But if the auto-switch outfit not exist, it will be created.
     * @return the iterator to the actor entry, or end() if the actor was not found.
     */
    constexpr auto set_auto_switch_outfit(RE::FormID actor_id, AutoSwitch policy, OutfitId outfit_id) -> const_iterator
    {
        auto it = find(actor_id);
        if (it != end())
        {
            const auto it1 =
                std::ranges::find_if(it->auto_switch_outfits, [policy](const AutoSwitchOutfit &entry) { return entry.policy == policy; });
            if (it1 != it->auto_switch_outfits.end())
            {
                it1->outfit_id = outfit_id;
            }
            else
            {
                it->auto_switch_outfits.emplace_back(policy, outfit_id);
            }
        }
        return it;
    }
};
} // namespace SosGui
