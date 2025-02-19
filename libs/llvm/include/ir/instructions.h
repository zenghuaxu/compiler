//
// Created by LENOVO on 2024/11/12.
//
#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H
#include <unordered_map>
#include <cassert>
#include <set>

#include "../../../frontend/include/token.h"
#include "../llvm.h"
#include "user.h"
#include "../llvmContext.h"
#include "../valueReturnType.h"

class Instruction: public User {
    friend class BasicBlock;
    friend class Translator;

    public:
    Instruction(ValueReturnTypePtr return_type, ValueType type, BasicBlockPtr basic_block);

    ~Instruction() override = default;

    void mark_active(int i) {
        active_block_seq.insert(i);
    }

    void print(std::ostream &out) override {
        out << "%" << id;
    }
    virtual void print_full(std::ostream &out) = 0;

    std::set<int> get_active_block_seq() {
        return active_block_seq;
    }
    void add_conflict(Variable instruction) {
        conflicting_instructions.insert(instruction);
    }
    int get_conflict_count() {
        return conflicting_instructions.size();
    }
    bool contains_conflict(Variable instruction) {
        return conflicting_instructions.find(instruction) != conflicting_instructions.end();
    }
    BasicBlockPtr get_basic_block() {
        return basic_block;
    }
    void substitute_instruction(ValuePtr value);

    void substitute_instruction_for_lvn(ValuePtr value);

    void delete_myself();

private:
    unsigned int id = 0;
    void mark_id(unsigned int &id_alloc);
    BasicBlockPtr basic_block;

    //活跃的块
    std::set<int> active_block_seq;
    //跨块活跃
    bool is_global = false;
    //冲突的全局变量
    std::set<Variable> conflicting_instructions;
    int map_times = 0;

    bool add_map_and_try_release() {
        if (is_global) return false;
        map_times++;
        if (user_list.size() <= map_times) {
            int a = 1;
        }
        assert(user_list.size() > map_times);
        if (user_list.size() == map_times + 1) {
            return true;
        }
        return false;
    }

    bool try_release() {
        if (is_global) return false;
        assert(user_list.size() > map_times);
        if (user_list.size() == map_times + 1) {
            return true;
        }
        return false;
    }

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
    friend class Translator;
    friend class BasicBlock;

    public:
    UnaryOpInstruction(ValueReturnTypePtr return_type, TokenType op, ValuePtr value, BasicBlockPtr basic_block);
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
    friend class Translator;
    friend class BasicBlock;

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
    friend class Translator;
    friend class BasicBlock;

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

    CompOp get_comp_op() { return op; }

    private:
    CompOp op;
    ValueReturnTypePtr comp_type;
};

class BranchInstruction: public Instruction {
    friend class Translator;

    public:
    BranchInstruction(ValuePtr condition, BasicBlockPtr true_block, BasicBlockPtr false_block,
        BasicBlockPtr current_block);

    void substitute(BasicBlockPtr old_block, BasicBlockPtr new_block);

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
    friend class Translator;

    public:
    JumpInstruction(BasicBlockPtr jump_block, BasicBlockPtr current_block);

    void substitute(BasicBlockPtr old, BasicBlockPtr);

    void print_full(std::ostream &out) override {
        out << "br label %";
        use_list.at(0)->getValue()->print(out);
    }
};

class AllocaInstruction: public Instruction {
    friend class Translator;

public:
    //the return_type is the object type, not the pointer

    void print_full(std::ostream &out) override {
        print(out);
        out << " = alloca ";
        object_type->print(out);
    }

    AllocaInstruction(ValueReturnTypePtr return_type, BasicBlockPtr basic_block);
    ValueReturnTypePtr get_object_type() { return object_type; }

    private:
    ValueReturnTypePtr object_type;
};

class ZextInstruction: public Instruction {
    friend class Translator;
    friend class BasicBlock;

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
    friend class Translator;
    friend class BasicBlock;

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
    friend class Translator;

    public:

    void insert_parameter(ValuePtr new_value) {
        this->add_use(new Use(this, new_value));
    }

    CallInstruction(ValueReturnTypePtr return_type, FunctionPtr function, BasicBlockPtr basic_block);

    void print_full(std::ostream &out);

private:
    FunctionPtr function;
};

class ReturnInstruction: public Instruction {
    friend class Translator;

public:

    void print_full(std::ostream &out) override;

    ReturnInstruction(ValueReturnTypePtr return_type, FunctionPtr function, ValuePtr ret_val,
                      BasicBlockPtr basic_block);

private:
    FunctionPtr function;
};

class LoadInstruction: public Instruction {
    friend class Translator;

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
    friend class Translator;

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
    friend class Translator;
    friend class BasicBlock;

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
    friend class Translator;

    public:
    InputInstruction(ValueReturnTypePtr return_type, BasicBlockPtr basic_block, bool ch_type);

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
    friend class Translator;

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

class PhiInstruction: public Instruction {
    public:
    PhiInstruction(AllocaInstructionPtr alloca_inst, BasicBlockPtr basic_block);
    std::map<BasicBlockPtr, ValuePtr> get_options() { return options; }
    void add_option(ValuePtr value, BasicBlockPtr basic_block);
    void print_full(std::ostream &out) override;
    void replace_use(ValuePtr old, ValuePtr new_value);

private:
    AllocaInstructionPtr alloca;
    std::map<BasicBlockPtr, ValuePtr> options;
};

class PCNode;
using PCNodePtr = PCNode*;
class PCInstruction: public Instruction {
    friend class Translator;
public:
    explicit PCInstruction(BasicBlockPtr basic_block);
    void add_edge(ValuePtr from, PhiInstructionPtr to);
    std::set<InstructionPtr> get_def() {return def;}
    std::set<Variable> get_use() {return use;}

    void print_full(std::ostream &out) override;
private:
    std::set<InstructionPtr> def;
    std::set<Variable> use;
    std::vector<PCNode*> nodes;
};

class PCNode {
    friend class Translator;
    friend class PCInstruction;
public:
    explicit PCNode(ValuePtr value);
    bool operator==(ValuePtr other);
    void insert_child(PCNode* pc_node) {children.insert(pc_node);}
    void insert_ancestor(PCNode* pc_node) {ancestor.insert(pc_node);}
    void delete_child(PCNodePtr pc_node) {children.erase(pc_node);}
    void delete_ancestor(PCNodePtr pc_node) {children.erase(pc_node);}
private:
    ValuePtr value;
    std::set<PCNodePtr> children;
    std::set<PCNodePtr> ancestor;
};

#endif //INSTRUCTIONS_H
