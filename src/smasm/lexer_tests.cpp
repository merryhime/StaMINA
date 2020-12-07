// This file is part of the stamina project.
// Copyright (c) 2020 MerryMage
// SPDX-License-Identifier: 0BSD

#include <string>
#include <vector>
#include <catch.hpp>
#include "common/common_types.hpp"
#include "smasm/lexer.hpp"

using namespace stamina;

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

TEST_CASE("tokenizer: cmp", "[smasm]") {
    StringTokenizer tok{"cmpi/eq r0, 34"};
    std::vector<Token> tokens;

    while (true) {
        const auto t = tok.next_token();
        if (t.type == Token::Type::EndOfFile) {
            break;
        }
        tokens.push_back(t);
    }

    std::vector<Token> expect{
        Token{Position{"(unknown)", 1, 1}, Token::Type::Mnemonic, "CMPI/EQ"},
        Token{Position{"(unknown)", 1, 9}, Token::Type::Identifier, "r0"},
        Token{Position{"(unknown)", 1, 11}, Token::Type::Comma},
        Token{Position{"(unknown)", 1, 13}, Token::Type::NumericLit, 34},
        Token{Position{"(unknown)", 1, 15}, Token::Type::NewLine},
    };

    REQUIRE(expect == tokens);
}

