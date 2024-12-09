//
// Created by LENOVO on 2024/10/6.
//

#ifndef PARSER_H
#define PARSER_H
#include "ast.h"
#include "error.h"

class Parser {
    private:
    std::vector<Token>& tokens;
    std::vector<Error>& errors;
    int currentPos = 0;

    public:
    explicit Parser(std::vector<Token>& tokens, std::vector<Error>& errors):
    tokens(tokens), errors(errors) {};

    std::unique_ptr<CompUnit> parser();

    private:
    std::unique_ptr<Decl> parse_decl();
    std::unique_ptr<ConstDecl> parse_const_decl();
    std::unique_ptr<VarDecl> parse_var_decl();

    std::unique_ptr<VarDef> parse_var_def();
    std::unique_ptr<InitVal> parse_init_val();

    std::unique_ptr<LVal> parse_l_val();
    std::unique_ptr<Number> parse_number();
    std::unique_ptr<Character> parse_character();
    std::unique_ptr<PrimaryExp> parse_primary_exp();

    std::unique_ptr<FuncRParams> parse_func_r_params();
    std::unique_ptr<FuncFParam> parse_func_f_param();
    std::unique_ptr<FuncFParams> parse_func_f_params();
    std::unique_ptr<CallExp> parse_call_exp();

    std::unique_ptr<Exp> parse_exp();
    std::unique_ptr<PrimaryUnaryExp> parse_primary_unary_exp();
    std::unique_ptr<UnaryOp> parse_unary_op();
    std::unique_ptr<UnaryExp> parse_unary_exp();

    std::unique_ptr<MulExp> parse_mul_exp();
    std::unique_ptr<AddExp> parse_add_exp();
    std::unique_ptr<RelExp> parse_rel_exp();
    std::unique_ptr<EqExp> parse_eq_exp();
    std::unique_ptr<LAndExp> parse_l_and_exp();
    std::unique_ptr<LOrExp> parse_l_or_exp();
    std::unique_ptr<ConstExp> parse_const_exp();

    std::unique_ptr<ConstInitVal> parse_const_init_val();
    std::unique_ptr<ConstDef> parse_const_def();

    std::unique_ptr<FuncType> parse_func_type();

    std::unique_ptr<Stmt> parse_stmt();

    std::unique_ptr<LValExp> parse_l_val_exp();
    std::unique_ptr<AssignStmt> parse_assign_stmt();
    std::unique_ptr<ExpStmt> parse_exp_stmt();

    std::unique_ptr<LValWrapStmt> parse_l_val_wrap_stmt();
    std::unique_ptr<Cond> parse_cond();
    std::unique_ptr<IfStmt> parse_if_stmt();
    std::unique_ptr<ForStmt_> parse_for_stmt_();
    std::unique_ptr<ForStmt> parse_for_stmt();
    std::unique_ptr<BreakStmt> parse_break_stmt();
    std::unique_ptr<ContinueStmt> parse_continue_stmt();
    std::unique_ptr<ReturnStmt> parse_return_stmt();
    std::unique_ptr<PrintfStmt> parse_printf_stmt();

    std::unique_ptr<BlockItem> parse_block_item();
    std::unique_ptr<Block> parse_block();

    std::unique_ptr<FuncDef> parse_func_def();
    std::unique_ptr<MainFuncDef> parse_main_func_def();
    std::unique_ptr<CompUnit> parse_comp_unit();


    [[nodiscard]] bool is_decl_in_comp_unit() const;
    [[nodiscard]] bool is_func_def_in_comp_unit() const;

    bool can_be_exp() const;
};

#endif //PARSER_H
