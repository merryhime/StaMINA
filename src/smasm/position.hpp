// This file is part of the stamina project.
// Copyright (c) 2020 MerryMage
// SPDX-License-Identifier: 0BSD

#include <tuple>
#include <string>
#include <fmt/format.h>

namespace stamina {

struct Position final {
    std::string filename = "(unknown)";
    unsigned line = 1;
    unsigned column = 0;

    Position next_line() const {
        return Position {
            filename,
            line + 1,
            1,
        };
    }

    Position advance(unsigned num_char) const {
        return Position {
            filename,
            line,
            column + num_char,
        };
    }

    friend auto operator<=>(const Position&, const Position&) = default;
};

}

template <>
struct fmt::formatter<stamina::Position> {
    constexpr auto parse(format_parse_context& ctx) {
        return ctx.end();
    }

    template <typename FormatContext>
    auto format(const stamina::Position& p, FormatContext& ctx) {
        return format_to(ctx.out(), "{}:{}:{}", p.filename, p.line, p.column);
    }
};
