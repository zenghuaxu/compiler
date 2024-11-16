//
// Created by LENOVO on 2024/10/6.
//

#ifndef AST_H
#define AST_H

#include <assert.h>
#include <unordered_map>
#include <vector>
#include <variant>
#include <bits/unique_ptr.h>

#include "token.h"

struct Node {
    int line;

    Node() = default;
    virtual ~Node() = default;

    virtual void print(std::ostream &out) = 0;
};

// EXPRESSION
struct Number : Node {
    int value;
    std::unique_ptr<Token> token;

    void print(std::ostream &out) override;
};

struct Character : Node {
    int value;
    std::unique_ptr<Token> token;

    void print(std::ostream &out) override;
    int get_value();
};

struct LVal;
struct Exp;
using PrimaryExp = std::variant<Exp, Number, Character, LVal>;

struct PrimaryUnaryExp;
struct CallExp;
struct OpUnaryExp;
using UnaryExp = std::variant<PrimaryUnaryExp, CallExp, OpUnaryExp>;

struct PrimaryUnaryExp : Node {
    std::unique_ptr<PrimaryExp> primary;

    void print(std::ostream &out) override;
};

struct UnaryOp : Node {
    std::unique_ptr<Token> op_token;

    void print(std::ostream &out) override;
};

struct OpUnaryExp : Node {
    std::unique_ptr<UnaryOp> unary_op;
    std::unique_ptr<UnaryExp> unary_exp;

    void print(std::ostream &out) override;
};

struct MulExp : Node {
    std::unique_ptr<MulExp> lhs;
    std::unique_ptr<Token> op;
    std::unique_ptr<UnaryExp> rhs;

    void print(std::ostream &out) override;
};

struct AddExp : Node {
    std::unique_ptr<AddExp> lhs;
    std::unique_ptr<Token> op;
    std::unique_ptr<MulExp> rhs;

    void print(std::ostream &out) override;
};

struct Exp : Node {
    std::unique_ptr<AddExp> add_exp;

    void print(std::ostream &out) override;
};

struct ConstExp : Node {
    std::unique_ptr<AddExp> add_exp;

    void print(std::ostream &out) override;
};

struct RelExp : Node {
    std::unique_ptr<RelExp> lhs;
    std::unique_ptr<Token> op;
    std::unique_ptr<AddExp> rhs;

    void print(std::ostream &out) override;
};

struct EqExp :Node {
    std::unique_ptr<EqExp> lhs;
    std::unique_ptr<Token> op;
    std::unique_ptr<RelExp> rhs;

    void print(std::ostream &out) override;
};

struct LAndExp :Node {
    std::unique_ptr<LAndExp> lhs;
    std::unique_ptr<Token> op;
    std::unique_ptr<EqExp> rhs;

    void print(std::ostream &out) override;
};

struct LOrExp :Node {
    std::unique_ptr<LOrExp> lhs;
    std::unique_ptr<Token> op;
    std::unique_ptr<LAndExp> rhs;

    void print(std::ostream &out) override;
};

struct GetCharExp;
struct GetIntExp;
using LValExp = std::variant<Exp, GetCharExp, GetIntExp>;

struct LVal : Node {
    std::unique_ptr<Token> identifier;
    std::unique_ptr<Exp> exp;

    void print(std::ostream &out) override;
};

struct FuncRParams : Node {
    std::vector<std::unique_ptr<Exp>> exps;

    void print(std::ostream &out) override;
};

struct CallExp : Node {
    std::unique_ptr<Token> identifier;
    std::unique_ptr<FuncRParams> func_r_params;

    void print(std::ostream &out) override;
};

struct ConstInitVal :Node {
    enum Type {
        constExp,
        bracedConstExp,
        stringConst
    } type;
    std::vector<std::unique_ptr<ConstExp>> const_exps;
    std::unique_ptr<Token> string_const;

    void print(std::ostream &out) override;
};

struct InitVal :Node {
    enum Type {
        exp,
        bracedExp,
        stringConst
    } type;
    std::vector<std::unique_ptr<Exp>> exps;
    std::unique_ptr<Token> string_const;

    void print(std::ostream &out) override;
};

struct ConstDef :Node {
    std::unique_ptr<Token> identifier;
    std::unique_ptr<ConstExp> const_expr;
    std::unique_ptr<ConstInitVal> const_init_val;

    void print(std::ostream &out) override;
};

struct VarDef :Node {
    std::unique_ptr<Token> identifier;
    std::unique_ptr<ConstExp> const_exp;
    std::unique_ptr<InitVal> init_val;

    void print(std::ostream &out) override;
};

struct FuncType :Node {
    std::unique_ptr<Token> type;

    void print(std::ostream &out) override;
};

struct FuncFParam: Node {
    std::unique_ptr<Token> b_type;
    std::unique_ptr<Token> identifier;
    bool array_type;

    void print(std::ostream &out) override;
};

struct FuncFParams :Node {
    std::vector<std::unique_ptr<FuncFParam>> func_f_params;

