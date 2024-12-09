//
// Created by LENOVO on 2024/11/14.
//
#include "../../include/ir/use.h"
#include "../../include/llvmContext.h"

Use::Use(UserPtr user, ValuePtr value):
    user(user), value(value) {
    value->getContext()->SaveUse(this);
}
