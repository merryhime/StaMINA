// This file is part of the stamina project.
// Copyright (c) 2020 MerryMage
// SPDX-License-Identifier: 0BSD

#include <cstdio>
#include <exception>

#include <fmt/format.h>

#include "common/assert.hpp"

namespace stamina {

[[noreturn]] void Terminate(fmt::string_view msg, fmt::format_args args) {
    fmt::print(stderr, "stamina assertion failed: ");
    fmt::vprint(stderr, msg, args);
    std::terminate();
}

} // namespace stamina
