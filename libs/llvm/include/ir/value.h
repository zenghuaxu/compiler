//
// Created by LENOVO on 2024/11/5.
//

#ifndef VALUE_H
#define VALUE_H
#include "vector"

#include "../llvm.h"
#include "../valueReturnType.h"

enum class ValueType {
    Argument,
    BasicBlock,

    Constant,

    Function,
    GlobalVariable,

    BinaryInst,
    CompareInst,
    BranchInst,
    JumpInst,
    ReturnInst,
    StoreInst,
    CallInst,
    InputInst,
    OutputInst,
    AllocaInst,
    LoadInst,
    GetElementInst,
    UnaryInst,
    ZextInst,
    TruncInst,
    PhiInst,
    PCInst,
    tmp,
};

class Value {
    friend class Translator;
    public:
    Value(ValueReturnTypePtr return_type, ValueType type):
        return_type(return_type), type(type) {
        #ifdef IR_DEBUG
        return_type->print(std::cout);
        std::cout << std::endl;
        #endif
    }

    virtual ~Value() = default;
    virtual void print(std::ostream& out) = 0;

    ValueReturnTypePtr get_value_return_type() { return return_type; }
    ValueType get_type() { return type; }
    LLVMContextPtr getContext() {return return_type->getContext();}
    void print_value_return_type(std::ostream& out) {
        return_type->print(out);
    }

    protected:
    //for translator, to see if end
    std::vector<UserPtr> user_list;


    public:
    void add_user(UserPtr user) {
        user_list.emplace_back(user);
    }
    void delete_user(UserPtr user);


    std::vector<UserPtr> get_user_list() { return user_list;}

    private:
    ValueReturnTypePtr return_type;
    ValueType type;
};

#endif //VALUE_H