    void print(std::ostream &out) override;
};

//BLOCK ITEM
struct ConstDecl;
struct VarDecl;
using Decl = std::variant<ConstDecl, VarDecl>;

struct AssignStmt;
struct ExpStmt;
using  LValStmt = std::variant<AssignStmt, ExpStmt>;

struct AssignStmt : Node {
    std::unique_ptr<LVal> l_val;
    std::unique_ptr<LValExp> l_val_exp;

    void print(std::ostream &out) override;
};

struct ExpStmt : Node {
    std::unique_ptr<Exp> exp;

    void print(std::ostream &out) override;
};

struct LValWrapStmt;
struct IfStmt;
struct ForStmt;
struct BreakStmt;
struct ContinueStmt;
struct ReturnStmt;
struct PrintfStmt;
struct Block;
using Stmt = std::variant<LValWrapStmt, IfStmt, ForStmt,
BreakStmt, ContinueStmt, ReturnStmt, PrintfStmt, Block>;

using BlockItem = std::variant<Decl, Stmt>;

struct ConstDecl : Node {
    std::unique_ptr<Token> b_type;
    std::vector<std::unique_ptr<ConstDef>> const_defs;

    void print(std::ostream &out) override;
};

struct VarDecl : Node {
    std::unique_ptr<Token> b_type;
    std::vector<std::unique_ptr<VarDef>> var_defs;

    void print(std::ostream &out) override;
};

struct LValWrapStmt : Node {
    std::unique_ptr<LValStmt> l_val_stmt;

    void print(std::ostream &out) override;
};

struct Cond : Node {
    std::unique_ptr<LOrExp> l_or_exp;

    void print(std::ostream &out) override;
};

struct IfStmt : Node {
    std::unique_ptr<Cond> cond;
    std::unique_ptr<Stmt> if_stmt;
    std::unique_ptr<Stmt> else_stmt;

    void print(std::ostream &out) override;
};

//forStmt in the GRAMMAR
struct ForStmt_ :Node {
    std::unique_ptr<LVal> l_val;
    std::unique_ptr<Exp> exp;

    void print(std::ostream &out) override;
};

//My ForStmt
struct ForStmt : Node {
    std::unique_ptr<ForStmt_> init;
    std::unique_ptr<Cond> cond;
    std::unique_ptr<ForStmt_> loop_stmt;
    std::unique_ptr<Stmt> body_stmt;

    void print(std::ostream &out) override;
};

struct BreakStmt : Node {
    void print(std::ostream &out) override;
};

struct ContinueStmt : Node {
    void print(std::ostream &out) override;
};

struct ReturnStmt : Node {
    std::unique_ptr<Exp> exp;

    void print(std::ostream &out) override;
};

struct GetIntExp : Node {
    void print(std::ostream &out) override;
};

struct GetCharExp : Node {
    void print(std::ostream &out) override;
};

struct PrintfStmt : Node {
    std::unique_ptr<Token> string_const;
    std::vector<std::unique_ptr<Exp>> exps;

    void print(std::ostream &out) override;
};

struct Block : Node {
    std::vector<std::unique_ptr<BlockItem>> block_items;

    void print(std::ostream &out) override;
};

struct FuncDef : Node {
    std::unique_ptr<FuncType> func_type;
    std::unique_ptr<Token> identifier;
    std::unique_ptr<FuncFParams> func_f_params;
    std::unique_ptr<Block> block;

    void print(std::ostream &out) override;
};

struct MainFuncDef : Node {
    std::unique_ptr<Block> block;

    void print(std::ostream &out) override;
};

struct CompUnit : Node {
    std::vector<std::unique_ptr<Decl>> decls;
    std::vector<std::unique_ptr<FuncDef>> func_defs;
    std::unique_ptr<MainFuncDef> main_func_def;

    void print(std::ostream &out) override;
};

const std::unordered_map<std::string, int> char_to_value = {
    {"\\a", 7},
    {"\\b", 8},
    {"\\t", 9},
    {"\\n", 10},
    {"\\v", 11},
    {"\\f", 12},
    {"\\\"", 34},
    {"\\'", 39},
    {"\\\\", 92},
    {"\\0", 0},
};

inline int Character::get_value() {
    auto content = token->getContent();
    auto ch = content[1];
    if (ch != '\\') {
        assert(std::size(content) == 3);
        return content[1];
    }
    assert(std::size(content) > 2);
    std::string str = content.substr(1, content.size() - 2);
    assert(char_to_value.find(str) != char_to_value.end());
    return char_to_value.at(str);
}

inline std::vector<int> split_to_int(const std::string& str) {
    std::vector<int> result;
    for (int i = 1 ; i < str.size() - 1; i++) {
        if (str[i] == '\\') {
            result.push_back(char_to_value.at(str.substr(i, 2)));
            i++;
        }
        else {
            result.push_back(str.at(i));
        }
    }
    return result;
}

#endif //AST_H
