//
// Created by LENOVO on 2024/12/10.
//
#include <algorithm>
#include "../../include/ir/user.h"

void User::delete_use(UsePtr use) {
    auto it
        = std::find(use_list.begin(), use_list.end(), use);
    use_list.erase(it);
    (*it)->getValue()->delete_user(this);
}
