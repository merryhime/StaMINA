// This file is part of the stamina project.
// Copyright (c) 2020 MerryMage
// SPDX-License-Identifier: 0BSD

#include "common/assert.hpp"
#include "common/common_types.hpp"
#include <fmt/format.h>
#include <optional>
#include <string>
#include <variant>

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
};

template <>
struct fmt::formatter<Position> {
    constexpr auto parse(format_parse_context& ctx) {
        return ctx.end();
    }

    template <typename FormatContext>
    auto format(const Position& p, FormatContext& ctx) {
        return format_to(ctx.out(), "{}:{}:{}", p.filename, p.line, p.column);
    }
};

struct Token final {
    enum class Type {
        Error,
        EndOfFile,
        NewLine,
        Identifier,
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
};

template <>
struct fmt::formatter<Token::Type> {
    constexpr auto parse(format_parse_context& ctx) {
        return ctx.end();
    }

    template <typename FormatContext>
    auto format(const Token::Type& t, FormatContext& ctx) {
        return format_to(ctx.out(), "{}", [t]{
            switch (t) {
            case Token::Type::EndOfFile:
                return "EndOfFile";
            case Token::Type::NewLine:
                return "NewLine";
            case Token::Type::Identifier:
                return "Identifier";
            case Token::Type::Directive:
                return "Directive";
            case Token::Type::StringLit:
                return "StringLit";
            case Token::Type::NumericLit:
                return "NumericLit";
            case Token::Type::Comma:
                return "Comma";
            case Token::Type::LParen:
                return "LParen";
            case Token::Type::RParen:
                return "RParen";
            case Token::Type::Plus:
                return "Plus";
            case Token::Type::Minus:
                return "Minus";
            case Token::Type::Mul:
                return "Mul";
            case Token::Type::Div:
                return "Div";
            case Token::Type::Mod:
                return "Mod";
            case Token::Type::Xor:
                return "Xor";
            case Token::Type::ShLeft:
                return "ShLeft";
            case Token::Type::LessEqual:
                return "LessEqual";
            case Token::Type::Less:
                return "Less";
            case Token::Type::ShRight:
                return "ShRight";
            case Token::Type::GreaterEqual:
                return "GreaterEqual";
            case Token::Type::Greater:
                return "Greater";
            case Token::Type::Equal:
                return "Equal";
            case Token::Type::NotEqual:
                return "NotEqual";
            case Token::Type::Not:
                return "Not";
            case Token::Type::LogicAnd:
                return "LogicAnd";
            case Token::Type::BitAnd:
                return "BitAnd";
            case Token::Type::LogicOr:
                return "LogicOr";
            case Token::Type::BitOr:
                return "BitOr";
            default:
                return "Error";
            }
        }());
    }
};

template <>
struct fmt::formatter<Token> {
    constexpr auto parse(format_parse_context& ctx) {
        if (ctx.begin() != ctx.end()) {
            throw format_error("invalid format");
        }
        return ctx.end();
    }

