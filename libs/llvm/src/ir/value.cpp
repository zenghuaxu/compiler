//
// Created by LENOVO on 2024/12/10.
//
# include <algorithm>
# include "../../include/ir/value.h"
# include "../../include/ir/user.h"

void Value::delete_user(UserPtr user)  {
    //TODO CHECK HAVE TO ONLY DELETE ONE
    auto it = std::find(user_list.begin(), user_list.end(), user);
    user_list.erase(it);
}

void Value::mark_active_for_dce() {
    active = true;
}

bool Value::get_active() {
    return active;
}