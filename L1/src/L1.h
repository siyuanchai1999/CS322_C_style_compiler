#pragma once

#include <vector>
#include <string>

namespace L1 {
  enum ItemType{
    item_registers,
    item_constant,
    item_memory,
    item_labels,
    item_aop,
    item_cmp
  };

  enum Register_type {rdi, rax, rsi, rdx, rcx, r8, r9, rbx, rbp, r10, r11, r12, r13, r14, r15, rsp};
  enum Register_8b_type {r10b, r11b, r12b, r13b, r14b, r15b, r8b, r9b, al, bpl, bl, cl, dil, dl, sil};
  enum AopType {plus_eq, minus_eq, times_eq, bitand_eq};
  enum CmpType {less, leq, eq, great, geq};
  enum ShiftType {left, right};

  struct Item{
    ItemType itemtype;
  };

  struct ItemRegister : Item {
    Register_type rType;
  };

  struct ItemConstant : Item {
    int64_t constVal;
  }; 

  struct ItemLabel : Item {
    std::string labelName;
  };

  struct ItemMemoryAccess : Item {
    Register_type rType;
    int64_t offset;
  };

  struct ItemAop : Item {
    AopType aopType;
  };

  struct ItemCmp : Item {
    Item * op1;
    Item * op2;
    CmpType cmptype;
  };




  // struct Item {

  //   std::string labelName;
  //   Register r;
  //   bool isARegister;
  // };


  enum InstType{
    inst_ret,
    inst_assign,
    inst_call,
    inst_label,
    inst_aop,
    inst_sop,
    inst_lea,
    inst_goto,
    inst_dec,
    inst_inc,
    inst_cjump
  };

  /*
   * Instruction interface.
   */
  struct Instruction{
    InstType type;
  };

  /*
   * Instructions.
   */
  struct Instruction_ret : Instruction{

  };

  struct Instruction_label : Instruction{
    std::string labelName;
  };

  struct Instruction_call : Instruction{
    bool isRuntimeCall;  
  };

  struct Instruction_call_runtime: Instruction_call {
    std::string callee;
    int arg_cnt;
  };

  struct Instruction_call_user: Instruction_call {
    Item * callee;
    Item * num_args;
  };

  struct Instruction_aop: Instruction {
    Item *op1, *op2;
    AopType aopType; 
  };

  struct Instruction_assignment : Instruction{
    Item *src, *dst;
  };

  struct Instruction_sop: Instruction {
    Item *target;
    Item *offset;
    ShiftType direction;
  };

  struct Instruction_lea: Instruction {
    Item *dst;
    Item *addr;
    Item *multr;
    Item *const_multr;
  };

  struct Instruction_goto: Instruction {
    Item * gotoLabel;
  };

  struct Instruction_dec: Instruction {
    Item * op;
  };

  struct Instruction_inc: Instruction {
     Item * op;
  };

  struct Instruction_cjump: Instruction {
    Item *dst;        /* expected ItemLabel*/
    Item *condition;  /* expected ItemCmp*/
  };

  /*
   * Function.
   */
  struct Function{
    std::string name;
    int64_t arguments;
    int64_t locals;
    std::vector<Instruction *> instructions;
  };

  /*
   * Program.
   */
  struct Program{
    std::string entryPointLabel;
    std::vector<Function *> functions;
  };

}
