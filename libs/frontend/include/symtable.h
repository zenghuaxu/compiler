//
// Created by LENOVO on 2024/10/14.
//

#ifndef SYMTABLE_H
#define SYMTABLE_H
#include <cassert>
#include <complex>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

#include "../../include/configure.h"
#include "token.h"
#include "../../llvm/include/llvm.h"
#include "../../llvm/include/llvmContext.h"

class SymType {
    public:
    virtual ~SymType() = default;
    SymType() = default;

    virtual bool match(std::shared_ptr<SymType> sym_type) = 0;
    virtual bool operator==(std::shared_ptr<SymType> sym_type) = 0;
    virtual std::shared_ptr<SymType> evaluate () = 0;
    virtual bool isConst() = 0;
    virtual bool isVoid() = 0;
    virtual bool isInt() = 0;
    virtual bool isInferable() = 0;
    virtual void set_inferable() = 0;
    virtual ValueReturnTypePtr toValueType(LLVMContextPtr context) = 0;
    virtual void print(std::ostream &out) = 0;
};

class BasicType final : public SymType {
    public:
    enum basic_type {
        int_type,
        char_type,
        void_type,
    };

    private:
    basic_type type;
    bool constant_obj;
    bool inferable;

    public:
    BasicType(const bool constant_obj, const TokenType token_type):
    constant_obj(constant_obj), inferable(constant_obj) {
        static std::map<TokenType, basic_type> basic_type_map = {
            {TokenType::INTTK, int_type},
            {TokenType::CHARTK, char_type},
            {TokenType::VOIDTK, void_type},
        };
        try {
            type = basic_type_map[token_type];
        } catch (const std::invalid_argument& e) {
            std::cout << e.what() << '\n';
        }
    };

    BasicType(const bool const_obj, basic_type type) :
    constant_obj(const_obj), type(type) {}

    bool match(std::shared_ptr<SymType> sym_type) override {
        return type != void_type &&
            typeid(*sym_type) == typeid(BasicType) &&
            std::dynamic_pointer_cast<BasicType>(sym_type)->type != void_type;
    }

    std::shared_ptr<SymType> evaluate() override {
        return std::make_shared<BasicType>(isConst(), int_type);
        //MAINTAIN THE ATTRIBUTE OF ISCONSTANT, CONVERT TO INT
    }

    bool isConst() override {
        return constant_obj;
    }

    bool isInferable() override {
        return inferable;
    }

    void set_inferable() override {
        inferable = false;
    }

    void print(std::ostream &out) override {
        if (constant_obj) {
            out << "Const";
        }
        switch (type) {
            case void_type: {out << "Void"; break;}
            case int_type: {out << "Int"; break;}
            case char_type: {out << "Char"; break;}
        }
    }

    bool isVoid() override {
        return type == void_type;
    }

    bool isInt() override {
        return type == int_type;
    }

    bool operator==(std::shared_ptr<SymType> other) override {
        if (typeid(*other) != typeid(BasicType)) {
            return false;
        }
        auto basic = std::dynamic_pointer_cast<BasicType>(other);
        return type == basic->type;
        //TODO ONLY TYPE THE SAME, CONSTANT ATTRIBUTE NOT
    }

    basic_type get_basic_type() {
        return type;
    }

    ValueReturnTypePtr toValueType(LLVMContextPtr context) override {
        switch (type) {
            case void_type: return context->getVoidType();
            case int_type: return context->getIntType();
            case char_type: return context->getCharType();
        }
        return nullptr;
    }
};

class ArrayType final : public SymType {
    private:
    std::shared_ptr<SymType> _element_type;
    unsigned int _size; //TODO ->value

    public:
    ArrayType(unsigned int size, std::shared_ptr<SymType> element_type):
    _element_type(std::move(element_type)), _size(size) {}

    bool match(std::shared_ptr<SymType> sym_type) override {
        if (typeid(*sym_type) != typeid(ArrayType)) {
            return false;
        }
        const auto array = std::dynamic_pointer_cast<ArrayType>(sym_type);
        return *array->_element_type == _element_type;
    }

    bool operator==(std::shared_ptr<SymType> sym_type) override {
        if (typeid(*sym_type) != typeid(ArrayType)) {
            return false;
        }
        auto basic = std::dynamic_pointer_cast<ArrayType>(sym_type);
        return *_element_type == basic->_element_type;
    }

    std::shared_ptr<SymType> evaluate() override {
        return std::make_shared<ArrayType>(*this);//Array -> Element, not simple evaluate
    }

    std::shared_ptr<SymType> element_type() {
        return _element_type;
    }

    bool isConst() override {
        return _element_type->isConst();
    }

    bool isInferable() override {
        return false;//value not ferable
    }

    void set_inferable() override {}

    bool isVoid() override {
        return false;
    }

    bool isInt() override {
        return false;
    }

    void print(std::ostream &out) override {
        _element_type->print(out);
        out << "Array";
    }

    ValueReturnTypePtr toValueType(LLVMContextPtr context) override {
        return context->getArrayType(_element_type->toValueType(context), _size);
    }

    unsigned int get_size() {
        return _size;
    }
};

class FunctionType : public SymType {
    private:
    std::shared_ptr<SymType> _return_type;
    std::vector<std::shared_ptr<SymType>> _params;

    public:
    explicit FunctionType(std::shared_ptr<SymType> return_type):
    _return_type(std::move(return_type)) {};

