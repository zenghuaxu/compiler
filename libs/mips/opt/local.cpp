//
// Created by LENOVO on 2024/12/19.
//

#include "../include/mipsManager.h"

void MipsManager::reduce_jump() {
    for (auto it = insts.begin(); it != insts.end(); it++) {
        if (it + 1 != insts.end()) {
            auto jump = dynamic_cast<JumpCode*>(*it);
            auto tag = dynamic_cast<Tag*>(*(it + 1));
            if (jump && tag) {
                if (jump->getLabelName() == tag->getName()) {
                    it = insts.erase(it);
                }
            }
        }
    }
}
