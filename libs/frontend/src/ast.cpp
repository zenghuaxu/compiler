//
// Created by LENOVO on 2024/10/7.
//
#include "../include/ast.h"

#define LPARENT    "LPARENT ("
#define RPARENT    "RPARENT )"
#define LBRACKET   "LBRACK ["
#define RBRACKET   "RBRACK ]"
#define LBRACE     "LBRACE {"
#define RBRACE     "RBRACE }"
#define SEMI_COLON "SEMICN ;"
#define COMMA      "COMMA ,"
#define ASSIGN     "ASSIGN ="
#define CONST      "CONSTTK const"
#define IF         "IFTK if"
#define ELSE       "ELSETK else"
#define FOR        "FORTK for"
#define BREAK      "BREAKTK break"
#define CONTINUE   "CONTINUETK continue"
#define RETURN     "RETURNTK return"
#define GETINT     "GETINTTK getint"
#define GETCHAR    "GETCHARTK getchar"
#define PRINTF     "PRINTFTK printf"

void Number::print(std::ostream &out) {
    out << token->toString() << std::endl;
    out << "<Number>" <<std::endl;
}

void Character::print(std::ostream &out) {
    out << token->toString() << std::endl;
    out << "<Character>" <<std::endl;
}

void PrimaryUnaryExp::print(std::ostream &out) {
    if (std::holds_alternative<Exp>(*primary)) {
        out << LPARENT << std::endl;
    }
    std::visit([&](auto &arg) {
        arg.print(out);
    },
    (*primary));
    if (std::holds_alternative<Exp>(*primary)) {
        out << RPARENT << std::endl;
    }
    out << "<PrimaryExp>" << std::endl;
}

void UnaryOp::print(std::ostream &out) {
    out << op_token->toString() << std::endl;
    out << "<UnaryOp>" << std::endl;
}

void OpUnaryExp::print(std::ostream &out) {
    unary_op->print(out);
    std::visit(
        [&out](auto &arg) {
            arg.print(out);
        },
        (*unary_exp));
    out << "<UnaryExp>" << std::endl;
}

void MulExp::print(std::ostream &out) {
    if (lhs) {
        lhs->print(out);
        out << op->toString()<<std::endl;
    }
    std::visit(
        [&out](auto &arg) {
            arg.print(out);
        },
        (*rhs));
    out << "<UnaryExp>" << std::endl;
    out << "<MulExp>" << std::endl;
}

void AddExp::print(std::ostream &out) {
    if (lhs) {
        lhs->print(out);
        out << op->toString()<<std::endl;
    }
    rhs->print(out);
    out << "<AddExp>" << std::endl;
}

void Exp::print(std::ostream &out) {
    add_exp->print(out);
    out << "<Exp>" << std::endl;
}

void ConstExp::print(std::ostream &out) {
    add_exp->print(out);
    out << "<ConstExp>" << std::endl;
}

void RelExp::print(std::ostream &out) {
    if (lhs) {
        lhs->print(out);
        out << op->toString()<<std::endl;
    }
    rhs->print(out);
    out << "<RelExp>" << std::endl;
}

void EqExp::print(std::ostream &out) {
    if (lhs) {
        lhs->print(out);
        out << op->toString()<<std::endl;
    }
    rhs->print(out);
    out << "<EqExp>" << std::endl;
}

void LAndExp::print(std::ostream &out) {
    if (lhs) {
        lhs->print(out);
        out << op->toString() <<std::endl;
    }
    rhs->print(out);
    out << "<LAndExp>" << std::endl;
}

void LOrExp::print(std::ostream &out) {
    if (lhs) {
        lhs->print(out);
        out << op->toString()<<std::endl;
    }
    rhs->print(out);
    out << "<LOrExp>" << std::endl;
}

void LVal::print(std::ostream &out) {
    out << identifier->toString()<<std::endl;
    if (exp) {
        out << LBRACKET << std::endl;
        exp->print(out);
        out << RBRACKET << std::endl;
    }
    out << "<LVal>" << std::endl;
}

void FuncRParams::print(std::ostream &out) {
    exps.at(0)->print(out);
    for (int i = 1; i < exps.size(); i++) {
        out << COMMA << std::endl;
        exps.at(i)->print(out);
    }
    out << "<FuncRParams>" << std::endl;
}

