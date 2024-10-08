//
// Created by LENOVO on 2024/10/6.
//

#include "include/parser.h"
#include "include/ast.h"
#include "include/configure.h"
#include <cassert>

#include <memory>

std::unique_ptr<CompUnit> Parser::parser() {
    auto compUnit = parse_comp_unit();
    return compUnit;
}

std::unique_ptr<CompUnit> Parser::parse_comp_unit() {
    auto compUnit = std::make_unique<CompUnit>();
    while (is_decl_in_comp_unit()) {
        compUnit->decls.push_back(parse_decl());
    }
    while (is_func_def_in_comp_unit()) {
        compUnit->func_defs.push_back(parse_func_def());
    }
    compUnit->main_func_def = parse_main_func_def();
    return compUnit;
}

std::unique_ptr<Decl> Parser::parse_decl() {
    if (tokens.at(currentPos) == TokenType::CONSTTK) {
        return std::make_unique<Decl>(std::move(*parse_const_decl()));
    }
    return std::make_unique<Decl>(std::move(*parse_var_decl()));
}

std::unique_ptr<ConstDecl> Parser::parse_const_decl() {
    auto constDecl = std::make_unique<ConstDecl>();
    currentPos++; //const
    constDecl->b_type = std::make_unique<Token>(tokens.at(currentPos)); //?
    currentPos++;
    constDecl->const_defs.push_back(parse_const_def());
    while (tokens.at(currentPos) == TokenType::COMMA) {
        currentPos++;
        constDecl->const_defs.push_back(parse_const_def());
    }
    if (tokens.at(currentPos) == TokenType::SEMICN) {
        currentPos++;
    }
    else {
        errors.emplace_back('i', tokens.at(currentPos - 1).getLine());
    }
    return constDecl;
}

std::unique_ptr<VarDecl> Parser::parse_var_decl() {
    auto varDecl = std::make_unique<VarDecl>();
    varDecl->b_type = std::make_unique<Token>(tokens.at(currentPos));
    currentPos++;
    varDecl->var_defs.push_back(parse_var_def());
    while (tokens.at(currentPos) == TokenType::COMMA) {
        currentPos++;
        varDecl->var_defs.push_back(parse_var_def());
    }
    if (tokens.at(currentPos) == TokenType::SEMICN) {
        currentPos++;
    }
    else {
        errors.emplace_back('i', tokens.at(currentPos - 1).getLine());
    }
    return varDecl;
}

std::unique_ptr<VarDef> Parser::parse_var_def() {
    auto varDef = std::make_unique<VarDef>();
    assert(tokens.at(currentPos) == TokenType::IDENFR);
    varDef->identifier = std::make_unique<Token>(tokens.at(currentPos));
    currentPos++;

    if (tokens.at(currentPos) == TokenType::LBRACK) {
        currentPos++;
        varDef->const_exp = parse_const_exp();
        if (tokens.at(currentPos) == TokenType::RBRACK) {
            currentPos++;
        }
        else {
            errors.emplace_back('k', tokens.at(currentPos - 1).getLine());
        }
    }

    if (tokens.at(currentPos) == TokenType::ASSIGN) {
        currentPos++;
        varDef->init_val = parse_init_val();
    }
    return varDef;
}

std::unique_ptr<LVal> Parser::parse_l_val() {
    auto lVal = std::make_unique<LVal>();

    #ifdef DEBUG_PARSER
        std::cout << tokens.at(currentPos).toString() << std::endl;
    #endif
    assert(tokens.at(currentPos) == TokenType::IDENFR);

    lVal->identifier = std::make_unique<Token>(tokens.at(currentPos));
    currentPos++;

    if (tokens.at(currentPos) == TokenType::LBRACK) {
        currentPos++;
        lVal->exp = parse_exp();
        if (tokens.at(currentPos) == TokenType::RBRACK) {
            currentPos++;
        }
        else {
            errors.emplace_back('k', tokens.at(currentPos - 1).getLine());
        }
        return lVal;
    }
    lVal->exp = nullptr;

    return lVal;
}