    void insert_param(const std::shared_ptr<SymType>& param_type) {
        _params.push_back(param_type);
    }

    bool match(std::shared_ptr<SymType> sym_type) override {return false;}//TODO

    bool operator==(std::shared_ptr<SymType> sym_type) override {
        if (typeid(*sym_type) != typeid(FunctionType)) {
            return false;
        }
        auto basic = std::dynamic_pointer_cast<FunctionType>(sym_type);
        return *_return_type == basic->_return_type; //return same
    }

    std::shared_ptr<SymType> evaluate() override {
        return _return_type;//Call, not simple evaluate
    }

    bool isConst() override {
        return false;
    }

    bool isInferable() override {
        return false;//value not ferable
    }

    void set_inferable() override {}

    void print(std::ostream &out) override {
        _return_type->print(out);
        out << "Func";
    }

    bool isVoid() override {
        return false;
    }

    bool isInt() override {
        return false;
    }

    std::vector<std::shared_ptr<SymType>> get_params() {
        return _params;
    }

    ValueReturnTypePtr get_return_type(LLVMContextPtr context) {
        assert(typeid(*_return_type) == typeid(BasicType));
        auto type = std::dynamic_pointer_cast<BasicType>(_return_type);
        switch (type->get_basic_type()) {
            case BasicType::void_type: return context->getVoidType();
            case BasicType::int_type: return context->getIntType();
            case BasicType::char_type: return context->getCharType();
        }
        return nullptr;
    }

    ValueReturnTypePtr toValueType(LLVMContextPtr context) override {
        return _return_type->toValueType(context);
    }
};

class Symbol {
    std::shared_ptr<SymType> type;
    std::string identifier;
    int line;
    unsigned int id;
    unsigned int table_id;
    ValuePtr addr_addr;
    ValuePtr object_addr;
    ConstantPtr const_value;

    public:
    Symbol(std::shared_ptr<SymType> type, std::string identifier, int line, unsigned int id):
    type(std::move(type)), identifier(std::move(identifier)), line(line),
    addr_addr(nullptr), object_addr(nullptr), const_value(nullptr),
    id(id) {};

    [[nodiscard]] std::string get_identifier() const {
        return identifier;
    }

    [[nodiscard]] std::shared_ptr<SymType> get_type() const {
        return type;
    }

    void insert_addr_addr(ValuePtr addr) {
        addr_addr = addr;
    }

    ValuePtr get_addr_addr() {
        return addr_addr;
    }

    void insert_addr(ValuePtr addr) {
        object_addr = addr;
    }

    ValuePtr get_addr() {
        return object_addr;
    }

    void insert_value(ConstantPtr value) {
        const_value = value;
    }

    ConstantPtr get_value() {
        return const_value;
    }
    void insert_parameter(const std::shared_ptr<SymType>& type) {
        #ifdef DEBUG_SEMANTIC
        std::cout << typeid(*this->type).name() << std::endl;
        #endif
        assert(typeid(*this->type) == typeid(FunctionType));
        const auto ptr = std::dynamic_pointer_cast<FunctionType>(this->type);
        ptr->insert_param(type);
    }

    void print(std::ostream &out) {
        out << identifier + " ";
        type->print(out);
    }
};

class SymTable: public std::enable_shared_from_this<SymTable>{
    public:

    SymTable(): father(nullptr), scope_id(1) {};

    SymTable(const std::shared_ptr<SymTable>& father, int id):
    father(father), scope_id(id) {
        #ifdef DEBUG_SEMANTIC
        std::cout << "sym_table:" + std::to_string(id) << std::endl;
        #endif
    };

    bool exist_in_table(const std::string& identifier) {
        return symbols.find(identifier) != symbols.end();
    }

    std::shared_ptr<Symbol> get_from_all_scopes(const std::string& identifier) {
        if (auto index = symbols.find(identifier); index != symbols.end()) {
            return symbols[identifier];
        }
        if (!father) {
            return nullptr;
        }
        return father->get_from_all_scopes(identifier);
    }

    bool exist_in_all_scopes(const std::string& identifier) {
        if (exist_in_table(identifier)) {
            return true;
        }
        if (father) {
            return father->exist_in_all_scopes(identifier);
        }
        return false;
    }

    bool add_symbol(const std::shared_ptr<Symbol>& symbol) {
        if (exist_in_table(symbol->get_identifier())) {
            return false;
        }
        symbols.insert({symbol->get_identifier(), symbol});
        ordered_symbol.push_back(symbol);
        #ifdef DEBUG_SEMANTIC
        std::cout << "add_symbol(" << symbol->get_identifier() << ")" << std::endl;
        #endif
        return true;
    }

    std::shared_ptr<SymTable> push_scope(int id) {
        auto new_scope = std::make_shared<SymTable>(shared_from_this(), id);
        return new_scope;
    }

    std::shared_ptr<SymTable> pop_scope() {
        return father;
    }

    void print_table(std::ostream &out) {
        for (const auto& symbol: ordered_symbol) {
            out << std::to_string(scope_id) + " ";
            symbol->print(out);
            out << std::endl;
        }
    }

    private:
    std::unordered_map<std::string, std::shared_ptr<Symbol>> symbols;
    std::vector<std::shared_ptr<Symbol>> ordered_symbol;
    std::shared_ptr<SymTable> father;
    int scope_id;
};

#endif //SYMTABLE_H
