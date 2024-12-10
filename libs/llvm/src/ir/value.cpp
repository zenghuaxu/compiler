//
// Created by LENOVO on 2024/12/10.
//
# include <algorithm>
# include "../../include/ir/value.h"
# include "../../include/ir/user.h"

void Value::delete_user(UserPtr user)  {
    auto it = std::find(user_list.begin(), user_list.end(), user);
    user_list.erase(it);
}