void CallExp::print(std::ostream &out) {
    out << identifier->toString()<<std::endl;
    out << LPARENT << std::endl;
    if (func_r_params) {
        func_r_params->print(out);
    }
    out << RPARENT << std::endl;
}

void ConstInitVal::print(std::ostream &out) {
    switch (type) {
        case constExp: {
            const_exps.at(0)->print(out);
            break;
        }
        case bracedConstExp: {
            out << LBRACE << std::endl;
            if (!const_exps.empty()) {
                const_exps.at(0)->print(out);
                for (int i = 1; i < const_exps.size(); i++) {
                    out << COMMA << std::endl;
                    const_exps.at(i)->print(out);
                }
            }
            out << RBRACE << std::endl;
            break;
        }
        case stringConst: {
            out << string_const->toString()<<std::endl;
            break;
        }
    }
    out << "<ConstInitVal>" << std::endl;
}

void InitVal::print(std::ostream &out) {
    switch (type) {
        case exp: {
            exps.at(0)->print(out);
            break;
        }
        case bracedExp: {
            out << LBRACE << std::endl;
            if (!exps.empty()) {
                exps.at(0)->print(out);
            }
            for (int i = 1; i < exps.size(); i++) {
                out << COMMA << std::endl;
                exps.at(i)->print(out);
            }
            out << RBRACE << std::endl;
            break;
        }
        case stringConst: {
            out << string_const->toString()<<std::endl;
            break;
        }
    }
    out << "<InitVal>" << std::endl;
}

void ConstDef::print(std::ostream &out) {
    out << identifier->toString()<<std::endl;
    if (const_expr) {
        out << LBRACKET << std::endl;
        const_expr->print(out);
        out << RBRACKET << std::endl;
    }
    out << ASSIGN << std::endl;
    const_init_val->print(out);
    out << "<ConstDef>" << std::endl;
}

void VarDef::print(std::ostream &out) {
    out << identifier->toString()<<std::endl;
    if (const_exp) {
        out << LBRACKET << std::endl;
        const_exp->print(out);
        out << RBRACKET << std::endl;
    }
    if (init_val) {
        out << ASSIGN << std::endl;
        init_val->print(out);
    }
    out << "<VarDef>" << std::endl;
}

void FuncType::print(std::ostream &out) {
    out << type->toString()<<std::endl;
    out << "<FuncType>" << std::endl;
}

void FuncFParam::print(std::ostream &out) {
    out << b_type->toString()<<std::endl;
    out << identifier->toString()<<std::endl;
    if (array_type) {
        out << LBRACKET << std::endl;
        out << RBRACKET << std::endl;
    }
    out << "<FuncFParam>" << std::endl;
}
void FuncFParams::print(std::ostream &out) {
    func_f_params.at(0)->print(out);
    for (int i = 1; i < func_f_params.size(); i++) {
        out << COMMA << std::endl;
        func_f_params.at(i)->print(out);
    }
    out << "<FuncFParams>" << std::endl;
}

void ConstDecl::print(std::ostream &out) {
    out << CONST << std::endl;
    out << b_type->toString() <<std::endl;
    const_defs.at(0)->print(out);
    for (int i = 1; i < const_defs.size(); i++) {
        out << COMMA << std::endl;
        const_defs.at(i)->print(out);
    }
    out << SEMI_COLON << std::endl;
    out << "<ConstDecl>" << std::endl;
}

void VarDecl::print(std::ostream &out) {
    out << b_type->toString()<<std::endl;
    var_defs.at(0)->print(out);
    for (int i = 1; i < var_defs.size(); i++) {
        out << COMMA << std::endl;
        var_defs.at(i)->print(out);
    }
    out << SEMI_COLON << std::endl;
    out << "<VarDecl>" << std::endl;
}

void AssignStmt::print(std::ostream &out) {
    l_val->print(out);
    out << ASSIGN << std::endl;
    std::visit(
        [&out](auto &arg) {
            auto &node_ref = static_cast<Node&>(arg);
            node_ref.print(out);
        },
        (*l_val_exp));
    out <<SEMI_COLON << std::endl;
}

void ExpStmt::print(std::ostream &out) {
    if (exp) {
        exp->print(out);
    }
    out << SEMI_COLON << std::endl;
}

