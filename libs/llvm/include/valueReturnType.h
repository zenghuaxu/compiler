//
// Created by LENOVO on 2024/11/7.
//

#ifndef VALUERETURNTYPE_H
#define VALUERETURNTYPE_H
#include <iostream>

#include "llvm.h"

class ValueReturnType {
    friend class LLVMContext;

    public:
    LLVMContextPtr getContext() {
        return context;
    }
    virtual bool isInt() = 0;
    virtual void print(std::ostream &out) = 0;
    virtual ValueReturnTypePtr get_ele_type() = 0;

    protected:
    ValueReturnType(unsigned int type_id, LLVMContextPtr context):
        type_id(type_id), context(context) {}

    private:
    unsigned int type_id;
    LLVMContextPtr context;
};

class CharType : public ValueReturnType {
    friend class LLVMContext;

    public:
    bool isInt() override {return false;}
    void print(std::ostream &out) override {
        out << "i8";
    }
    ValueReturnTypePtr get_ele_type() override { return this; }

    protected:
    CharType(unsigned int type_id, LLVMContextPtr context):
        ValueReturnType(type_id, context) {}
};

class IntType : public ValueReturnType {
    friend class LLVMContext;

    public:
    bool isInt() override {return true;}
    void print(std::ostream &out) override {
        out << "i32";
    }
    ValueReturnTypePtr get_ele_type() override { return this; }

    private:
    IntType(unsigned int type_id, LLVMContextPtr context):
        ValueReturnType(type_id, context) {}
};

class VoidType : public ValueReturnType {
    friend class LLVMContext;

    public:
    bool isInt() override {return false;}
    void print(std::ostream &out) override {
        out << "void";
    }
    ValueReturnTypePtr get_ele_type() override { return this; }

    private:
    VoidType(unsigned int type_id, LLVMContextPtr context):
        ValueReturnType(type_id, context) {}
};

class PointerType : public ValueReturnType {
    friend class LLVMContext;

    public:
    bool isInt() override {return false;}
    ValueReturnTypePtr get_referenced_type() { return referenced_type; }
    void print(std::ostream &out) override {
        referenced_type->print(out);
        out << '*';
    }
    ValueReturnTypePtr get_ele_type() override { return this; }

    protected:
    PointerType(unsigned int type_id, LLVMContextPtr context, ValueReturnType* referenced_type):
        ValueReturnType(type_id, context), referenced_type(referenced_type) {}

    private:
    ValueReturnTypePtr referenced_type;
};

class ValueArrayType : public ValueReturnType {
    friend class LLVMContext;

    public:
    bool isInt() override {return false;}
    [[nodiscard]] unsigned int get_size() const { return size; }
    void print(std::ostream &out) override {
        out << '[';
        out << size;
        out << " x ";
        element_type->print(out);
        out << ']';
    }
    ValueReturnTypePtr get_ele_type() override { return element_type; }

    ValueReturnTypePtr get_element_type() { return element_type; }

    protected:
    ValueArrayType(unsigned int type_id, LLVMContextPtr context, ValueReturnTypePtr element_type, unsigned int size):
        ValueReturnType(type_id, context), element_type(element_type), size(size) {}

    private:
    ValueReturnTypePtr element_type;
    unsigned int size;
};

//
// class FunctType : public ValueReturnType {
//     friend class LLVMContext;
//
//     public:
//     bool isInt() override {return false;}
//     protected:
//     FunctType(unsigned int type_id, LLVMContextPtr context,
//         ValueReturnType* return_type, std::vector<ValueReturnTypePtr> &parameter_types):
//             ValueReturnType(type_id, context),
//             return_type(return_type), arguments(parameter_types) {}
//
//     private:
//     ValueReturnType* return_type;
//     std::vector<ValueReturnTypePtr> arguments;
// };

#endif //VALUERETURNTYPE_H