std::unique_ptr<Number> Parser::parse_number() {
    auto number = std::make_unique<Number>();
    number->token = std::make_unique<Token>(tokens.at(currentPos));
    currentPos++;
    return number;
}

std::unique_ptr<Character> Parser::parse_character() {
    auto character = std::make_unique<Character>();
    character->token = std::make_unique<Token>(tokens.at(currentPos));
    currentPos++;
    return character;
}

std::unique_ptr<PrimaryExp> Parser::parse_primary_exp() {

    #ifdef DEBUG_PARSER
        std::cout << tokens.at(currentPos).toString() << std::endl;
    #endif

    if (tokens.at(currentPos) == TokenType::LPARENT) {
        currentPos++;
        const auto exp = parse_exp();
        if (tokens.at(currentPos) == TokenType::RPARENT) {
            currentPos++;
        }
        else {
            errors.emplace_back('j', tokens.at(currentPos - 1).getLine());
        }
        return std::make_unique<PrimaryExp>(std::move(*exp));
    }

    else if (tokens.at(currentPos) == TokenType::IDENFR) {
        return std::make_unique<PrimaryExp>(std::move(*parse_l_val()));
    }
    else if (tokens.at(currentPos) == TokenType::INTCON) {
        return std::make_unique<PrimaryExp>(std::move(*parse_number()));
    }
    else if (tokens.at(currentPos) == TokenType::CHRCON) {
        return std::make_unique<PrimaryExp>(std::move(*parse_character()));
    }

    throw std::invalid_argument("Invalid token in primary exp");
}

std::unique_ptr<Exp> Parser::parse_exp() {
    auto exp = std::make_unique<Exp>();
    exp->add_exp = parse_add_exp();
    return exp;
}

std::unique_ptr<FuncRParams> Parser::parse_func_r_params() {
    auto funcRParams = std::make_unique<FuncRParams>();
    funcRParams->exps.push_back(parse_exp());
    while (tokens.at(currentPos) == TokenType::COMMA) {
        currentPos++;
        funcRParams->exps.push_back(parse_exp());
    }
    return funcRParams;
}

std::unique_ptr<FuncFParam> Parser::parse_func_f_param() {
    auto funcFParam = std::make_unique<FuncFParam>();
    assert(tokens.at(currentPos) == TokenType::INTTK ||
        tokens.at(currentPos) == TokenType::CHARTK);
    funcFParam->b_type = std::make_unique<Token>(tokens.at(currentPos));
    currentPos++;

    assert(tokens.at(currentPos) == TokenType::IDENFR);
    funcFParam->identifier = std::make_unique<Token>(tokens.at(currentPos));
    currentPos++;

    if (tokens.at(currentPos) == TokenType::LBRACK) {
        currentPos++;
        funcFParam->array_type = true;
        if (tokens.at(currentPos) == TokenType::RBRACK) {
            currentPos++;
        }
        else {
            errors.emplace_back('k', tokens.at(currentPos - 1).getLine());
        }
    }
    else {
        funcFParam->array_type = false;
    }

    return funcFParam;
}

std::unique_ptr<FuncFParams> Parser::parse_func_f_params() {
    auto funcFParams = std::make_unique<FuncFParams>();
    funcFParams->func_f_params.push_back(parse_func_f_param());
    while (tokens.at(currentPos) == TokenType::COMMA) {
        currentPos++;
        funcFParams->func_f_params.push_back(parse_func_f_param());
    }
    return funcFParams;
}

