//
// Created by LENOVO on 2024/9/20.
//

#ifndef TOKEN_H
#define TOKEN_H
#include <map>
#include <string>
#include <utility>
#include <iostream>

enum class TokenType {
    IDENFR,
    INTCON,
    STRCON,
    CHRCON,
    MAINTK,
    CONSTTK,
    INTTK,
    CHARTK,
    BREAKTK,
    CONTINUETK,
    IFTK,
    ELSETK,
    NOT,
    AND,
    OR,
    FORTK,
    GETINTTK,
    GETCHARTK,
    PRINTFTK,
    RETURNTK,
    PLUS,
    MINU,
    VOIDTK,
    MULT,
    DIV,
    MOD,
    LSS,
    LEQ,
    GRE,
    GEQ,
    EQL,
    NEQ,
    ASSIGN,
    SEMICN,
    COMMA,
    LPARENT,
    RPARENT,
    LBRACK,
    RBRACK,
    LBRACE,
    RBRACE,
};

inline std::string tokenTypeToStr(TokenType t) {
    static std::map<TokenType, std::string> tokenStrMap = {
        {TokenType::IDENFR, "IDENFR"},
        {TokenType::INTCON, "INTCON"},
        {TokenType::STRCON, "STRCON"},
        {TokenType::CHRCON, "CHRCON"},
        {TokenType::MAINTK, "MAINTK"},
        {TokenType::CONSTTK, "CONSTTK"},
        {TokenType::INTTK, "INTTK"},
        {TokenType::CHARTK, "CHARTK"},
        {TokenType::BREAKTK, "BREAKTK"},
        {TokenType::CONTINUETK, "CONTINUETK"},
        {TokenType::IFTK, "IFTK"},
        {TokenType::ELSETK, "ELSETK"},
        {TokenType::NOT, "NOT"},
        {TokenType::AND, "AND"},
        {TokenType::OR, "OR"},
        {TokenType::FORTK, "FORTK"},
        {TokenType::GETINTTK, "GETINTTK"},
        {TokenType::GETCHARTK, "GETCHARTK"},
        {TokenType::PRINTFTK, "PRINTFTK"},
        {TokenType::RETURNTK, "RETURNTK"},
        {TokenType::PLUS, "PLUS"},
        {TokenType::MINU, "MINU"},
        {TokenType::VOIDTK, "VOIDTK"},
        {TokenType::MULT, "MULT"},
        {TokenType::DIV, "DIV"},
        {TokenType::MOD, "MOD"},
        {TokenType::LSS, "LSS"},
        {TokenType::LEQ, "LEQ"},
        {TokenType::GRE, "GRE"},
        {TokenType::GEQ, "GEQ"},
        {TokenType::EQL, "EQL"},
        {TokenType::NEQ, "NEQ"},
        {TokenType::ASSIGN, "ASSIGN"},
        {TokenType::SEMICN, "SEMICN"},
        {TokenType::COMMA, "COMMA"},
        {TokenType::LPARENT, "LPARENT"},
        {TokenType::RPARENT, "RPARENT"},
        {TokenType::LBRACK, "LBRACK"},
        {TokenType::RBRACK, "RBRACK"},
        {TokenType::LBRACE, "LBRACE"},
        {TokenType::RBRACE, "RBRACE"},
        {TokenType::LBRACK, "LBRACK"},
    };
    try {
        return tokenStrMap[t];
    } catch (const std::out_of_range& e) {
        std::cout << "Key 2 does not exist in the map." << std::endl;
        return "";
    }
}

class Token {
    TokenType type;
    int line;
    std::string content;

    public:

    Token(TokenType type, int line, std::string content)
    : type(type), line(line), content(std::move(content)) {}

    std:: string toString() const {
        # ifdef DEBUG
                return tokenTypeToStr(type) +  ' ' + content + ' ' + std::to_string(line);
        # else
                return tokenTypeToStr(type) + ' ' + content;
        #endif
    }

    bool operator==(TokenType t) const {
        return type == t;
    }

    int getLine() const {
        return line;
    }

    std::string getContent() {
        return content;
    }
};

#endif //TOKEN_H
