//
// Created by LENOVO on 2024/11/21.
//

#ifndef MIPSREG_H
#define MIPSREG_H
#include <iostream>

#include "../../llvm/include/llvm.h"
#include "mips.h"

class Reg {
public:
    virtual ~Reg() = default;
    Reg() = default;
    virtual void print(std::ostream &out) = 0;

    virtual int get_id() = 0;
};

class SpReg: public Reg {
public:
    int get_id() override {return 0;}
    void print(std::ostream &out) override {
        out << "$sp";
    }
};

class DynamicOffset {
public:
    DynamicOffset(int origin_offset): sp_offset(origin_offset) {}

    void insert_offset(int offset) {
        sp_offset += offset;
    }

    void align_with(int a) {
        if (sp_offset % a) {
            sp_offset += a - sp_offset % a;
        }
    }

    [[nodiscard]] int get_offset() const { return sp_offset; }

private:
    int sp_offset;
};

class MemOffset: public Reg {
public:
    MemOffset(DynamicOffsetPtr offset, int byte_size, int align_size):
        dynamic_offset(offset), align_size(align_size) {
        offset->align_with(align_size);
        offset->insert_offset(byte_size);
        object_offset = offset->get_offset();
    }
    MemOffset(bool new_pos, DynamicOffsetPtr offset, int byte_pos, int align_size):
        new_pos(new_pos), dynamic_offset(offset), object_offset(byte_pos), align_size(align_size) {}
    [[nodiscard]] int get_align_size() const { return align_size;}
    //NOTICE:: IS ALSO OBJECT SIZE !!!
    int get_id() override {return 0;}

    void print(std::ostream &out) override {
        if (new_pos) {
            out << -object_offset;
        }
        else {
            out << dynamic_offset->get_offset() - object_offset;
        }
    }

private:
    DynamicOffsetPtr dynamic_offset;
    int object_offset;
    int align_size;
    bool new_pos = false;
};

class TmpReg: public Reg {
public:
    explicit TmpReg(int id): id(id) {}
    int get_id() override {return id;}
    void mark_occupied() { occupied = true; }
    [[nodiscard]] bool check_occupied() const { return occupied; }
    void release_occupied() { occupied = false; }

    void print(std::ostream &out) override {
        out << "$t" << id;
    }
private:
    int id;
    bool occupied = false;
};

class SaveReg: public Reg {
public:
    explicit SaveReg(int id): id(id) {}
    int get_id() override {return id;}

    void print(std::ostream &out) override {
        out << "$s" << id;
    }
private:
    int id;
};

class SwapReg: public Reg {
public:
    explicit SwapReg(int id): id(id) {}
    int get_id() override { return id; }
    void mark_occupied() { occupied = true; }
    [[nodiscard]] bool check_occupied() const { return occupied; }
    void release_occupied() { occupied = false; }

    void print(std::ostream &out) override {
        out << "$t" << id + 8;
    }
private:
    int id;
    bool occupied = false;
};

class ZeroReg: public Reg {
    public:
    ZeroReg() = default;
    int get_id() override {return 0;}
    void print(std::ostream &out) override {
        out << "$0";
    }
private:
};

class RaReg: public Reg {
public:
    RaReg() = default;
    int get_id() override {return 0;}
    void print(std::ostream &out) override {
        out << "$ra";
    }
};

class AReg: public Reg {
    public:
    AReg(int id): id(id) {}
    int get_id() override {return id;}
    void print(std::ostream &out) override {
        out << "$a" << id;
    }
    private:
    int id;
};

class VReg: public Reg {
    public:
    VReg(int id): id(id) {}
    int get_id() override {return id;}
    void print(std::ostream &out) override {
        out << "$v" << id;
    }
    private:
    int id;
};

#endif //MIPSREG_H
