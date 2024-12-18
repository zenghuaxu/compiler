//
// Created by LENOVO on 2024/12/18.
//

#ifndef OPT_H
#define OPT_H
#include <vector>


class Opt {
    public:
    static void generate_div(RegPtr rs, RegPtr rd, int imm, std::vector<MipsInstPtr> &insts, RegPtr swap1, RegPtr swap2,
                      RegPtr zero);
};

#endif //OPT_H
