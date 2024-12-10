//
// Created by LENOVO on 2024/11/24.
//
#include "../include/symtable.h"

#include "../../llvm/include/llvm.h"
#include "../../llvm/include/llvmContext.h"

ValueReturnTypePtr BasicType::toValueType(LLVMContextPtr context) {
    switch (type) {
        case void_type: return context->getVoidType();
        case int_type: return context->getIntType();
        case char_type: return context->getCharType();
    }
    return nullptr;
}

ValueReturnTypePtr ArrayType::toValueType(LLVMContextPtr context) {
    return context->getArrayType(_element_type->toValueType(context), _size);
}

ValueReturnTypePtr FunctionType::get_return_type(LLVMContextPtr context) {
    assert(typeid(*_return_type) == typeid(BasicType));
    auto type = std::dynamic_pointer_cast<BasicType>(_return_type);
    switch (type->get_basic_type()) {
        case BasicType::void_type: return context->getVoidType();
        case BasicType::int_type: return context->getIntType();
        case BasicType::char_type: return context->getCharType();
    }
    return nullptr;
}
