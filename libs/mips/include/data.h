//
// Created by LENOVO on 2024/11/19.
//

#ifndef DATA_H
#define DATA_H

#include <ostream>
#include <vector>
#include <unordered_map>

class Data {
  public:
    virtual ~Data() = default;

    Data() = default;
    void virtual put_value(int value) = 0;
    void virtual set_zero_num(int num) = 0;
    void virtual print(std::ostream &out) = 0;
};

class ByteData: public Data {
    public:
    ByteData(std::string &name): Data(), name(name), zero_initializer_num(0) {}

    void put_value(int value) override {
        data.push_back(value);
    }
    void set_zero_num(int num) override {
        zero_initializer_num = num;
    }

    void print(std::ostream& out) override {
        out << "\t" << name << ": .byte ";
        if (zero_initializer_num) {
            out << "0:" << zero_initializer_num;
        }
        else {
            out << data.at(0);
            for (auto it
                = data.begin() + 1; it != data.end(); it++) {
                out << ", " << *it;//TODO
            }
        }
    }

    private:
    int zero_initializer_num;
    std::vector<int> data;
    std::string name;
};

class WordData: public Data {
    public:
    WordData(std::string &name):Data(), name(name), zero_initializer_num(0) {}

    void put_value(int value) override {
        data.push_back(value);
    }
    void set_zero_num(int num) override {
        zero_initializer_num = num;
    }

    void print(std::ostream& out) override {
        out << "\t" << name << ": .word ";
        if (zero_initializer_num) {
            out << "0:" << zero_initializer_num;
        }
        else {
            out << data.at(0);
            for (auto it
                = data.begin() + 1; it != data.end(); it++) {
                out << ", " << *it;
            }
        }
    }

    private:
    int zero_initializer_num;
    std::vector<int> data;
    std::string name;
};

const std::unordered_map<int, std::string> mips_value_to_char = {
    {7, "\\a"},
    {8, "\\b"},
    {9, "\\t"},
    {10, "\\n"},
    {11, "\\v"},
    {12, "\\f"},
    {34, "\\\""},
    {39, "\\\'"},
    { 92, "\\\\"},
    {0, "\\0"},
};

class StringData: public Data {
    public:
    StringData(std::string &name, std::string &content):
       Data(), name(name), content(content) {}

    void print(std::ostream& out) override {
        out << "\t" << name << ": .asciiz\"";
        for(char & it : content) {
            if (mips_value_to_char.find(it) != mips_value_to_char.end()) {
                out << mips_value_to_char.at(it);
            }
            else {
                out << it;
            }
        }
        out << "\"";
    }
    void set_zero_num(int num) override {}//DO NOTHING

    void put_value(int value) override {}

    private:
    std::string name;
    std::string content;
};

#endif //DATA_H
