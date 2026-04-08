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
        RE::Actor                    *actor;
        OutfitId                      outfit_id; ///< active outfit id
        bool                          auto_switch_enabled;
        std::vector<AutoSwitchOutfit> auto_switch_outfits;
    };

    using Container      = std::vector<Entry>;
    using iterator       = Container::iterator;
    using const_iterator = Container::const_iterator;

    Container container;

    [[nodiscard]] constexpr auto find(const RE::Actor *actor) -> iterator
    {
        if (actor == nullptr) return container.end();

        const auto it = std::ranges::lower_bound(container, actor->formID, std::less<>(), [](const Entry &entry) { return entry.actor->formID; });
        return it != container.end() && it->actor->formID == actor->formID ? it : container.end();
    }

    [[nodiscard]] constexpr auto find(const RE::Actor *actor) const -> const_iterator
    {
        if (actor == nullptr) return container.end();

        const auto it = std::ranges::lower_bound(container, actor->formID, std::less<>(), [](const Entry &entry) { return entry.actor->formID; });
        return it != container.end() && it->actor->formID == actor->formID ? it : container.end();
    }

    [[nodiscard]] constexpr auto find_auto_switch_outfit(const RE::Actor *actor, AutoSwitch policy) const -> std::optional<const AutoSwitchOutfit *>
    {
        if (auto it = find(actor); it != end())
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

    [[nodiscard]] constexpr auto exists(const RE::Actor *actor) const -> bool { return find(actor) != container.end(); }

    [[nodiscard]] constexpr auto IsActorOutfit(const RE::Actor *actor, OutfitId outfitId) const -> bool
    {
        if (auto it = find(actor); it != container.end())
        {
            return it->outfit_id == outfitId;
        }
        return false;
    }

    [[nodiscard]] constexpr auto is_auto_switch_enabled(const RE::Actor *actor) const -> bool
    {
        auto it = find(actor);
        if (it != container.end())
        {
            return it->auto_switch_enabled;
        }
        return false;
    }

    constexpr void sort()
    {
        std::ranges::sort(container, [](const Entry &lhs, const Entry &rhs) { return lhs.actor->formID < rhs.actor->formID; });
    }

    constexpr void enable_auto_switch(RE::Actor *actor, bool enable)
    {
        auto it = find(actor);
        if (it != container.end())
        {
            it->auto_switch_enabled = enable;
        }
        else
        {
            container.emplace(it, actor, INVALID_OUTFIT_ID, enable);
        }
    }

    constexpr void add_actor(RE::Actor *actor)
    {
        if (auto it = find(actor); it == end())
        {
            container.emplace(it, actor);
        }
    }

    constexpr void remove_actor(RE::Actor *actor)
    {
        if (auto it = find(actor); it != end())
        {
            container.erase(it);
        }
    }

    constexpr void set(RE::Actor *actor, OutfitId outfitId)
    {
        auto it = find(actor);
        if (it != container.end())
        {
            it->outfit_id = outfitId;
        }
        else
        {
            container.emplace(it, actor, outfitId);
        }
    }

    constexpr void clear() { container.clear(); }

    /**
     * @brief Sets the auto switch outfit for the given actor and policy.
     * NOTE: if the actor does not exist, this function will not create a new entry for the actor.
     * But if the auto-switch outfit not exist, it will be created.
     * @return the iterator to the actor entry, or end() if the actor was not found.
     */
    constexpr auto set_auto_switch_outfit(const RE::Actor *actor, AutoSwitch policy, OutfitId outfit_id) -> const_iterator
    {
        auto it = find(actor);
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
