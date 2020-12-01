// This file is part of the stamina project.
// Copyright (c) 2020 MerryMage
// SPDX-License-Identifier: 0BSD

#pragma once

#include <fmt/format.h>

namespace stamina {

[[noreturn]] void Terminate(fmt::string_view msg, fmt::format_args args);

namespace detail {

template <typename... Ts>
[[noreturn]] void TerminateHelper(fmt::string_view msg, Ts... args) {
    Terminate(msg, fmt::make_format_args(args...));
}

} // namespace detail

} // namespace stamina

#if defined(NDEBUG)
    #if defined(__clang) || defined(__GNUC__)
        #define UNREACHABLE() __builtin_unreachable()
        #define ASSUME(expr) [&]{ if (!(expr)) __builtin_unreachable(); }()
    #elif defined(_MSC_VER)
        #define UNREACHABLE() __assume(0)
        #define ASSUME(expr) __assume(expr)
    #else
        #define UNREACHABLE() ASSERT_FALSE("Unreachable code!")
        #define ASSUME(expr)
    #endif
#else
    #define UNREACHABLE() ASSERT_FALSE("Unreachable code!")
    #define ASSUME(expr)
#endif

#define ASSERT(expr)                                                                        \
    [&]{                                                                                    \
        if (!(expr)) [[unlikely]] {                                                         \
            ::stamina::detail::TerminateHelper(#expr);                                      \
        }                                                                                   \
    }()
#define ASSERT_MSG(expr, ...)                                                               \
    [&]{                                                                                    \
        if (!(expr)) [[unlikely]] {                                                         \
            ::stamina::detail::TerminateHelper(#expr "\nMessage: " __VA_ARGS__);            \
        }                                                                                   \
    }()
#define ASSERT_FALSE(...) [[unlikely]] ::stamina::detail::TerminateHelper("false\nMessage: " __VA_ARGS__)

#if defined(NDEBUG)
    #define DEBUG_ASSERT(expr) ASSUME(expr)
    #define DEBUG_ASSERT_MSG(expr, ...) ASSUME(expr)
#else
    #define DEBUG_ASSERT(expr) ASSERT(expr)
    #define DEBUG_ASSERT_MSG(expr, ...) ASSERT_MSG(expr, __VA_ARGS__)
#endif