std::unique_ptr<CallExp> Parser::parse_call_exp() {
    auto callExp = std::make_unique<CallExp>();

    assert(tokens.at(currentPos) == TokenType::IDENFR);
    callExp->identifier = std::make_unique<Token>(tokens.at(currentPos));
    currentPos++;

    assert(tokens.at(currentPos) == TokenType::LPARENT);
    currentPos++;

    if (can_be_exp()) {
        callExp->func_r_params = parse_func_r_params();
    }
    else {
        callExp->func_r_params = nullptr;
    }

    if (tokens.at(currentPos) == TokenType::RPARENT) {
        currentPos++;
    }
    else {
        errors.emplace_back('j', tokens.at(currentPos - 1).getLine());
    }
    return callExp;
}

std::unique_ptr<UnaryOp> Parser::parse_unary_op() {
    auto unaryOp = std::make_unique<UnaryOp>();
    unaryOp->op_token = std::make_unique<Token>(tokens.at(currentPos));
    currentPos++;
    return unaryOp;
}

std::unique_ptr<UnaryExp> Parser::parse_unary_exp() {
    if (tokens.at(currentPos) == TokenType::PLUS ||
        tokens.at(currentPos) == TokenType::MINU ||
        tokens.at(currentPos) == TokenType::NOT) {
        OpUnaryExp op_unary_exp;
        op_unary_exp.unary_op = parse_unary_op();
        op_unary_exp.unary_exp = parse_unary_exp();
        return std::make_unique<UnaryExp>(std::move(op_unary_exp));
    }
    assert(currentPos + 2 < tokens.size());
    if (tokens.at(currentPos) == TokenType::IDENFR &&
        tokens.at(currentPos + 1) == TokenType::LPARENT) {
        return std::make_unique<UnaryExp>(std::move(*parse_call_exp()));
    }
    const auto primary_unary_exp = std::make_unique<PrimaryUnaryExp>();
    primary_unary_exp->primary = parse_primary_exp();
    return std::make_unique<UnaryExp>(std::move(*primary_unary_exp));
}

std::unique_ptr<MulExp> Parser::parse_mul_exp() {
    auto mul_exp = std::make_unique<MulExp>();
    mul_exp->lhs = nullptr;
    mul_exp->op  = nullptr;
    mul_exp->rhs = parse_unary_exp();

    while (tokens.at(currentPos) == TokenType::MULT ||
        tokens.at(currentPos) == TokenType::DIV ||
        tokens.at(currentPos) == TokenType::MOD) {
        auto newMulExp = std::make_unique<MulExp>();
        newMulExp->lhs = std::move(mul_exp);
        newMulExp->op  = std::make_unique<Token>(tokens.at(currentPos));
        currentPos++;
        newMulExp->rhs = parse_unary_exp();
        mul_exp = std::move(newMulExp);
    }

    return mul_exp;
}

std::unique_ptr<AddExp> Parser::parse_add_exp() {
    auto addExp = std::make_unique<AddExp>();
    addExp->lhs = nullptr;
    addExp->op  = nullptr;
    addExp->rhs = parse_mul_exp();

    while (tokens.at(currentPos) == TokenType::PLUS ||
        tokens.at(currentPos) == TokenType::MINU) {
        auto newAddExp = std::make_unique<AddExp>();
        newAddExp->lhs = std::move(addExp);
        newAddExp->op  = std::make_unique<Token>(tokens.at(currentPos));
        currentPos++;
        newAddExp->rhs = parse_mul_exp();
        addExp = std::move(newAddExp);
    }

    return addExp;
}

std::unique_ptr<RelExp> Parser::parse_rel_exp() {
    auto relExp = std::make_unique<RelExp>();
    relExp->lhs = nullptr;
    relExp->op  = nullptr;
    relExp->rhs = parse_add_exp();

    while (tokens.at(currentPos) == TokenType::LEQ ||
        tokens.at(currentPos) == TokenType::GEQ ||
        tokens.at(currentPos) == TokenType::LSS ||
        tokens.at(currentPos) == TokenType::GRE) {
        auto newRelExp = std::make_unique<RelExp>();
        newRelExp->lhs = std::move(relExp);
        newRelExp->op  = std::make_unique<Token>(tokens.at(currentPos));
        currentPos++;
        newRelExp->rhs = parse_add_exp();
        relExp = std::move(newRelExp);
    }

    return relExp;
}

