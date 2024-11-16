//
// Created by LENOVO on 2024/11/12.
//
#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H
#include <unordered_map>
#include <cassert>

#include "../../../frontend/include/token.h"
#include "../llvm.h"
#include "function.h"
#include "user.h"
#include "../llvmContext.h"
#include "../valueReturnType.h"

class Instruction: public User {
    friend class BasicBlock;

    public:
    Instruction(ValueReturnTypePtr return_type, ValueType type, BasicBlockPtr basic_block);

    ~Instruction() override = default;

    void print(std::ostream &out) override {
        out << "%" << id;
    }
    virtual void print_full(std::ostream &out) = 0;

    private:
    unsigned int id = 0;
    void mark_id(unsigned int &id_alloc);

    protected:
    unsigned int get_id() {
        return id;
    }
};

enum class UnaryOpType {NEG, NOT};

inline std::unordered_map<TokenType, UnaryOpType> tokentype_to_unary_op = {
        {TokenType::MINU, UnaryOpType::NEG},
        {TokenType::NOT, UnaryOpType::NOT},
};

class UnaryOpInstruction: public Instruction {
    public:
    explicit UnaryOpInstruction(ValueReturnTypePtr return_type, TokenType op,
        ValuePtr value, BasicBlockPtr basic_block):
        Instruction(return_type, ValueType::UnaryInst, basic_block) {
        try {this->op = tokentype_to_unary_op[op];} catch(std::invalid_argument& e) {std::cout << e.what() << '\n';}
        add_use(new Use(this, value));
    }

    void print_full(std::ostream &out) override {
        switch (this->op) {
            case UnaryOpType::NEG: {
                print(out);
                out << " = sub i32 0, ";
                use_list.at(0)->getValue()->print(out);
                break;
            }
            default: {
                print(out);
                out << " = icmp eq i32 0, ";
                use_list.at(0)->getValue()->print(out);
                break;
            }
        }
    }

    private:
    UnaryOpType op;
};

enum class BinaryOp {ADD, SUB, MUL, DIV, MOD};

inline std::unordered_map<TokenType, BinaryOp> tokentype_to_binary_op = {
        {TokenType::PLUS, BinaryOp::ADD},
        {TokenType::MINU, BinaryOp::SUB},
        {TokenType::MULT, BinaryOp::MUL},
        {TokenType::DIV, BinaryOp::DIV},
        {TokenType::MOD, BinaryOp::MOD},
};

inline std::unordered_map<BinaryOp, std::string> binary_op_to_string = {
    {BinaryOp::ADD, "add"},
    {BinaryOp::SUB, "sub"},
    {BinaryOp::MUL, "mul"},
    {BinaryOp::DIV, "sdiv"},
    {BinaryOp::MOD, "srem"},
};

class BinaryInstruction: public Instruction {
    public:
    BinaryInstruction(ValueReturnTypePtr return_type, TokenType op, ValuePtr lhs, ValuePtr rhs,
                  BasicBlockPtr basic_block);

    ValuePtr get_lhs() {
        return use_list.at(0)->getValue();
    }

    ValuePtr get_rhs() {
        return use_list.at(1)->getValue();
    }

    BinaryOp getBinaryOp() { return op; }

    void print_full(std::ostream &out) override {
        //TODO JUST MUNUS, ELSE TO JUMP
        print(out);
        out << " = " << binary_op_to_string[op] <<" i32 ";
        use_list.at(0)->getValue()->print(out);
        out << ", ";
        use_list.at(1)->getValue()->print(out);
    }

    private:
    BinaryOp op;
};

enum class CompOp {EQ, NE, SGT, SGE, SLT, SLE};

inline std::unordered_map<TokenType, CompOp> tokentype_to_comp_op = {
    {TokenType::EQL, CompOp::EQ},
    {TokenType::NEQ, CompOp::NE},
    {TokenType::GRE, CompOp::SGT},
    {TokenType::GEQ, CompOp::SGE},
    {TokenType::LSS, CompOp::SLT},
    {TokenType::LEQ, CompOp::SLE},
};

inline std::unordered_map<CompOp, std::string> comp_op_to_string = {
    {CompOp::EQ, "eq"},
    {CompOp::NE, "ne"},
    {CompOp::SGT, "sgt"},
    {CompOp::SGE, "sge"},
    {CompOp::SLT, "slt"},
    {CompOp::SLE, "sle"},
};

class CompareInstruction: public Instruction {
    public:
    CompareInstruction(ValueReturnTypePtr return_type, TokenType op, ValuePtr lhs, ValuePtr rhs,
                  BasicBlockPtr basic_block);

    void print_full(std::ostream &out) override {
        print(out);
        out << " = icmp " << comp_op_to_string.at(op) <<" ";
        comp_type->print(out);
        out << " ";
        use_list.at(0)->getValue()->print(out);
        out << ", ";
        use_list.at(1)->getValue()->print(out);
    }

    private:
    CompOp op;
    ValueReturnTypePtr comp_type;
};

class BranchInstruction: public Instruction {
    public:
    BranchInstruction(ValuePtr condition, BasicBlockPtr true_block, BasicBlockPtr false_block,
        BasicBlockPtr current_block);

    void print_full(std::ostream &out) override {
        out << "br i1 ";
        use_list.at(0)->getValue()->print(out);
        out << ", label %";
        use_list.at(1)->getValue()->print(out);
        out << ", label %";
        use_list.at(2)->getValue()->print(out);
    }
};

