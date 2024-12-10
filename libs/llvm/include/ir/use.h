//
// Created by LENOVO on 2024/11/5.
//

#ifndef USE_H
#define USE_H
#include "../llvm.h"
#include "value.h"

class Use {
    public:
    Use(UserPtr user, ValuePtr value);

    public:
    UserPtr getUser() { return user; }
    ValuePtr getValue() {
        return value;
    }

    private:
    UserPtr user;
    ValuePtr value;
};

#endif //USE_H