std::unique_ptr<EqExp> Parser::parse_eq_exp() {
    auto eqExp = std::make_unique<EqExp>();
    eqExp->lhs = nullptr;
    eqExp->op  = nullptr;
    eqExp->rhs = parse_rel_exp();

    while (tokens.at(currentPos) == TokenType::EQL ||
        tokens.at(currentPos) == TokenType::NEQ) {
        auto newEqExp = std::make_unique<EqExp>();
        newEqExp->lhs = std::move(eqExp);
        newEqExp->op  = std::make_unique<Token>(std::move(tokens.at(currentPos)));
        currentPos++;
        newEqExp->rhs = parse_rel_exp();
        eqExp = std::move(newEqExp);
    }

    return eqExp;
}

std::unique_ptr<LAndExp> Parser::parse_l_and_exp() {
    auto lAndExp = std::make_unique<LAndExp>();
    lAndExp->lhs = nullptr;
    lAndExp->rhs = parse_eq_exp();

    while (tokens.at(currentPos) == TokenType::AND) {
        auto newLAndExp = std::make_unique<LAndExp>();
        newLAndExp->lhs = std::move(lAndExp);
        newLAndExp->op  = std::make_unique<Token>(std::move(tokens.at(currentPos)));
        currentPos++;
        newLAndExp->rhs = parse_eq_exp();
        lAndExp = std::move(newLAndExp);
    }

    return lAndExp;
}

std::unique_ptr<LOrExp> Parser::parse_l_or_exp() {
    auto lOrExp = std::make_unique<LOrExp>();
    lOrExp->lhs = nullptr;
    lOrExp->rhs = parse_l_and_exp();

    while (tokens.at(currentPos) == TokenType::OR) {
        auto newLOrExp = std::make_unique<LOrExp>();
        newLOrExp->lhs = std::move(lOrExp);
        newLOrExp->op  = std::make_unique<Token>(std::move(tokens.at(currentPos)));
        currentPos++;
        newLOrExp->rhs = parse_l_and_exp();
        lOrExp = std::move(newLOrExp);
    }

    return lOrExp;
}

std::unique_ptr<ConstExp> Parser::parse_const_exp() {
    auto constExpr = std::make_unique<ConstExp>();
    constExpr->add_exp = parse_add_exp();
    return constExpr;
}

std::unique_ptr<ConstInitVal> Parser::parse_const_init_val() {
    auto constInitVal = std::make_unique<ConstInitVal>();

    if (tokens.at(currentPos) == TokenType::STRCON) {
        constInitVal->type = ConstInitVal::stringConst;
        constInitVal->string_const =
            std::make_unique<Token>(std::move(tokens.at(currentPos)));
        currentPos++;
        return constInitVal;
    }

    if (tokens.at(currentPos) == TokenType::LBRACE) {
        constInitVal->type = ConstInitVal::bracedConstExp;
        currentPos++;
        if (tokens.at(currentPos) == TokenType::RBRACE) {
            currentPos++;
            return constInitVal;
        }
        constInitVal->const_exps.push_back(parse_const_exp());
        while (tokens.at(currentPos) == TokenType::COMMA) {
            currentPos++;
            constInitVal->const_exps.push_back(parse_const_exp());
        }
        assert(tokens.at(currentPos) == TokenType::RBRACE);
        currentPos++;
        return constInitVal;
    }

    constInitVal->type = ConstInitVal::constExp;
    constInitVal->const_exps.push_back(parse_const_exp());
    return constInitVal;
}

