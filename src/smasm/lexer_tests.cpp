// This file is part of the stamina project.
// Copyright (c) 2020 MerryMage
// SPDX-License-Identifier: 0BSD

#include <string>
#include <vector>
#include <catch.hpp>
#include "common/common_types.hpp"
#include "smasm/lexer.hpp"

using namespace stamina;

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

TEST_CASE("tokenizer: Test 1", "[smasm]") {
    StringTokenizer tok{".def foo foo+3*4-a^b==1"};
    std::vector<Token> tokens;

    while (true) {
        const auto t = tok.next_token();
        if (t.type == Token::Type::EndOfFile) {
            break;
        }
        tokens.push_back(t);
    }

    std::vector<Token> expect{
        Token{Position{"(unknown)", 1, 1}, Token::Type::Directive, "def"},
        Token{Position{"(unknown)", 1, 6}, Token::Type::Identifier, "foo"},
        Token{Position{"(unknown)", 1, 10}, Token::Type::Identifier, "foo"},
        Token{Position{"(unknown)", 1, 13}, Token::Type::Plus},
        Token{Position{"(unknown)", 1, 14}, Token::Type::NumericLit, 3},
        Token{Position{"(unknown)", 1, 15}, Token::Type::Mul},
        Token{Position{"(unknown)", 1, 16}, Token::Type::NumericLit, 4},
        Token{Position{"(unknown)", 1, 17}, Token::Type::Minus},
        Token{Position{"(unknown)", 1, 18}, Token::Type::Identifier, "a"},
        Token{Position{"(unknown)", 1, 19}, Token::Type::Xor},
        Token{Position{"(unknown)", 1, 20}, Token::Type::Identifier, "b"},
        Token{Position{"(unknown)", 1, 21}, Token::Type::Equal},
        Token{Position{"(unknown)", 1, 23}, Token::Type::NumericLit, 1},
        Token{Position{"(unknown)", 1, 24}, Token::Type::NewLine},
    };

    REQUIRE(expect == tokens);
}
