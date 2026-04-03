#pragma once

#include "log.h"

#include <stringapiset.h>
#include <windows.h>

namespace SosGui::util
{

static auto UnicodeStringCompare(const wchar_t *lhs, const wchar_t *rhs) -> bool
{
    if (lhs == nullptr || rhs == nullptr)
    {
        return false;
    }

    const int result = ::CompareStringEx(LOCALE_NAME_SYSTEM_DEFAULT, 0, lhs, -1, rhs, -1, NULL, NULL, NULL);
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

struct StringCompactor
{
    auto operator()(const char *lhs, const char *rhs) const -> bool { return UnicodeStringCompare(lhs, rhs); }

    auto operator()(const std::string &lhs, const std::string &rhs) const -> bool { return UnicodeStringCompare(lhs.c_str(), rhs.c_str()); }

    auto operator()(const std::wstring_view &lhs, const std::wstring_view &rhs) const -> bool { return UnicodeStringCompare(lhs.data(), rhs.data()); }
};

inline constexpr StringCompactor StrLess;

} // namespace SosGui::util