std::unique_ptr<InitVal> Parser::parse_init_val() {
    auto initVal = std::make_unique<InitVal>();

    if (tokens.at(currentPos) == TokenType::STRCON) {
        initVal->type = InitVal::stringConst;
        initVal->string_const = std::make_unique<Token>(std::move(tokens.at(currentPos)));
        currentPos++;
        return initVal;
    }

    if (tokens.at(currentPos) == TokenType::LBRACE) {
        initVal->type = InitVal::bracedExp;
        currentPos++;
        if (tokens.at(currentPos) == TokenType::RBRACE) {
            currentPos++;
            return initVal;
        }
        initVal->exps.push_back(parse_exp());
        while (tokens.at(currentPos) == TokenType::COMMA) {
            currentPos++;
            initVal->exps.push_back(parse_exp());
        }
        assert(tokens.at(currentPos) == TokenType::RBRACE);
        currentPos++;
        return initVal;
    }

    initVal->type = InitVal::exp;
    initVal->exps.push_back(parse_exp());
    return initVal;
}

std::unique_ptr<ConstDef> Parser::parse_const_def() {
    auto constDef = std::make_unique<ConstDef>();
    assert(tokens.at(currentPos) == TokenType::IDENFR);
    constDef->identifier = std::make_unique<Token>(std::move(tokens.at(currentPos)));
    currentPos++;

    if (tokens.at(currentPos) == TokenType::LBRACK) {
        currentPos++;
        constDef->const_expr = parse_const_exp();
        if (tokens.at(currentPos) == TokenType::RBRACK) {
            currentPos++;
        }
        else {
            errors.emplace_back('k', tokens.at(currentPos - 1).getLine());
        }
    }
    else {
        constDef->const_expr = nullptr;
    }

    assert(tokens.at(currentPos) == TokenType::ASSIGN);
    currentPos++;
    constDef->const_init_val = parse_const_init_val();
    return constDef;
}

std::unique_ptr<FuncType> Parser::parse_func_type() {
    auto funcType = std::make_unique<FuncType>();
    funcType->type = std::make_unique<Token>(std::move(tokens.at(currentPos)));
    assert(tokens.at(currentPos) == TokenType::VOIDTK ||
        tokens.at(currentPos) == TokenType::INTTK ||
        tokens.at(currentPos) == TokenType::CHARTK);
    currentPos++;
    return funcType;
}

std::unique_ptr<Stmt> Parser::parse_stmt() {
    Token currentToken = tokens.at(currentPos);
    if (currentToken == TokenType::LBRACE) {
        return std::make_unique<Stmt>(std::move(*parse_block()));
    }
    else if (currentToken == TokenType::IFTK) {
        return std::make_unique<Stmt>(std::move(*parse_if_stmt()));
    }
    else if (currentToken == TokenType::FORTK) {
        return std::make_unique<Stmt>(std::move(*parse_for_stmt()));
    }
    else if (currentToken == TokenType::BREAKTK) {
        return std::make_unique<Stmt>(std::move(*parse_break_stmt()));
    }
    else if (currentToken == TokenType::CONTINUETK) {
        return std::make_unique<Stmt>(std::move(*parse_continue_stmt()));
    }
    else if (currentToken == TokenType::RETURNTK) {
        return std::make_unique<Stmt>(std::move(*parse_return_stmt()));
    }
    else if (currentToken == TokenType::PRINTFTK) {
        return std::make_unique<Stmt>(std::move(*parse_printf_stmt()));
    }
    else {
        return std::make_unique<Stmt>(std::move(*parse_l_val_wrap_stmt()));
    }
}

