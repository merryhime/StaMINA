// This file is part of the stamina project.
// Copyright (c) 2020 MerryMage
// SPDX-License-Identifier: 0BSD

#include <set>
#include <string>
#include "common/assert.hpp"
#include "common/common_types.hpp"
#include "common/string_util.hpp"
#include "smasm/lexer.hpp"

namespace stamina {

namespace {

bool is_letter(std::optional<char> c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool is_decimal_digit(std::optional<char> c) {
    return (c >= '0' && c <= '9');
}

bool is_identifier_char(std::optional<char> c) {
    return is_letter(c) || is_decimal_digit(c) || c == '.' || c == '_';
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

const std::set<std::string> mnemonics {
#define INSTRUCTION(mnemonic, ...) #mnemonic,
#define COMPAREINST(...)
#include "common/instructions.inc"
#undef INSTRUCTION
#undef COMPAREINST
};

const std::set<std::string> conds {
#define INSTRUCTION(...)
#define COMPAREINST(mnemonic, cond, ...) #cond,
#include "common/instructions.inc"
#undef INSTRUCTION
#undef COMPAREINST
};

}

Token Tokenizer::next_token() {
    while (is_whitespace(ch)) {
        // skip whitespace
        advance();
    }

    if (ch == ';') {
        // skip comment
        while (ch != '\n') {
            advance();
        }
    }

    pos = ch_pos;
    source_code.clear();

    if (ch == '\n') {
        next_ch();
        if (can_newline) {
            return make_token(Token::Type::NewLine);
        }
        return next_token();
    }

    if (ch == std::nullopt) {
        if (can_newline) {
            can_newline = false;
            return make_token(Token::Type::NewLine);
        }
        return make_token(Token::Type::EndOfFile);
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
    case '@':
        if (maybe_ch('@')) {
            return make_token(Token::Type::TokCat);
        }
        can_newline = true;
        return lex_directive();
    case ',':
        return make_token(Token::Type::Comma);
    case '(':
        return make_token(Token::Type::LParen);
    case ')':
        can_newline = true;
        return make_token(Token::Type::RParen);
    case '+':
        return make_token(Token::Type::Plus);
    case '-':
        return make_token(Token::Type::Minus);
    case '*':
        return make_token(Token::Type::Mul);
    case '/':
        return make_token(Token::Type::Div);
    case '%':
        return make_token(Token::Type::Mod);
    case '^':
        return make_token(Token::Type::Xor);
    case '<':
        if (maybe_ch('<')) {
            return make_token(Token::Type::ShLeft);
        }
        if (maybe_ch('=')) {
            return make_token(Token::Type::LessEqual);
        }
        return make_token(Token::Type::Less);
    case '>':
        if (maybe_ch('>')) {
            return make_token(Token::Type::ShRight);
        }
        if (maybe_ch('=')) {
            return make_token(Token::Type::GreaterEqual);
        }
        return make_token(Token::Type::Greater);
    case '=':
        if (maybe_ch('=')) {
            return make_token(Token::Type::Equal);
        }
        return make_token(Token::Type::Error, "Single equals sign is not a valid token");
    case '!':
        if (maybe_ch('=')) {
            return make_token(Token::Type::NotEqual);
        }
        return make_token(Token::Type::LogicNot);
    case '~':
        return make_token(Token::Type::BitNot);
    case '&':
        if (maybe_ch('&')) {
            return make_token(Token::Type::LogicAnd);
        }
        return make_token(Token::Type::BitAnd);
    case '|':
        if (maybe_ch('|')) {
            return make_token(Token::Type::LogicOr);
        }
        return make_token(Token::Type::BitOr);
    }

    if (is_decimal_digit(prev_ch)) {
        can_newline = true;
        return lex_numerical(prev_ch);
    }
    if (is_identifier_char(prev_ch)) {
        can_newline = true;
        return lex_identifier(prev_ch);
    }

    return make_token(Token::Type::Error, "Unknown character");
}

void Tokenizer::next_ch() {
    if (ch) {
        source_code += *ch;
    }
    advance();
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
            return make_token(Token::Type::Error, "invalid character in string");
        }
    }
    next_ch();
    return make_token(Token::Type::StringLit, str);
}

Token Tokenizer::lex_char() {
    s64 value;
    if (const auto c = lex_single_translated_char()) {
        value = *c;
    } else {
        return make_token(Token::Type::Error, "invalid character");
    }
    next_ch();
    if (ch != '\'') {
        return make_token(Token::Type::Error, "character literal can only contain single character");
    }
    next_ch();
    return make_token(Token::Type::NumericLit, value);
}

Token Tokenizer::lex_raw_string() {
    std::string str;
    while (ch != '`') {
        if (ch == std::nullopt) {
            return make_token(Token::Type::Error, "invalid end-of-file in raw string");
        }
        str += *ch;
        next_ch();
    }
    next_ch();
    return make_token(Token::Type::StringLit, str);
}

Token Tokenizer::lex_directive() {
    std::string ident;
    while (is_identifier_char(ch)) {
        ident += *ch;
        next_ch();
    }
    return make_token(Token::Type::Directive, ident);
}

Token Tokenizer::lex_identifier(char c) {
    const std::string ident = [c, this]{
        std::string ret{c};
        while (is_identifier_char(ch)) {
            ret += *ch;
            next_ch();
        }
        return ret;
    }();
    const std::string upper_ident = toupper(ident);

    if (mnemonics.count(upper_ident) > 0) {
        return make_token(Token::Type::Mnemonic, upper_ident);
    }

    if (upper_ident == "CMP" || upper_ident == "CMPI") {
        if (ch != '/') {
            return make_token(Token::Type::Error, ident + " must be followed by /");
        }
        next_ch();

        const std::string cond = [this]{
            std::string ret;
            while (is_letter(ch)) {
                ret += *ch;
                next_ch();
            }
            return ret;
        }();
        const std::string upper_cond = toupper(cond);

        if (conds.count(upper_cond) == 0) {
            return make_token(Token::Type::Error, ident + " must be followed by a valid condition, " + cond + " is not a valid condition");
        }

        return make_token(Token::Type::Mnemonic, upper_ident + '/' + upper_cond);
    }

    return make_token(Token::Type::Identifier, ident);
}

Token Tokenizer::lex_numerical(char c) {
    const auto numeric_fn = [this](s64 value, auto is_digit, s64 radix){
        while (is_digit(ch)) {
            value = value * radix + digit_value(*ch);
            next_ch();
            if (value < 0) {
                return make_token(Token::Type::Error, "number literal overflow");
            }
        }
        return make_token(Token::Type::NumericLit, value);
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

Token Tokenizer::make_token(Token::Type type, std::variant<std::monostate, std::string, s64> payload) {
    return Token{pos, type, payload, source_code};
}

StringTokenizer::StringTokenizer(std::string str) : str(str) {
    advance();
}

void StringTokenizer::advance() {
    if (ch == '\n') {
        ch_pos = ch_pos.next_line();
    } else {
        ch_pos = ch_pos.advance(1);
    }

    if (index >= str.size()) {
        ch = std::nullopt;
        return;
    }
    ch = str[index++];
}

}
