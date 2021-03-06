#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <unordered_map>
// #include "analysis.h"

namespace L2 {
    struct Item;
    struct ItemRegister;
    struct ItemConstant;
    struct ItemLabel;
    struct ItemMemoryAccess;
    struct ItemAop;
    struct ItemCmp;
    struct ItemVariable;
    struct ItemStackArg;

    class ItemVisitor {
        public:
            virtual void visit(ItemRegister *) = 0 ;
            virtual void visit(ItemConstant *) = 0 ;
            virtual void visit(ItemLabel *) = 0 ;
            virtual void visit(ItemMemoryAccess *) = 0 ;
            virtual void visit(ItemAop *) = 0 ;
            virtual void visit(ItemCmp *) = 0 ;
            virtual void visit(ItemVariable *) = 0 ;
            virtual void visit(ItemStackArg *) = 0 ;
    };

    enum ItemType{
        item_registers,
        item_constant,
        item_memory,
        item_labels,
        item_aop,
        item_cmp,
        item_variable,
        item_stack_arg
    };

    enum Register_type {rdi, rax, rsi, rdx, rcx, r8, r9, rbx, rbp, r10, r11, r12, r13, r14, r15, rsp};
    enum Register_8b_type {r10b, r11b, r12b, r13b, r14b, r15b, r8b, r9b, al, bpl, bl, cl, dil, dl, sil};
    enum AopType {plus_eq, minus_eq, times_eq, bitand_eq};
    enum CmpType {less, leq, eq, great, geq};
    enum ShiftType {left, right};

    struct Item{
        ItemType itemtype;

        virtual std::string to_string() = 0;
        virtual void accept(ItemVisitor &) = 0;
        virtual Item * copy() = 0;
    };

    struct ItemRegister : Item {
        Register_type rType;

        ItemRegister(Register_type rType); 
        
        std::string to_string() override;
        void accept(ItemVisitor & visitor) override;
        ItemRegister * copy() override;
    };

    struct ItemConstant : Item {
        int64_t constVal;

        ItemConstant();
        ItemConstant(int64_t constVal);

        std::string to_string() override;
        void accept(ItemVisitor & visitor) override;
        ItemConstant * copy() override;
    }; 

    struct ItemLabel : Item {
        std::string labelName;

        ItemLabel(std::string str);

        std::string to_string() override;
        void accept(ItemVisitor & visitor) override;
        ItemLabel * copy() override;
    };

    struct ItemMemoryAccess : Item {
        Item * reg;
        Item * offset;

        ItemMemoryAccess();
        ItemMemoryAccess(Item * reg, Item * offset);

        std::string to_string() override;
        void accept(ItemVisitor & visitor) override;
        ItemMemoryAccess * copy() override;
    };

    struct ItemAop : Item {
        AopType aopType;

        std::string to_string() override;
        void accept(ItemVisitor & visitor) override;
        ItemAop * copy() override;
    };

    struct ItemCmp : Item {
        Item * op1;
        Item * op2;
        CmpType cmptype;

        ItemCmp();
        ItemCmp(Item *op1, Item *op2, CmpType cmpType);

        std::string to_string() override;
        void accept(ItemVisitor & visitor) override;
        ItemCmp * copy() override;
    };

    struct ItemVariable: Item {
        std::string name;
        
        ItemVariable(std::string str);

        std::string to_string() override;
        void accept(ItemVisitor & visitor) override;
        ItemVariable * copy() override;
    };

    struct ItemStackArg: Item {
        Item * offset;      /*Expected to be a multiple of 8*/

        ItemStackArg();
        ItemStackArg(Item *offset);

        std::string to_string() override;
        void accept(ItemVisitor & visitor) override;
        ItemStackArg * copy() override;
    };

    extern ItemRegister reg_rdi ;
    extern ItemRegister reg_rax ;
    extern ItemRegister reg_rsi ;
    extern ItemRegister reg_rdx ;
    extern ItemRegister reg_rcx ;
    extern ItemRegister reg_r8  ;
    extern ItemRegister reg_r9  ;
    extern ItemRegister reg_rbx ;
    extern ItemRegister reg_rbp ;
    extern ItemRegister reg_r10 ;
    extern ItemRegister reg_r11 ;
    extern ItemRegister reg_r12 ;
    extern ItemRegister reg_r13 ;
    extern ItemRegister reg_r14 ;
    extern ItemRegister reg_r15 ;
    extern ItemRegister reg_rsp ;
    
    const int CALLER_NUM = 9;
    const int CALLEE_NUM = 6;
    const int ARG_NUM = 6;
    const int GP_NUM = 15;

    extern ItemRegister *caller_saved[CALLER_NUM];
    extern ItemRegister *callee_saved[CALLEE_NUM];
    extern ItemRegister *arg_regs[ARG_NUM];
    extern ItemRegister *GP_regs[GP_NUM];

    const std::string print_str = "print";
    const std::string input_str = "input";
    const std::string allocate_str = "allocate";
    const std::string tensor_str = "tensor-error";
    extern ItemLabel print_label;
    extern ItemLabel input_label;
    extern ItemLabel allocate_label;
    extern ItemLabel tensor_label;



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