std::unique_ptr<LValExp> Parser::parse_l_val_exp() {
    auto lValExp = std::make_unique<LValExp>();

    if (tokens.at(currentPos) == TokenType::GETINTTK) {
        currentPos++;
        assert(tokens.at(currentPos) == TokenType::LPARENT);
        currentPos++;
        if (tokens.at(currentPos) == TokenType::RPARENT) {
            currentPos++;
        }
        else {
            errors.emplace_back('j', tokens.at(currentPos - 1).getLine());
        }
        GetIntExp getIntExp;
        return std::make_unique<LValExp>(std::move(getIntExp));
    }

    if (tokens.at(currentPos) == TokenType::GETCHARTK) {
        currentPos++;
        assert(tokens.at(currentPos) == TokenType::LPARENT);
        currentPos++;
        if (tokens.at(currentPos) == TokenType::RPARENT) {
            currentPos++;
        }
        else {
            errors.emplace_back('j', tokens.at(currentPos - 1).getLine());
        }
        GetCharExp getCharExp;
        return std::make_unique<LValExp>(std::move(getCharExp));
    }

    return std::make_unique<LValExp>(std::move(*parse_exp()));
}

std::unique_ptr<Cond> Parser::parse_cond() {
    Cond cond;
    cond.l_or_exp = parse_l_or_exp();
    return std::make_unique<Cond>(std::move(cond));
}

std::unique_ptr<IfStmt> Parser::parse_if_stmt() {
    assert(tokens.at(currentPos) == TokenType::IFTK);
    currentPos++;
    assert(tokens.at(currentPos) == TokenType::LPARENT);
    currentPos++;
    auto ifStmt = std::make_unique<IfStmt>();
    ifStmt->cond = parse_cond();
    if (tokens.at(currentPos) == TokenType::RPARENT) {
        currentPos++;
    }
    else {
        errors.emplace_back('j', tokens.at(currentPos - 1).getLine());
    }

    ifStmt->if_stmt = parse_stmt();
    if (tokens.at(currentPos) == TokenType::ELSETK) {
        currentPos++;
        ifStmt->else_stmt = parse_stmt();
    }
    else {
        ifStmt->else_stmt = nullptr;
    }
    return ifStmt;
}

std::unique_ptr<ForStmt_> Parser::parse_for_stmt_() {
    auto forStmt_ = std::make_unique<ForStmt_>();
    forStmt_->l_val = parse_l_val();
    assert(tokens.at(currentPos) == TokenType::ASSIGN);
    currentPos++;
    forStmt_->exp = parse_exp();
    return forStmt_;
}

std::unique_ptr<ForStmt> Parser::parse_for_stmt() {
    assert(tokens.at(currentPos) == TokenType::FORTK);
    currentPos++;
    assert(tokens.at(currentPos) == TokenType::LPARENT);
    currentPos++;

    auto forStmt = std::make_unique<ForStmt>();
    if (tokens.at(currentPos) == TokenType::SEMICN) {
        forStmt->init = nullptr;
        currentPos++;
    }
    else {
        forStmt->init = parse_for_stmt_();
        assert(tokens.at(currentPos) == TokenType::SEMICN);
        currentPos++;
    }

    if (tokens.at(currentPos) == TokenType::SEMICN) {
        forStmt->cond = nullptr;
        currentPos++;
    }
    else {
        forStmt->cond = parse_cond();
        assert(tokens.at(currentPos) == TokenType::SEMICN);
        currentPos++;
    }

    if (tokens.at(currentPos) == TokenType::RPARENT) {
        forStmt->loop_stmt = nullptr;
        currentPos++;
    }
    else {
        forStmt->loop_stmt = parse_for_stmt_();
        assert(tokens.at(currentPos) == TokenType::RPARENT);
        //TODO FOR WITH NO ERROR
        currentPos++;
    }

    forStmt->body_stmt = parse_stmt();
    return forStmt;
}

std::unique_ptr<BreakStmt> Parser::parse_break_stmt() {
    assert(tokens.at(currentPos) == TokenType::BREAKTK);
    currentPos++;
    if (tokens.at(currentPos) == TokenType::SEMICN) {
        currentPos++;
    }
    else {
        errors.emplace_back('i', tokens.at(currentPos - 1).getLine());
    }
    return std::make_unique<BreakStmt>();
}

