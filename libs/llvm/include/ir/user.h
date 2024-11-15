//
// Created by LENOVO on 2024/11/12.
//

#ifndef USER_H
#define USER_H
#include <memory>
#include <vector>

#include "use.h"
#include "value.h"

class User: public Value {
    public:
    User(ValueReturnTypePtr return_type, ValueType type):
    Value(return_type, type) {}

    void add_use(UsePtr use) {
        use_list.push_back(use);
        //TODO ADD INTO THE CONTEXT
        use->getValue()->add_user(this);
    }

    ~User() override = default;

    protected:
    std::vector<UsePtr> use_list;
};

#endif //USER_H
