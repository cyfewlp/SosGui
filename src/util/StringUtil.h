#pragma once

#include "log.h"
#include "tracy/Tracy.hpp"

#include <stringapiset.h>
#include <windows.h>

namespace SosGui::util
{

static auto UnicodeStringCompare(std::wstring_view wsView1, std::wstring_view wsView2) -> bool
{
    ZoneScopedN(__FUNCTION__);
    if (wsView1.empty() || wsView2.empty())
    {
        return false;
    }

    const int result = ::CompareStringEx(
        LOCALE_NAME_SYSTEM_DEFAULT,
        0,
        wsView1.data(),
        static_cast<int>(wsView1.size()),
        wsView2.data(),
        static_cast<int>(wsView2.size()),
        NULL,
        NULL,
        NULL
    );
    return result > 0 && result == CSTR_LESS_THAN;
}

static auto UnicodeStringCompare(const char *lhs, const char *rhs) -> bool
{
    if (lhs == nullptr || rhs == nullptr)
    {
        return false;
    }
    auto lUtf16 = SKSE::stl::utf8_to_utf16(lhs).value_or(L"");
    auto rUtf16 = SKSE::stl::utf8_to_utf16(rhs).value_or(L"");

    const int result = ::CompareStringEx(LOCALE_NAME_SYSTEM_DEFAULT, 0, lUtf16.c_str(), -1, rUtf16.c_str(), -1, NULL, NULL, NULL);
    return result > 0 && result == CSTR_LESS_THAN;
}

constexpr auto StringCompare(const std::string_view &lhs, const std::string_view &rhs) -> bool
{
    const int result =
        ::CompareStringA(LOCALE_INVARIANT, NORM_IGNORECASE, lhs.data(), static_cast<int>(lhs.size()), rhs.data(), static_cast<int>(rhs.size()));
    return result > 0 && result == CSTR_LESS_THAN;
}

struct StringCompactor
{
    auto operator()(const char *lhs, const char *rhs) const -> bool { return UnicodeStringCompare(lhs, rhs); }

    auto operator()(const std::string &lhs, const std::string &rhs) const -> bool { return UnicodeStringCompare(lhs.c_str(), rhs.c_str()); }

    auto operator()(const std::string_view &lhs, const std::string_view &rhs) const -> bool { return UnicodeStringCompare(lhs.data(), rhs.data()); }

    auto operator()(const std::wstring_view &lhs, const std::wstring_view &rhs) const -> bool { return UnicodeStringCompare(lhs.data(), rhs.data()); }
};

inline constexpr StringCompactor StrLess;

} // namespace SosGui::util