std::unique_ptr<ContinueStmt> Parser::parse_continue_stmt() {
    assert(tokens.at(currentPos) == TokenType::CONTINUETK);
    currentPos++;
    if (tokens.at(currentPos) == TokenType::SEMICN) {
        currentPos++;
    }
    else {
        errors.emplace_back('i', tokens.at(currentPos - 1).getLine());
    }
    return std::make_unique<ContinueStmt>();
}

std::unique_ptr<ReturnStmt> Parser::parse_return_stmt() {
    assert(tokens.at(currentPos) == TokenType::RETURNTK);
    currentPos++;
    auto returnStmt = std::make_unique<ReturnStmt>();

    if (can_be_exp()) {
        returnStmt->exp = parse_exp();
    }
    else {
        returnStmt->exp = nullptr;
    }

    if (tokens.at(currentPos) == TokenType::SEMICN) {
        currentPos++;
    }
    else {
        errors.emplace_back('i', tokens.at(currentPos - 1).getLine());
    }

    return returnStmt;
}

std::unique_ptr<PrintfStmt> Parser::parse_printf_stmt() {
    auto printfStmt = std::make_unique<PrintfStmt>();
    assert(tokens.at(currentPos) == TokenType::PRINTFTK);
    currentPos++;
    assert(tokens.at(currentPos) == TokenType::LPARENT);
    currentPos++;
    assert(tokens.at(currentPos) == TokenType::STRCON);
    printfStmt->string_const = std::make_unique<Token>(tokens.at(currentPos));
    currentPos++;

    while (tokens.at(currentPos) == TokenType::COMMA) {
        currentPos++;
        printfStmt->exps.push_back(parse_exp());
    }

    if (tokens.at(currentPos) == TokenType::RPARENT) {
        currentPos++;
    }
    else {
        errors.emplace_back('j', tokens.at(currentPos - 1).getLine());
    }

    if (tokens.at(currentPos) == TokenType::SEMICN) {
        currentPos++;
    }
    else {
        errors.emplace_back('i', tokens.at(currentPos - 1).getLine());
    }
    return printfStmt;
}

std::unique_ptr<LValWrapStmt> Parser::parse_l_val_wrap_stmt() {
    auto lValWrapStmt = std::make_unique<LValWrapStmt>();

    if (!can_be_exp()) {
        ExpStmt exp_stmt;
        exp_stmt.exp = nullptr;
        lValWrapStmt->l_val_stmt = std::make_unique<LValStmt>(std::move(exp_stmt));
        if (tokens.at(currentPos) == TokenType::SEMICN) {
            currentPos++;
        }
        else {
            errors.emplace_back('i', tokens.at(currentPos - 1).getLine());
        }
        return lValWrapStmt;
    }

    auto exp = parse_exp();//perceive as exp

    if (tokens.at(currentPos) == TokenType::ASSIGN) {
        auto assign_stmt = std::make_unique<AssignStmt>();
        auto primary_unary_exp =
            std::get<PrimaryUnaryExp>(std::move(*exp->add_exp->rhs->rhs));
        assign_stmt->l_val =
            std::make_unique<LVal>(std::move(
                std::get<LVal>(std::move(*primary_unary_exp.primary))));
        currentPos++;
        assign_stmt->l_val_exp = parse_l_val_exp();
        if (tokens.at(currentPos) == TokenType::SEMICN) {
            currentPos++;
        }
        else {
            errors.emplace_back('i', tokens.at(currentPos - 1).getLine());
        }
        lValWrapStmt->l_val_stmt = std::make_unique<LValStmt>(std::move(*assign_stmt));
        return lValWrapStmt;
    }

    if (tokens.at(currentPos) == TokenType::SEMICN) {
        currentPos++;
    }
    else {
        errors.emplace_back('i', tokens.at(currentPos - 1).getLine());
    }

    ExpStmt exp_stmt;
    exp_stmt.exp = std::move(exp);
    lValWrapStmt->l_val_stmt = std::make_unique<LValStmt>(std::move(exp_stmt));
    return lValWrapStmt;
}