void LValWrapStmt::print(std::ostream &out) {
    std::visit(
        [&out](auto &arg) {
            arg.print(out);
        },
        (*l_val_stmt));
}

void Cond::print(std::ostream &out) {
    l_or_exp->print(out);
    out << "<Cond>" << std::endl;
}

void IfStmt::print(std::ostream &out) {
    out << IF << std::endl;
    out << LPARENT << std::endl;
    cond->print(out);
    out << RPARENT << std::endl;
    std::visit(
        [&out](auto &arg) {
            arg.print(out);
        },
        (*if_stmt));
    out << "<Stmt>" << std::endl;

    if (else_stmt) {
        out << ELSE << std::endl;
        std::visit(
            [&out](auto &arg) {
                arg.print(out);
            },
            (*else_stmt));
        out << "<Stmt>" << std::endl;
    }
}

void ForStmt_::print(std::ostream &out) {
    l_val->print(out);
    out << ASSIGN << std::endl;
    exp->print(out);
    out << "<ForStmt>" << std::endl;
}

void ForStmt::print(std::ostream &out) {
    out << FOR << std::endl;
    out << LPARENT << std::endl;
    if (init) {
        init -> print(out);
    }
    out << SEMI_COLON << std::endl;
    if (cond) {
        cond -> print(out);
    }
    out << SEMI_COLON << std::endl;
    if (loop_stmt) {
        loop_stmt -> print(out);
    }
    out << RPARENT << std::endl;

    std::visit(
        [&out](auto &arg) {
            arg.print(out);
        },
        (*body_stmt));
    out << "<Stmt>" << std::endl;
}

void BreakStmt::print(std::ostream &out) {
    out << BREAK << std::endl;
    out << SEMI_COLON << std::endl;
}

void ContinueStmt::print(std::ostream &out) {
    out << CONTINUE << std::endl;
    out << SEMI_COLON << std::endl;
}

void ReturnStmt::print(std::ostream &out) {
    out << RETURN << std::endl;
    if (exp) {
        exp->print(out);
    }
    out << SEMI_COLON << std::endl;
}

void GetIntExp::print(std::ostream &out) {
    out << GETINT << std::endl;
    out << LPARENT << std::endl;
    out << RPARENT << std::endl;
}

void GetCharExp::print(std::ostream &out) {
    out << GETCHAR << std::endl;
    out << LPARENT << std::endl;
    out << RPARENT << std::endl;
}

void PrintfStmt::print(std::ostream &out) {
    out << PRINTF << std::endl;
    out << LPARENT << std::endl;
    out << string_const->toString() << std::endl;
    for (const auto & exp : exps) {
        out << COMMA << std::endl;
        exp->print(out);
    }
    out << RPARENT << std::endl;
    out << SEMI_COLON << std::endl;
}

void Block::print(std::ostream &out) {
    out << LBRACE << std::endl;
    for (auto & block_item : block_items) {
        std::visit(
            [&out](auto &arg) {
                std::visit(
                    [&out](auto &arg) {
                        arg.print(out);
                    },
                    arg);
            },
            (*block_item));
        if (std::holds_alternative<Decl>(*block_item)) {}
        else {
            out << "<Stmt>" << std::endl;
        }
    }
    out << RBRACE << std::endl;
    out << "<Block>" << std::endl;
}

void FuncDef::print(std::ostream &out) {
    func_type->print(out);
    out << identifier->toString() << std::endl;
    out << LPARENT << std::endl;
    if (func_f_params) {
        func_f_params -> print(out);
    }
    out << RPARENT << std::endl;
    block->print(out);
    out << "<FuncDef>" << std::endl;
}

void MainFuncDef::print(std::ostream &out) {
    out << "INTTK int" << std::endl;
    out << "MAINTK main" << std::endl;
    out << LPARENT << std::endl;
    out << RPARENT << std::endl;
    block->print(out);
    out << "<MainFuncDef>" << std::endl;
}

void CompUnit::print(std::ostream &out) {
    for (auto & decl : decls) {
        std::visit(
        [&out](auto &arg) {
            arg.print(out);
        },
        (*decl));
    }

    for (auto & func_def : func_defs) {
        func_def->print(out);
    }

    main_func_def->print(out);
    out << "<CompUnit>" << std::endl;
}
