#include "include/lexer.h"
//
// Created by LENOVO on 2024/9/19.
//


#define whitespace std::string(" \n\t\r")

void Lexer::nextToken() {
        std::string content;
        char _char = input.get();


        while (whitespace.find(_char) != std::string::npos) {
            if (_char == '\n') {
                line++;
            }
            _char = input.get();
        }

        if (_char == EOF) {
            // std::cout << "end of file" << std::endl;
            return;
        }

        if (isalpha(_char) || _char == '_') {
            while (isalnum(_char) || _char == '_') {
                content.append(1, _char);
                _char = input.get();
            }
            input.unget();
            if (keywordsMap.find(content) != keywordsMap.end()) {
                tokens.emplace_back(keywordsMap.at(content), line, content);
            }
            else {
                tokens.emplace_back(TokenType::IDENFR, line, content);
            }
        }

        else if (isdigit(_char)) {
            while (isdigit(_char)) {
                content.append(1, _char);
                _char = input.get();
            }
            input.unget();
            tokens.emplace_back(TokenType::INTCON, line, content);
        }

        else if (_char == '"') {
            content.append(1, _char);
            _char = input.get();
            while (_char != '"') {
                content.append(1, _char);
                if (_char == '\\') {
                    _char = input.get();
                    content.append(1, _char);
                }
                _char = input.get();
            }
            content.append(1, _char);
            tokens.emplace_back(TokenType::STRCON, line, content);
        }

        else if (_char == '\'') {
            content.append(1, _char);
            _char = input.get();
            if (_char == '\\') {
                content.append(1, _char);
                _char = input.get();
                content.append(1, _char);
                _char = input.get();
                content.append(1, _char);
                tokens.emplace_back(TokenType::CHRCON, line, content);
            }
            else {
                content.append(1, _char);
                _char = input.get();
                content.append(1, _char);
                tokens.emplace_back(TokenType::CHRCON, line, content);
            }
        }

        else if (_char == '!') {
            content.append(1, _char);
            _char = input.get();
            if (_char == '=') {
                content.append(1, _char);
                tokens.emplace_back(TokenType::NEQ, line, content);
            }
            else {
                input.unget();
                tokens.emplace_back(TokenType::NOT, line, content);
            }
        }

        else if (_char == '&') {
            content.append(1, _char);
            _char = input.get();
            if (_char == '&') {
                content.append(1, _char);
                tokens.emplace_back(TokenType::AND, line, content);
            }
            else {
                errors.emplace_back('a', line);//type a error
                input.unget();
                content.append(1, '&');
                tokens.emplace_back(TokenType::AND, line, content);
            }
        }

        else if (_char == '|') {
            content.append(1, _char);
            _char = input.get();
            if (_char == '|') {
                content.append(1, _char);
                tokens.emplace_back(TokenType::OR, line, content);
            }
            else {
                errors.emplace_back('a', line);
                input.unget();
                content.append(1, '|');
                tokens.emplace_back(TokenType::OR, line, content);
            }
        }

        else if (_char == '+') {
            content.append(1, _char);
            tokens.emplace_back(TokenType::PLUS, line, content);
        }

        else if (_char == '-') {
            content.append(1, _char);
            tokens.emplace_back(TokenType::MINU, line, content);
        }

        else if (_char == '*') {
            content.append(1, _char);
            tokens.emplace_back(TokenType::MULT, line, content);
        }

        else if (_char == '/') {
            content.append(1, _char);
            _char = input.get();
            if (_char == '/') {
                while (_char != '\n' && _char != EOF) {
                    _char = input.get();
                }
                if (_char == EOF) {
                    return;
                }
                input.unget();
                // nextToken();
            }
            else if (_char == '*') {
                char pre_char = input.get();
                _char = input.get();
                while (!(_char == '/' && pre_char == '*')) {
                    if (pre_char == '\n') {
                        line++;
                    } //record the lineno
                    if (_char == EOF) {
                        return;
                    }
                    pre_char = _char;
                    _char = input.get();
                }
                // nextToken();
            }
            else {
                input.unget();
                tokens.emplace_back(TokenType::DIV, line, content);
            }
        }

        else if (_char == '%') {
            content.append(1, _char);
            tokens.emplace_back(TokenType::MOD, line, content);
        }

        else if (_char == '<') {
            content.append(1, _char);
            _char = input.get();
            if (_char == '=') {
                content.append(1, _char);
                tokens.emplace_back(TokenType::LEQ, line, content);
            }
            else {
                input.unget();
                tokens.emplace_back(TokenType::LSS, line, content);
            }
        }

        else if (_char == '>') {
            content.append(1, _char);
            _char = input.get();
            if (_char == '=') {
                content.append(1, _char);
                tokens.emplace_back(TokenType::GEQ, line, content);
            }
            else {
                input.unget();
                tokens.emplace_back(TokenType::GRE, line, content);
            }
        }

        else if (_char == '=') {
            content.append(1, _char);
            _char = input.get();
            if (_char == '=') {
                content.append(1, _char);
                tokens.emplace_back(TokenType::EQL, line, content);
            }
            else {
                input.unget();
                tokens.emplace_back(TokenType::ASSIGN, line, content);
            }
        }

        else if (_char == ';') {
            content.append(1, _char);
            tokens.emplace_back(TokenType::SEMICN, line, content);
        }

        else if (_char == ',') {
            content.append(1, _char);
            tokens.emplace_back(TokenType::COMMA, line, content);
        }

        else if (_char == '(') {
            content.append(1, _char);
            tokens.emplace_back(TokenType::LPARENT, line, content);
        }

        else if (_char == ')') {
            content.append(1, _char);
            tokens.emplace_back(TokenType::RPARENT, line, content);
        }

        else if (_char == '[') {
            content.append(1, _char);
            tokens.emplace_back(TokenType::LBRACK, line, content);
        }

        else if (_char == ']') {
            content.append(1, _char);
            tokens.emplace_back(TokenType::RBRACK, line, content);
        }

        else if (_char == '{') {
            content.append(1, _char);
            tokens.emplace_back(TokenType::LBRACE, line, content);
        }

        else if (_char == '}') {
            content.append(1, _char);
            tokens.emplace_back(TokenType::RBRACE, line, content);
        }

        else {
            /*TODO ERR*/
        }
}