    struct Instruction;
    struct Instruction_ret;
    struct Instruction_label;
    struct Instruction_call;
    struct Instruction_call_runtime;
    struct Instruction_call_user;
    struct Instruction_aop;
    struct Instruction_assignment;
    struct Instruction_sop;
    struct Instruction_lea;
    struct Instruction_goto;
    struct Instruction_dec;
    struct Instruction_inc;
    struct Instruction_cjump;

    class InstVisitor {
        public:
            virtual void visit(Instruction_ret *) = 0 ;
            virtual void visit(Instruction_label *) = 0 ;
            // virtual void visit(Instruction_call *) = 0 ;
            virtual void visit(Instruction_call_runtime *) = 0 ;
            virtual void visit(Instruction_call_user *) = 0 ;
            virtual void visit(Instruction_aop *) = 0 ;
            virtual void visit(Instruction_assignment *) = 0 ;
            virtual void visit(Instruction_sop *) = 0 ;
            virtual void visit(Instruction_lea *) = 0 ;
            virtual void visit(Instruction_goto *) = 0 ;
            virtual void visit(Instruction_dec *) = 0 ;
            virtual void visit(Instruction_inc *) = 0 ;
            virtual void visit(Instruction_cjump *) = 0 ;
            
    };

    
    /*
     * Instruction interface.
     */
    struct Instruction{
        InstType type;

        virtual std::string to_string() = 0;
        virtual void accept(InstVisitor &) = 0;
        virtual Instruction * copy() = 0;
    };
    
    


    /*
     * Instructions.
     */
    struct Instruction_ret : Instruction{

        std::string to_string() override;
        void accept(InstVisitor & visitor) override;
        Instruction_ret * copy();
    };

    struct Instruction_label : Instruction{
        // std::string labelName;
        Item * item_label;
        std::string to_string() override;
        void accept(InstVisitor & visitor) override;
        Instruction_label * copy();
    };

    struct Instruction_call : Instruction{
        bool isRuntimeCall;
        
        // void accept_live_GENKILL(live_GENKILL_visitor & visitor) override {
        //     visitor.visit(this);
        // }
        // void accept(InstVisitor & visitor) override;
    };

    struct Instruction_call_runtime: Instruction_call {
        // std::string callee;
        // int arg_cnt;
        Item * runtime_callee;
        Item * num_args;

        std::string to_string() override;
        void accept(InstVisitor & visitor) override;
        Instruction_call_runtime * copy();
    };

    struct Instruction_call_user: Instruction_call {
        Item * callee;
        Item * num_args;

        std::string to_string() override;
        void accept(InstVisitor & visitor) override;
        Instruction_call_user * copy();
    };

    struct Instruction_aop: Instruction {
        Item *op1, *op2;
        AopType aopType; 
        
        void accept(InstVisitor & visitor) override;
        std::string to_string() override;
        Instruction_aop * copy();
    };

    struct Instruction_assignment : Instruction{
        Item *src, *dst;

        void accept(InstVisitor & visitor) override;
        std::string to_string() override;
        Instruction_assignment * copy();
    };

    struct Instruction_sop: Instruction {
        Item *target;
        Item *offset;
        ShiftType direction;

        void accept(InstVisitor & visitor) override;
        std::string to_string() override;
        Instruction_sop * copy();
    };

    struct Instruction_lea: Instruction {
        Item *dst;
        Item *addr;
        Item *multr;
        Item *const_multr;

        void accept(InstVisitor & visitor) override;
        std::string to_string() override;
        Instruction_lea * copy();
    };

    struct Instruction_goto: Instruction {
        Item * gotoLabel;

        void accept(InstVisitor & visitor) override;
        std::string to_string() override;
        Instruction_goto *copy();
    };

    struct Instruction_dec: Instruction {
        Item * op;

        void accept(InstVisitor & visitor) override;
        std::string to_string() override;
        Instruction_dec *copy();
    };

    struct Instruction_inc: Instruction {
        Item * op;

        void accept(InstVisitor & visitor) override;
        std::string to_string() override;
        Instruction_inc *copy();
    };

    struct Instruction_cjump: Instruction {
        Item *dst;        /* expected ItemLabel*/
        Item *condition;  /* expected ItemCmp*/

        void accept(InstVisitor & visitor) override;
        std::string to_string() override;
        Instruction_cjump *copy();
    };


    

    /*
     * Function.
     */
    struct Function{
        std::string name;
        int64_t arguments;
        int64_t locals;
        std::vector<Instruction *> instructions;
        std::unordered_map<std::string, Item *> varName2ptr;
        std::unordered_map<std::string, Item *> labelName2ptr;
    
        Function();
        void print();
        Function * copy();
        ~Function();
    };

    /*
     * Program.
     */
    struct Program{
        std::string entryPointLabel;
        std::vector<Function *> functions;

        /**
         *  only used for spill test
         * */
        ItemVariable * varToSpill;
        ItemVariable * prefix;
    };

}
