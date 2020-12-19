// This file is part of the stamina project.
// Copyright (c) 2020 MerryMage
// SPDX-License-Identifier: 0BSD

#include <optional>
#include <string>
#include <variant>
#include <fmt/format.h>
#include "common/common_types.hpp"
#include "smasm/position.hpp"

namespace stamina {

struct Token final {
    enum class Type {
        Error,
        #define TOKEN(token) token,
        #include "token.inc"
        #undef TOKEN
    };

    Position pos;
    Type type;
    std::variant<std::monostate, std::string, s64> payload;
    std::string source_code;

    friend auto operator<=>(const Token&, const Token&) = default;
};

struct Tokenizer {
public:
    Token next_token();

protected:
    virtual void advance() = 0;
    std::optional<char> ch;
    Position ch_pos;

    void next_ch();
    std::string source_code;

private:
    bool maybe_ch(char check_ch);

    std::optional<char> lex_single_translated_char();
    Token lex_translated_string();
    Token lex_char();
    Token lex_raw_string();
    Token lex_directive();
    Token lex_identifier(char c);
    Token lex_numerical(char c);

    Token make_token(Token::Type type, std::variant<std::monostate, std::string, s64> payload = {});

    Position pos;
    bool can_newline = true;
};

struct StringTokenizer final : public Tokenizer {
public:
    explicit StringTokenizer(std::string str);

protected:
    void advance() override;

private:
    size_t index = 0;
    std::string str;
};

}

template <>
struct fmt::formatter<stamina::Token::Type> {
    constexpr auto parse(format_parse_context& ctx) {
        return ctx.end();
    }

    template <typename FormatContext>
    auto format(const stamina::Token::Type& t, FormatContext& ctx) {
        return format_to(ctx.out(), "{}", [t]{
            switch (t) {
            #define TOKEN(token) case stamina::Token::Type::token: return #token;
            #include "token.inc"
            #undef TOKEN
            default:
                return "Error";
            }
        }());
    }
};

template <>
struct fmt::formatter<stamina::Token> {
    constexpr auto parse(format_parse_context& ctx) {
        return ctx.end();
    }

    template <typename FormatContext>
    auto format(const stamina::Token& t, FormatContext& ctx) {
        const std::string payload_str = std::visit([](auto&& arg) -> std::string {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, s64>) {
                return fmt::format("{}", arg);
            } else if constexpr (std::is_same_v<T, std::string>) {
                return fmt::format("`{}`", arg);
            } else {
                return "(empty)";
            }
        }, t.payload);
        return format_to(ctx.out(), "{} - {} - {} - `{}`", t.pos, t.type, payload_str, t.source_code);
    }
};