class JumpInstruction:public Instruction {
    public:
    JumpInstruction(BasicBlockPtr jump_block, BasicBlockPtr current_block);

    void print_full(std::ostream &out) override {
        out << "br label %";
        use_list.at(0)->getValue()->print(out);
    }
};

class AllocaInstruction: public Instruction {
public:
    //the return_type is the object type, not the pointer

    void print_full(std::ostream &out) override {
        print(out);
        out << " = alloca ";
        object_type->print(out);
    }

    AllocaInstruction(ValueReturnTypePtr return_type, BasicBlockPtr basic_block);

    private:
    ValueReturnTypePtr object_type;
};

class ZextInstruction: public Instruction {
    public:
    ZextInstruction(ValuePtr value, BasicBlockPtr basic_block);

    void print_full(std::ostream &out) override {
        print(out);
        out << " = zext ";
        auto value = use_list.at(0)->getValue();
        value->get_value_return_type()->print(out);
        out << " ";
        value->print(out);
        out << " to ";
        get_value_return_type()->print(out);
    }
};

class TruncInstruction: public Instruction {
    public:

    void print_full(std::ostream &out) override {
        print(out);
        out << " = trunc ";
        auto value = use_list.at(0)->getValue();
        value->get_value_return_type()->print(out);
        out << " ";
        value->print(out);
        out << " to ";
        get_value_return_type()->print(out);
    }

    TruncInstruction(ValuePtr value, BasicBlockPtr basic_block);
};

class CallInstruction: public Instruction {
    public:
    CallInstruction(ValueReturnTypePtr return_type, FunctionPtr function, BasicBlockPtr basic_block):
        Instruction(return_type, ValueType::CallInst, basic_block), function(function) {}

    void insert_parameter(ValuePtr new_value) {
        this->add_use(new Use(this, new_value));
    }

    void print_full(std::ostream &out);

private:
    FunctionPtr function;
};

class ReturnInstruction: public Instruction {
public:
    ReturnInstruction(ValueReturnTypePtr return_type, FunctionPtr function, ValuePtr ret_val, BasicBlockPtr basic_block):
        Instruction(return_type, ValueType::ReturnInst, basic_block) {
        if (ret_val) {
            this->add_use(new Use(this, ret_val));
        }
        this->function = function;
    }

    void print_full(std::ostream &out);
    private:
    FunctionPtr function;
};

class LoadInstruction: public Instruction {
    public:

    void print_full(std::ostream &out) override {
        print(out);
        out << " = load ";
        get_value_return_type()->print(out);
        out << ", ";
        get_value_return_type()->print(out);
        out << "* ";
        use_list.at(0)->getValue()->print(out);
    }

    LoadInstruction(ValuePtr value, BasicBlockPtr basic_block);
};

class StoreInstruction: public Instruction {
    public:

    void print_full(std::ostream &out) override {
        out << "store ";
        auto value = use_list.at(0)->getValue();
        value->get_value_return_type()->print(out);
        out << " ";
        value->print(out);
        out << ", ";
        auto addr = use_list.at(1)->getValue();
        addr->get_value_return_type()->print(out);
        out << " ";
        addr->print(out);
    }

    StoreInstruction(ValuePtr value, ValuePtr addr, BasicBlockPtr basic_block);
};

class GetElementPtrInstruction: public Instruction {
    public:
    GetElementPtrInstruction(ValuePtr base, ValuePtr offset, BasicBlockPtr basic_block);

    void print_full(std::ostream &out) override {
        print(out);
        out << " = getelementptr ";
        assert(typeid(*right_val) == typeid(PointerType));
        auto ele_type = dynamic_cast<PointerTypePtr>(right_val)
            ->get_referenced_type();
        ele_type->print(out);
        out << ", ";
        right_val->print(out);
        out << " ";
        auto base = use_list.at(0)->getValue();
        base->print(out);
        out << ", i32 ";
        if (typeid(*dynamic_cast<PointerTypePtr>(right_val)
            ->get_referenced_type()) == typeid(ValueArrayType)) {
            out << "0, i32 ";
        }
        auto offset = use_list.at(1)->getValue();
        offset->print(out);
    }

    private:
    ValueReturnTypePtr right_val;
};

class InputInstruction: public Instruction {
    public:
    explicit InputInstruction(ValueReturnTypePtr return_type, BasicBlockPtr basic_block, bool ch_type):
        Instruction(return_type, ValueType::InputInst, basic_block), ch_type(ch_type) {}

    void print_full(std::ostream &out) override {
        print(out);
        if (ch_type) {
            out << " = call i32 @getchar()";
        }
        else {
            out << " = call i32 @getint()";
        }
    }

    private:
    bool ch_type;
};

class OutputInstruction: public Instruction {
    public:
    //put char: char ;int: int :str: char*
    OutputInstruction(ValuePtr value, BasicBlockPtr basic_block);
    OutputInstruction(ValuePtr value, BasicBlockPtr basic_block, bool ch);

    void print_full(std::ostream &out) override {
        out << "call void @put";
        auto type = value_return_type;
        auto value = use_list.at(0)->getValue();
        if (typeid(*type) == typeid(PointerType)) {
            out << "str(i8* ";
            value->print(out);
            out << ")";
        }
        else if (ch) {
            out << "ch(i32 ";
            value->print(out);
            out << ")";
        }
        else {
            out << "int(i32 ";
            value->print(out);
            out << ")";
        }
    }

    private:
    ValueReturnTypePtr value_return_type;
    bool ch;
};

#endif //INSTRUCTIONS_H
