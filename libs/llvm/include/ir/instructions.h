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
        //TODO JUST MUNUS, ELSE TO JUMP
        print(out);
        out << " = sub i32 0, ";
        use_list.at(0)->getValue()->print(out);
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
        this->add_use(new Use(this, ret_val));
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
    GetElementPtrInstruction(ValuePtr base, ValuePtr offset, BasicBlockPtr basic_block):
        Instruction(base->get_value_return_type(), ValueType::GetElementInst, basic_block) {
        this->add_use(new Use(this, base));
        this->add_use(new Use(this, offset));
    }

    void print_full(std::ostream &out) override {
        print(out);
        out << " = getelementptr ";
        assert(typeid(*get_value_return_type()) == typeid(PointerType));
        auto ele_type = dynamic_cast<PointerTypePtr>(get_value_return_type())
            ->get_referenced_type();
        ele_type->print(out);
        out << ", ";
        print_value_return_type(out);
        out << " ";
        auto base = use_list.at(0)->getValue();
        base->print(out);
        out << ", i32 ";
        auto offset = use_list.at(1)->getValue();
        offset->print(out);
    }
};

class InputInstruction: public Instruction {
    public:
    explicit InputInstruction(ValueReturnTypePtr return_type, BasicBlockPtr basic_block):
        Instruction(return_type, ValueType::InputInst, basic_block) {}

    void print_full(std::ostream &out) override {
        print(out);
        out << " = call i32 @getchar()";
    }
};

class OutputInstruction: public Instruction {
    public:
    //put char: char ;int: int :str: char*
    OutputInstruction(ValuePtr value, BasicBlockPtr  basic_block):
        Instruction(value->get_value_return_type(), ValueType::OutputInst, basic_block) {
        this->add_use(new Use(this, value));
    }

    void print_full(std::ostream &out) override {
        out << "call void @put";
        auto type = get_value_return_type();
        auto value = use_list.at(0)->getValue();
        if (typeid(*type) == typeid(PointerType)) {
            out << "str(i8* ";
            value->print(out);
            out << ")";
        }
        else if (typeid(*type) == typeid(IntType)) {
            out << "int(i32 ";
            value->print(out);
            out << ")";
        }
        else {
            out << "char(i8 ";
            value->print(out);
            out << ")";
        }
    }

    private:
    bool is_str;
};

#endif //INSTRUCTIONS_H
