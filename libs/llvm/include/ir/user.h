//
// Created by LENOVO on 2024/11/12.
//

#ifndef USER_H
#define USER_H
#include <vector>

#include "../llvm.h"
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

    void delete_use(UsePtr use);

    void mark_active_for_dce();

    //TODO CHECK
    //REMOVE THE BREAK
    void replace_use(ValuePtr old_value, ValuePtr new_value) {
        for (auto & i : use_list) {
            if (i->getValue() == old_value) {
                i = new Use(this, new_value);
                new_value->add_user(this);
                //break;
            }
        }
    }

    void mark_child_active();

    ~User() override = default;

    protected:
    std::vector<UsePtr> use_list;
    private:
    bool visited = false;
};

#endif //USER_H
