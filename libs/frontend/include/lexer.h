//
// Created by LENOVO on 2024/9/20.
//

#ifndef LEXER_H
#define LEXER_H

#include <unordered_map>
#include <vector>

#include "error.h"
#include "token.h"

class Lexer {
    std::vector<Token>& tokens;
    std::istream& input;
    std::vector<Error>& errors;
    int line = 1;

public:
    Lexer(std::vector<Token>& t, std::istream& in, std::vector<Error>& e):
    tokens(t), input(in), errors(e) {};

    void nextToken();
};

const std::unordered_map<std::string, TokenType> keywordsMap = {
    {"main", TokenType::MAINTK},
    {"const", TokenType::CONSTTK},
    {"int", TokenType::INTTK},
    {"char", TokenType::CHARTK},
    {"break", TokenType::BREAKTK},
    {"continue", TokenType::CONTINUETK},
    {"if", TokenType::IFTK},
    {"else", TokenType::ELSETK},
    {"for", TokenType::FORTK},
    {"getint", TokenType::GETINTTK},
    {"getchar", TokenType::GETCHARTK},
    {"printf", TokenType::PRINTFTK},
    {"return", TokenType::RETURNTK},
    {"void", TokenType::VOIDTK},
};

#endif //LEXER_H
