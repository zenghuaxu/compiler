//
// Created by LENOVO on 2024/11/14.
//

#include <utility>

#include "../../include/llvm.h"
#include "../../include/ir/globalValue.h"
#include "../../include/valueReturnType.h"
#include "../../include/llvmContext.h"

GlobalValue::GlobalValue(ValueReturnTypePtr return_type, std::string name):
    Value(return_type->getContext()->toPointerType(return_type)
    //array to pointer
    , ValueType::GlobalVariable) {
    return_type->getContext()->SaveValue<GlobalValue>(this);
    return_type->getContext()->SaveGlobal(this);
    global_value_type = return_type;
    this->name = std::move(name);
}