std::unique_ptr<BlockItem> Parser::parse_block_item() {
    if (tokens.at(currentPos) == TokenType::CONSTTK ||
        tokens.at(currentPos) == TokenType::INTTK ||
        tokens.at(currentPos) == TokenType::CHARTK) {
        return std::make_unique<BlockItem>(std::move(*parse_decl()));
    }

    const auto stmt = parse_stmt();

    #ifdef DEBUG_PARSER
        std::visit(
            [&](auto &arg) {
                arg.print(std::cout);
            },
            *stmt);
        std::cout << "<Stmt>" << std::endl;
    #endif

    return std::make_unique<BlockItem>(std::move(*stmt));
}

std::unique_ptr<Block> Parser::parse_block() {
    auto block = std::make_unique<Block>();
    assert(tokens.at(currentPos) == TokenType::LBRACE);
    currentPos++;

    while (!(tokens.at(currentPos) == TokenType::RBRACE)) {
        block->block_items.push_back(parse_block_item());
    }

    assert(tokens.at(currentPos) == TokenType::RBRACE);
    currentPos++;
    return block;
}

std::unique_ptr<FuncDef> Parser::parse_func_def() {
    auto funcDef = std::make_unique<FuncDef>();
    funcDef->func_type = parse_func_type();

    assert(tokens.at(currentPos) == TokenType::IDENFR);
    funcDef->identifier = std::make_unique<Token>(tokens.at(currentPos));
    currentPos++;

    assert(tokens.at(currentPos) == TokenType::LPARENT);
    currentPos++;

    assert(currentPos + 1 < tokens.size());
    if ((tokens.at(currentPos) == TokenType::INTTK ||
        tokens.at(currentPos) == TokenType::CHARTK) &&
        tokens.at(currentPos + 1) == TokenType::IDENFR) {
        funcDef->func_f_params = parse_func_f_params();
    }
    else {
        funcDef->func_f_params = nullptr;
    }

    if (tokens.at(currentPos) == TokenType::RPARENT) {
        currentPos++;
    }
    else {
        errors.emplace_back('j', tokens.at(currentPos - 1).getLine());
    }

    funcDef->block = parse_block();

    return funcDef;
}

std::unique_ptr<MainFuncDef> Parser::parse_main_func_def() {
    assert(tokens.at(currentPos) == TokenType::INTTK);
    currentPos++;
    assert(tokens.at(currentPos) == TokenType::MAINTK);
    currentPos++;
    assert(tokens.at(currentPos) == TokenType::LPARENT);
    currentPos++;
    auto mainFuncDef = std::make_unique<MainFuncDef>();
    if (tokens.at(currentPos) == TokenType::RPARENT) {
        currentPos++;
    }
    else {
        errors.emplace_back('j', tokens.at(currentPos - 1).getLine());
    }
    mainFuncDef->block = parse_block();
    return mainFuncDef;
}

bool Parser::is_decl_in_comp_unit() const {
    assert(currentPos + 3 < tokens.size());
    if (tokens.at(currentPos) == TokenType::CONSTTK) {
        return true;
    }
    return !(tokens.at(currentPos + 2) == TokenType::LPARENT);
}

bool Parser::is_func_def_in_comp_unit() const {
    assert(currentPos + 3 < tokens.size());
    if (tokens.at(currentPos) == TokenType::CHARTK ||
        tokens.at(currentPos) == TokenType::VOIDTK) {
        return true;
    }
    return !(tokens.at(currentPos + 1) == TokenType::MAINTK);
}

bool Parser::can_be_exp() const {
    return tokens.at(currentPos) == TokenType::LPARENT ||
        tokens.at(currentPos) == TokenType::IDENFR ||
        tokens.at(currentPos) == TokenType::INTCON ||
        tokens.at(currentPos) == TokenType::CHRCON ||
        tokens.at(currentPos) == TokenType::PLUS ||
        tokens.at(currentPos) == TokenType::MINU;
}
