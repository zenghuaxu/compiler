//
// Created by LENOVO on 2024/10/14.
//

#ifndef VISITOR_H
#define VISITOR_H
#include "ast.h"
#include "error.h"
#include "symtable.h"
#include "../../llvm/include/llvm.h"
#include "../../include/configure.h"

#define BLOCK_WITH_RETURN 1
#define BLOCK_WITHOUT_RETURN 0
#define BLOCK_PLAIN (-1)

class Visitor {
    private:
    std::vector<std::shared_ptr<SymTable>> sym_tables;
    std::shared_ptr<SymTable> current_sym_tab;
    std::vector<Error> &errors;
    int current_tab_id = 1;
    int str_id = 0;
    ModulePtr current_module;
    BasicBlockPtr current_basic_block;

    public:
    explicit Visitor(std::vector<Error> &errors);
    ModulePtr visit(CompUnit &node);
    void print_symbol(std::ostream &out);
    void print_llvm(std::ostream &out);

private:
    BasicBlockPtr new_basic_block(FunctionPtr function);

    void visit_const_def(ConstDef &node, const std::shared_ptr<BasicType> &basic_type, bool isGlobal);

    void visit_var_def(VarDef &node, const std::shared_ptr<BasicType> &basic_type, bool isGlobal);

    void visit_stmt(Stmt &node, int if_return, bool if_for, BasicBlockPtr break_return, BasicBlockPtr continue_return);

    void visit_l_val_wrap_stmt(LValWrapStmt &node);

    void visit_l_val_stmt(LValStmt &node);

    void visit_assign_stmt(AssignStmt &node);

    void visit_exp_stmt(ExpStmt &node);

    void visit_if_stmt(IfStmt &node, int if_return, bool if_for, BasicBlockPtr break_return, BasicBlockPtr continue_return, BasicBlockPtr out);

    void visit_for_stmt(ForStmt &node, int if_return, bool if_for, BasicBlockPtr out);

    void visit_for__stmt(ForStmt_ &node);

    void visit_break_stmt(BreakStmt &node, bool if_for, BasicBlockPtr return_block);

    void visit_continue_stmt(ContinueStmt &node, bool if_for, BasicBlockPtr return_block);

    void visit_return_stmt(ReturnStmt &node, bool if_return);

    void visit_printf_stmt(PrintfStmt &node);

    static std::vector<bool> visit_str(const std::string &str);

    static std::string get_string_prefix(int &begin, std::string &str);

    void putstr(int &begin, std::string &string);

    ValuePtr visit_l_val_with_no_evaluate(LVal &node, std::shared_ptr<SymType> &l_type);

    ValuePtr visit_l_val(LVal &node, std::shared_ptr<SymType> &type);

    ValuePtr visit_l_val_exp(LValExp &node);

    void visit_func(FuncDef &node);

    void visit_func_f_params(FuncFParams &node, const std::shared_ptr<Symbol> &symbol);

    void visit_block(Block &node, int if_return, bool if_for, BasicBlockPtr break_return, BasicBlockPtr continue_return);

    void visit_main_func(MainFuncDef &node);


    void push_table() {
        current_sym_tab = current_sym_tab->push_scope(++current_tab_id);
        #ifdef SYMTABLE_
        sym_tables.push_back(current_sym_tab);
        #endif
    }

    ValuePtr visit_number(Number &node) const;

    ValuePtr visit_character(Character &node) const;

    void visit_l_or_exp(LOrExp &node, BasicBlockPtr true_block, BasicBlockPtr false_block);

    void visit_l_and_exp(LAndExp &node, BasicBlockPtr true_block, BasicBlockPtr false_block);

    ValuePtr visit_eq_exp(EqExp &node);

    ValuePtr visit_rel_exp(RelExp &node);

    ValuePtr type_conversion(std::shared_ptr<SymType> &origin, const std::shared_ptr<SymType>& target, ValuePtr &value);

    ValuePtr type_conversion(ValueReturnTypePtr origin, ValueReturnTypePtr target, ValuePtr value);

    ValuePtr construct_binary_inst(ValuePtr left, ValuePtr right, std::shared_ptr<SymType> &type);

    ValuePtr visit_add_exp(AddExp &node, std::shared_ptr<SymType> &type);

    ValuePtr visit_mul_exp(MulExp &node, std::shared_ptr<SymType> &type);

    ValuePtr visit_unary_exp(UnaryExp &node, std::shared_ptr<SymType> &type);

    ValuePtr visit_primary_exp(PrimaryExp &node, std::shared_ptr<SymType> &type);

    ValuePtr visit_call_exp(CallExp &node, std::shared_ptr<SymType> &type);

    void visit_decl(Decl &node, bool isGlobal);

    ValuePtr allocation(const std::shared_ptr<SymType> &type, bool isGlobal, bool isConst, std::string ident);

    template<class T>
    void init_global_object(ValuePtr value, std::shared_ptr<Symbol> symbol, T &const_exps, std::string string_const);

    template<class T1, class T2>
    void init_local_object(ValuePtr value, std::shared_ptr<Symbol> symbol, T1 type, T2 &const_exps,
                           std::string string_const);

    std::string get_string_name();
};

#endif //VISITOR_H
