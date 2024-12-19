//
// Created by LENOVO on 2024/12/10.
//
#include <algorithm>
#include "../../include/ir/user.h"

#include "../../include/ir/instructions.h"

void User::delete_use(UsePtr use) {
    auto it
        = std::find(use_list.begin(), use_list.end(), use);
    use_list.erase(it);
    (*it)->getValue()->delete_user(this);
}

void User::mark_active_for_dce() {
    if (visited) {
        return;
    }
    visited = true;
    for (auto it:use_list) {
        auto user = dynamic_cast<Instruction*>(it->getValue());
        if (user) {
            user->mark_active_for_dce();
        }
        else {
            it->getValue()->mark_active_for_dce();
        }
    }
    active = true;
}
