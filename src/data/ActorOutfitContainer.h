#pragma once

#include "data/id.h"

#include <unordered_map>

namespace SosGui
{
struct ActorOutfitContainer
{
    struct Entry
    {
        RE::FormID actor_id;
        OutfitId   outfit_id;
    };

    using Container      = std::vector<Entry>;
    using iterator       = Container::iterator;
    using const_iterator = Container::const_iterator;

    Container container;

    [[nodiscard]] constexpr auto find(RE::FormID actor_id) -> iterator
    {
        return std::ranges::lower_bound(container, actor_id, std::less<RE::FormID>(), [](const Entry &entry) {
            return entry.actor_id;
        });
    }

    [[nodiscard]] constexpr auto find(RE::FormID actor_id) const -> const_iterator
    {
        return std::ranges::lower_bound(container, actor_id, std::less<RE::FormID>(), [](const Entry &entry) {
            return entry.actor_id;
        });
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
};
} // namespace SosGui
