//
// Created by LENOVO on 2024/11/12.
//

#ifndef GLOBALVALUE_H
#define GLOBALVALUE_H
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

#include "value.h"


const std::unordered_map<int, std::string> value_to_char = {
    {7, "\\07"},
    {8, "\\08"},
    {9, "\\09"},
    {10, "\\0A"},
    {11, "\\0B"},
    {12, "\\0C"},
    {34, "\\22"},
    {39, "\'"},
    { 92, "\\\\"},
    {0, "\\00"},
};

class GlobalValue:public Value {
    friend class Translator;
    private:
    std::string init_string;
    std::vector<int> init_vector;
    //if using ordinary int, the uninitialized are all zero.

    public:
    GlobalValue(ValueReturnTypePtr return_type, std::string name);

    void set_string(std::string str) {
        init_string = std::move(str);
    }

    void set_int(int i) {
        init_vector.push_back(i);
    }

    ~GlobalValue() override = default;

    void print_all(std::ostream &out) {
        print(out);
        out << " = dso_local global ";
        global_value_type->print(out);
        out << " ";
        if (init_string.empty() && init_vector.empty()) {
            out << "zeroinitializer";
            out << std::endl;
            return;
        }
        if (typeid(*global_value_type) == typeid(ValueArrayType)) {
            auto type = dynamic_cast<ValueArrayTypePtr>(global_value_type);
            if (!init_vector.empty()) {
                out << "[";
                int i;
                for (i = 0; i < init_vector.size(); i++) {
                    type->get_element_type()->print(out);
                    out << " " << init_vector[i];
                    if (i != type->get_size() - 1) {
                        out << ", ";
                    }
                }
                for (; i < type->get_size(); i++) {
                    type->get_element_type()->print(out);
                    out << " 0";
                    if (i != type->get_size() - 1) {
                        out << ", ";
                    }
                }
                out << "]";
            }
            else {
                out << "c\"";
                print_str(out);
                for (int i = 0; i < type->get_size() - init_string.size(); i++) {
                    out << "\\00";
                }
                out << "\"";
            }
        }
        else {
            out << init_vector.at(0);
        }
        out << std::endl;
    }

    void print(std::ostream &out) override {
        out << "@" << name;
    }

    void print_str(std::ostream &out) {
        for (char i : init_string) {
            if (value_to_char.find(i) == value_to_char.end()) {
                out << i;
            }
            else {
                out << value_to_char.at(i);
            }
        }
    }

    private:
    ValueReturnTypePtr global_value_type;//real type
    //the type stored in value is pointer type
    std::string name;
};

#endif //GLOBALVALUE_H
