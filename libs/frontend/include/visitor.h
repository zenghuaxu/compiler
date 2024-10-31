//
// Created by LENOVO on 2024/10/14.
//

#ifndef VISITOR_H
#define VISITOR_H
#include <memory>

#include "ast.h"
#include "error.h"
#include "symtable.h"

#define BLOCK_WITH_RETURN 1
#define BLOCK_WITHOUT_RETURN 0
#define BLOCK_PLAIN (-1)

class Visitor {
    private:
    std::vector<std::shared_ptr<SymTable>> sym_tables;
    std::shared_ptr<SymTable> current_sym_tab;
    std::vector<Error> &errors;
    int current_tab_id = 1;

    public:
    Visitor(std::vector<Error> &errors): errors(errors) {
        current_sym_tab = std::make_shared<SymTable>();
        #ifdef SYMTABLE_
        sym_tables.push_back(current_sym_tab);
        #endif
    };

    void push_table() {
        current_sym_tab = current_sym_tab->push_scope(++current_tab_id);
        #ifdef SYMTABLE_
        sym_tables.push_back(current_sym_tab);
        #endif
    }

    void visit_number(Number &node);

    void visit_character(Character &node);

    void visit_l_or_exp(LOrExp &node);

    void visit_l_and_exp(LAndExp &node);

    void visit_eq_exp(EqExp &node);

    void visit_rel_exp(RelExp &node);

    void visit_add_exp(AddExp &node, std::shared_ptr<SymType> &type);

    void visit_mul_exp(MulExp &node, std::shared_ptr<SymType> &type);

    void visit_unary_exp(UnaryExp &node, std::shared_ptr<SymType> &type);

    void visit_primary_exp(PrimaryExp &node, std::shared_ptr<SymType> &type);

    void visit_call_exp(CallExp &node, std::shared_ptr<SymType> &type);

    void visit(CompUnit &node);

    void visit_decl(Decl &node);

    void print_symbol(std::ostream &out);

private:
    void visit_decl(Decl &node) const;

    void visit_const_def(ConstDef &node, const std::shared_ptr<BasicType>& basic_type);

    void visit_var_def(VarDef &node, const std::shared_ptr<BasicType>& basic_type);

    void visit_stmt(Stmt &node, int if_return, bool if_for);

    void visit_l_val_wrap_stmt(LValWrapStmt &node);

    void visit_l_val_stmt(LValStmt &node);

    void visit_assign_stmt(AssignStmt &node);

    void visit_exp_stmt(ExpStmt &node);

    void visit_if_stmt(IfStmt &node, int if_return, bool if_for);

    void visit_for_stmt(ForStmt &node, int if_return, bool if_for);

    void visit_for__stmt(ForStmt_ &node);

    void visit_break_stmt(BreakStmt &node, bool if_for);

    void visit_continue_stmt(ContinueStmt &node, bool if_for);

    void visit_return_stmt(ReturnStmt &node, bool if_return);

    void visit_printf_stmt(PrintfStmt &node);

    int visit_str(const std::string &str);

    void visit_l_val_with_no_evaluate(LVal &node, std::shared_ptr<SymType> &l_type);

    void visit_l_val(LVal &node, std::shared_ptr<SymType> &type);

    void visit_l_val_exp(LValExp &node);

    void visit_func(FuncDef &node);

    void visit_func_f_params(FuncFParams &node, const std::shared_ptr<Symbol> &symbol);

    void visit_return_block(Block &node);

    void visit_non_return_block(Block &node);

    void visit_block(Block &node, int if_return, bool if_for);

    void visit_main_func(MainFuncDef &node);
};

#endif //VISITOR_H
