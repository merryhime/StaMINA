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
        EndOfFile,
        NewLine,
        Identifier,
        Mnemonic,
        Directive,
        StringLit,
        NumericLit,
        Comma,
        LParen,
        RParen,
        Plus,
        Minus,
        Mul,
        Div,
        Mod,
        Xor,
        ShLeft,
        LessEqual,
        Less,
        ShRight,
        GreaterEqual,
        Greater,
        Equal,
        NotEqual,
        Not,
        LogicAnd,
        BitAnd,
        LogicOr,
        BitOr,
    };

    Token(Position pos, Type type) : pos(pos), type(type) {}
    Token(Position pos, Type type, std::string payload) : pos(pos), type(type), payload(payload) {}
    Token(Position pos, Type type, s64 payload) : pos(pos), type(type), payload(payload) {}

    Position pos;
    Type type;
    std::variant<std::monostate, std::string, s64> payload;

    friend auto operator<=>(const Token&, const Token&) = default;
};

struct Tokenizer {
public:
    Token next_token();

protected:
    virtual void next_ch() = 0;
    std::optional<char> ch;
    Position ch_pos;

private:
    void skip_whitespace();
    bool maybe_ch(char check_ch);

    std::optional<char> lex_single_translated_char();
    Token lex_translated_string();
    Token lex_char();
    Token lex_raw_string();
    Token lex_directive();
    Token lex_identifier(char c);
    Token lex_numerical(char c);

    Position pos;
    bool can_newline = true;
};

struct StringTokenizer final : public Tokenizer {
public:
    explicit StringTokenizer(std::string str);

protected:
    void next_ch() override;

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
            case stamina::Token::Type::EndOfFile:
                return "EndOfFile";
            case stamina::Token::Type::NewLine:
                return "NewLine";
            case stamina::Token::Type::Identifier:
                return "Identifier";
            case stamina::Token::Type::Mnemonic:
                return "Mnemonic";
            case stamina::Token::Type::Directive:
                return "Directive";
            case stamina::Token::Type::StringLit:
                return "StringLit";
            case stamina::Token::Type::NumericLit:
                return "NumericLit";
            case stamina::Token::Type::Comma:
                return "Comma";
            case stamina::Token::Type::LParen:
                return "LParen";
            case stamina::Token::Type::RParen:
                return "RParen";
            case stamina::Token::Type::Plus:
                return "Plus";
            case stamina::Token::Type::Minus:
                return "Minus";
            case stamina::Token::Type::Mul:
                return "Mul";
            case stamina::Token::Type::Div:
                return "Div";
            case stamina::Token::Type::Mod:
                return "Mod";
            case stamina::Token::Type::Xor:
                return "Xor";
            case stamina::Token::Type::ShLeft:
                return "ShLeft";
            case stamina::Token::Type::LessEqual:
                return "LessEqual";
            case stamina::Token::Type::Less:
                return "Less";
            case stamina::Token::Type::ShRight:
                return "ShRight";
            case stamina::Token::Type::GreaterEqual:
                return "GreaterEqual";
            case stamina::Token::Type::Greater:
                return "Greater";
            case stamina::Token::Type::Equal:
                return "Equal";
            case stamina::Token::Type::NotEqual:
                return "NotEqual";
            case stamina::Token::Type::Not:
                return "Not";
            case stamina::Token::Type::LogicAnd:
                return "LogicAnd";
            case stamina::Token::Type::BitAnd:
                return "BitAnd";
            case stamina::Token::Type::LogicOr:
                return "LogicOr";
            case stamina::Token::Type::BitOr:
                return "BitOr";
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
                return fmt::format(" - {}", arg);
            } else if constexpr (std::is_same_v<T, std::string>) {
                return fmt::format(" - `{}`", arg);
            } else {
                return "";
            }
        }, t.payload);
        return format_to(ctx.out(), "{} - {}{}", t.pos, t.type, payload_str);
    }
};
