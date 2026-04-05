//
// Created by jamie on 2026/4/3.
//

#pragma once
#include "SosUiOutfit.h"
#include "util/StringUtil.h"

#include <algorithm>

namespace SosGui
{
class OutfitContainer
{
    static inline OutfitId g_NextOutfitId = 1;

    std::vector<SosUiOutfit> outfits_;

public:
    using iterator       = std::vector<SosUiOutfit>::iterator;
    using const_iterator = std::vector<SosUiOutfit>::const_iterator;

    constexpr void resize(size_t size) { outfits_.resize(size); }

    constexpr void reserve(size_t size) { outfits_.reserve(size); }

    constexpr void sort()
    {
        std::ranges::sort(outfits_, [](const SosUiOutfit &lhs, const SosUiOutfit &rhs) -> bool {
            return util::StrLess(lhs.GetName(), rhs.GetName());
        });
    }

    constexpr void Add(std::string outfitName)
    {
        outfits_.emplace_back(g_NextOutfitId, std::move(outfitName));
        ++g_NextOutfitId;
    }

    constexpr auto find(OutfitId id) const -> const_iterator
    {
        return std::ranges::find_if(outfits_, [id](const auto &outfit) -> bool {
            return outfit.GetId() == id;
        });
    }

    constexpr auto find(const std::string &name) const -> const_iterator
    {
        return std::lower_bound(outfits_.begin(), outfits_.end(), name, [](const SosUiOutfit &outfit, const std::string &a_name) -> bool {
            return util::StrLess(outfit.GetName(), a_name);
        });
    }

    constexpr auto find(OutfitId id) -> iterator
    {
        return std::ranges::find_if(outfits_, [id](const auto &outfit) -> bool {
            return outfit.GetId() == id;
        });
    }

    constexpr auto find(const std::string &name) -> iterator
    {
        return std::lower_bound(outfits_.begin(), outfits_.end(), name, [](const SosUiOutfit &outfit, const std::string &a_name) -> bool {
            return util::StrLess(outfit.GetName(), a_name);
        });
    }

    constexpr auto erase(OutfitId id) -> bool
    {
        if (auto it = find(id); it != outfits_.end())
        {
            outfits_.erase(it);
            return true;
        }
        return false;
    }

    constexpr auto erase(const_iterator it) -> bool
    {
        if (it != outfits_.end())
        {
            outfits_.erase(it);
            return true;
        }
        return false;
    }

    [[nodiscard]] constexpr auto exists(OutfitId id) const -> bool { return find(id) != outfits_.end(); }

    [[nodiscard]] constexpr auto end() const -> const_iterator { return outfits_.end(); }

    [[nodiscard]] constexpr auto get_all() -> std::vector<SosUiOutfit> & { return outfits_; }

    [[nodiscard]] constexpr auto get_all() const -> const std::vector<SosUiOutfit> & { return outfits_; }
};

} // namespace SosGui
