//
// Created by LENOVO on 2024/11/20.
//

#ifndef MIPS_H
#define MIPS_H
#include <iostream>
#include "../../llvm/include/llvm.h"

class Data;
using DataPtr = Data*;

class MipsManager;
using MipsManagerPtr = MipsManager*;
class Translator;
using TranslatorPtr = Translator*;

class MipsInst;
using MipsInstPtr = MipsInst*;
class Tag;
using TagPtr = Tag*;

class Reg;
using RegPtr = Reg*;
class SpReg;
using SpRegPtr = SpReg*;

class DynamicOffset;
using DynamicOffsetPtr = DynamicOffset*;
class MemOffset;
using MemOffsetPtr = MemOffset*;
class TmpReg;
using TmpRegPtr = TmpReg*;
class SaveReg;
using SaveRegPtr = SaveReg*;
class SwapReg;
using SwapRegPtr = SwapReg*;
class ZeroReg;
using ZeroRegPtr = ZeroReg*;
class RaReg;
using RaRegPtr = RaReg*;
class AReg;
using ARegPtr = AReg*;
class VReg;
using VRegPtr = VReg*;

class RCode;
using RCodePtr = RCode*;
class ICode;
using ICodePtr = ICode*;

#define TMP_NUM 8
#define SAVE_NUM 8
#define SWAP_NUM 2
#define A_REG_NUM 4
#define V_REG_NUM 2

#endif //MIPS_H