    template <typename FormatContext>
    auto format(const Token& t, FormatContext& ctx) {
        const std::string payload_str = std::visit([](auto&& arg) -> std::string {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, s64> || std::is_same_v<T, std::string>) {
                return fmt::format(" - {}", arg);
            } else {
                return "";
            }
        }, t.payload);
        return format_to(ctx.out(), "{} - {}{}", t.pos, t.type, payload_str);
    }
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

bool is_letter(std::optional<char> c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool is_decimal_digit(std::optional<char> c) {
    return (c >= '0' && c <= '9');
}

bool is_octal_digit(std::optional<char> c) {
    return (c >= '0' && c <= '7');
}

bool is_binary_digit(std::optional<char> c) {
    return (c >= '0' && c <= '1');
}

bool is_hex_digit(std::optional<char> c) {
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

int digit_value(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }
    UNREACHABLE();
}

bool is_whitespace(std::optional<char> c) {
    return c == 0x20 || c == 0x09 || c == 0x0D;
}

Token Tokenizer::next_token() {
    skip_whitespace();

    if (ch == ';') {
        // comment
        while (ch != '\n') {
            next_ch();
        }
    }

    pos = ch_pos;

    if (ch == '\n') {
        next_ch();
        if (can_newline) {
            return Token{pos, Token::Type::NewLine};
        }
        return next_token();
    }

    if (ch == std::nullopt) {
        if (can_newline) {
            can_newline = false;
            return Token{pos, Token::Type::NewLine};
        }
        return Token{pos, Token::Type::EndOfFile};
    }

    can_newline = false;

    const char prev_ch = *ch;
    next_ch();
    switch (prev_ch) {
    case '"':
        can_newline = true;
        return lex_translated_string();
    case '\'':
        can_newline = true;
        return lex_char();
    case '`':
        can_newline = true;
        return lex_raw_string();
    case '.':
        can_newline = true;
        return lex_directive();
    case ',':
        return Token{pos, Token::Type::Comma};
    case '(':
        return Token{pos, Token::Type::LParen};
    case ')':
        can_newline = true;
        return Token{pos, Token::Type::RParen};
    case '+':
        return Token{pos, Token::Type::Plus};
    case '-':
        return Token{pos, Token::Type::Minus};
    case '*':
        return Token{pos, Token::Type::Mul};
    case '/':
        return Token{pos, Token::Type::Div};
    case '%':
        return Token{pos, Token::Type::Mod};
    case '^':
        return Token{pos, Token::Type::Xor};
    case '<':
        if (maybe_ch('<')) {
            return Token{pos, Token::Type::ShLeft};
        }
        if (maybe_ch('=')) {
            return Token{pos, Token::Type::LessEqual};
        }
        return Token{pos, Token::Type::Less};
    case '>':
        if (maybe_ch('>')) {
            return Token{pos, Token::Type::ShRight};
        }
        if (maybe_ch('=')) {
            return Token{pos, Token::Type::GreaterEqual};
        }
        return Token{pos, Token::Type::Greater};
    case '=':
        if (maybe_ch('=')) {
            return Token{pos, Token::Type::Equal};
        }
        return Token{pos, Token::Type::Error, "Single equals sign is not a valid token"};
    case '!':
        if (maybe_ch('=')) {
            return Token{pos, Token::Type::NotEqual};
        }
        return Token{pos, Token::Type::Not};
    case '&':
        if (maybe_ch('&')) {
            return Token{pos, Token::Type::LogicAnd};
        }
        return Token{pos, Token::Type::BitAnd};
    case '|':
        if (maybe_ch('|')) {
            return Token{pos, Token::Type::LogicOr};
        }
        return Token{pos, Token::Type::BitOr};
    }

    if (is_letter(prev_ch)) {
        can_newline = true;
        return lex_identifier(prev_ch);
    }
    if (is_decimal_digit(prev_ch)) {
        can_newline = true;
        return lex_numerical(prev_ch);
    }

    return Token{pos, Token::Type::Error, "Unknown character"};
}

void Tokenizer::skip_whitespace() {
    while (is_whitespace(ch)) {
        next_ch();
    }
}

bool Tokenizer::maybe_ch(char check_ch) {
    if (ch == check_ch) {
        next_ch();
        return true;
    }
    return false;
}

std::optional<char> Tokenizer::lex_single_translated_char() {
    if (!maybe_ch('\\')) {
        const auto c = ch;
        next_ch();
        return c;
    }

    if (ch == std::nullopt) {
        return std::nullopt;
    }

    const char c = *ch;
    next_ch();
    switch (c) {
    case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7':
    {
        int value = digit_value(c);
        while (is_octal_digit(ch)) {
            value = value * 8 + digit_value(*ch);
            next_ch();
            if (value >= 256) {
                return std::nullopt;
            }
        }
        return (char)value;
    }
    case 'a':
        return 0x07;
    case 'b':
        return 0x08;
    case 'f':
        return 0x0C;
    case 'n':
        return 0x0A;
    case 'r':
        return 0x0D;
    case 't':
        return 0x09;
    case 'v':
        return 0x0B;
    case '\\':
        return 0x5C;
    case '\'':
        return 0x27;
    case '"':
        return 0x22;
    }
    return std::nullopt;
}

Token Tokenizer::lex_translated_string() {
    std::string str;
    while (ch != '"') {
        if (const auto c = lex_single_translated_char()) {
            str += *c;
        } else {
            return Token{pos, Token::Type::Error, "invalid character in string"};
        }
    }
    next_ch();
    return Token{pos, Token::Type::StringLit, str};
}

Token Tokenizer::lex_char() {
    s64 value;
    if (const auto c = lex_single_translated_char()) {
        value = *c;
    } else {
        return Token{pos, Token::Type::Error, "invalid character"};
    }
    next_ch();
    if (ch != '\'') {
        return Token{pos, Token::Type::Error, "character literal can only contain single character"};
    }
    next_ch();
    return Token{pos, Token::Type::NumericLit, value};
}

Token Tokenizer::lex_raw_string() {
    std::string str;
    while (ch != '`') {
        if (ch == std::nullopt) {
            return Token{pos, Token::Type::Error, "invalid end-of-file in raw string"};
        }
        str += *ch;
        next_ch();
    }
    next_ch();
    return Token{pos, Token::Type::StringLit, str};
}

Token Tokenizer::lex_directive() {
    std::string ident;
    while (is_letter(ch) || is_decimal_digit(ch)) {
        ident += *ch;
        next_ch();
    }
    return Token{pos, Token::Type::Directive, ident};
}

Token Tokenizer::lex_identifier(char c) {
    std::string ident{1, c};
    while (is_letter(ch) || is_decimal_digit(ch)) {
        ident += *ch;
        next_ch();
    }
    return Token{pos, Token::Type::Identifier, ident};
}

Token Tokenizer::lex_numerical(char c) {
    const auto numeric_fn = [this](s64 value, auto is_digit, s64 radix){
        while (is_digit(ch)) {
            value = value * radix + digit_value(*ch);
            next_ch();
            if (value < 0) {
                return Token{pos, Token::Type::Error, "number literal overflow"};
            }
        }
        return Token{pos, Token::Type::NumericLit, value};
    };

    if (c == '0') {
        if (maybe_ch('b') || maybe_ch('b')) {
            return numeric_fn(0, is_binary_digit, 2);
        }
        if (maybe_ch('o') || maybe_ch('O')) {
            return numeric_fn(0, is_octal_digit, 8);
        }
        if (maybe_ch('x') || maybe_ch('X')) {
            return numeric_fn(0, is_hex_digit, 16);
        }
    }
    return numeric_fn(digit_value(c), is_decimal_digit, 10);
}

struct StringTokenizer final : public Tokenizer {
public:
    explicit StringTokenizer(std::string str) : str(str) {
        next_ch();
    }

protected:
    void next_ch() override {
        if (ch == '\n') {
            ch_pos = ch_pos.next_line();
        } else {
            ch_pos = ch_pos.advance(1);
        }

        fmt::print("{}\n", *ch);

        if (index >= str.size()) {
            ch = std::nullopt;
            return;
        }
        ch = str[index++];
    }

private:
    size_t index = 0;
    std::string str;
};

int main() {
    StringTokenizer tok{".def foo foo+3*4-a^b==1"};

    while (true) {
        const auto t = tok.next_token();
        if (t.type == Token::Type::EndOfFile) {
            break;
        }
        fmt::print("{}\n", t);
    }

    return 0;
}
