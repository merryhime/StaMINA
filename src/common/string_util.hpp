// This file is part of the stamina project.
// Copyright (c) 2020 MerryMage
// SPDX-License-Identifier: 0BSD

#include <algorithm>
#include <cctype>
#include <string_view>

namespace stamina {

inline constexpr bool iequal(std::string_view a, std::string_view b) {
    return std::equal(a.begin(), a.end(), b.begin(), b.end(), [](const char& a, const char& b) { return std::tolower(a) == std::tolower(b); });
}

inline std::string toupper(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), [](char c) { return std::toupper(c); });
    return str;
}

}
