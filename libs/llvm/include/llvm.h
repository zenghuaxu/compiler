//
// Created by LENOVO on 2024/11/7.
//

#ifndef LLVM_H
#define LLVM_H
#include <memory>

class Module;
using ModulePtr = std::shared_ptr<Module>;

class LLVMContext;
using LLVMContextPtr = LLVMContext*;

class Value;
using ValuePtr = Value*;

class ValueReturnType;
using ValueReturnTypePtr = ValueReturnType*;
class IntType;
using IntTypePtr = IntType*;
class CharType;
using CharTypePtr = CharType*;
class VoidType;
using VoidTypePtr = VoidType*;
class PointerType;
using PointerTypePtr = PointerType*;
class ValueArrayType;
using ValueArrayTypePtr = ValueArrayType*;

class User;
using UserPtr = User*;

class Instruction;
using InstructionPtr = Instruction*;
class UnaryOpInstruction;
using UnaryOpInstructionPtr = UnaryOpInstruction*;
class BinaryOpInstruction;
using BinaryOpInstructionPtr = BinaryOpInstruction*;
class AllocaInstruction;
using AllocaInstructionPtr = AllocaInstruction*;
class CallInstruction;
using CallInstructionPtr = CallInstruction*;
class LoadInstruction;
using LoadInstructionPtr = LoadInstruction*;
class StoreInstruction;
using StoreInstructionPtr = StoreInstruction*;
class ZextInstruction;
using ZextInstructionPtr = ZextInstruction*;
class TruncInstruction;
using TruncInstructionPtr = TruncInstruction*;

class BasicBlock;
using BasicBlockPtr = BasicBlock*;

class Function;
using FunctionPtr = Function*;

class Argument;
using ArgumentPtr = Argument*;

class GlobalValue;
using GlobalValuePtr = GlobalValue*;

class Constant;
using ConstantPtr = Constant*;

class Use;
using UsePtr = Use*;

#endif //LLVM_H
