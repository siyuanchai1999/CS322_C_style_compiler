#pragma once

#include <vector>
#include <string>
#include <set>
#include <map>
#include <iostream>
#include <unordered_map>
// #include "analysis.h"
#include <cassert>


#include "utils.h"
#include "config.h"



namespace LB {
    struct Program;
    struct Function;

    struct Item;

    struct ItemTypeSig;

    struct ItemConstant;
    struct ItemLabel;
    struct ItemVariable;
    struct ItemFName;

    struct ItemArrAccess;
    struct ItemOp;
    struct ItemCall;

    struct ItemNewArray;
    struct ItemNewTuple;
    struct ItemLength;


    class ItemVisitor {
        public:
            virtual void visit(ItemTypeSig *) = 0;

            virtual void visit(ItemConstant *) = 0 ;
            virtual void visit(ItemLabel *) = 0 ;
            virtual void visit(ItemVariable *) = 0 ;
            virtual void visit(ItemFName *) = 0 ;

            virtual void visit(ItemArrAccess *) = 0;
            virtual void visit(ItemOp *) = 0;
            virtual void visit(ItemCall *) = 0;

            virtual void visit(ItemNewArray *) = 0;
            virtual void visit(ItemNewTuple *) = 0;
            virtual void visit(ItemLength *) = 0;       
    };

    enum ItemType{
        item_type_sig,

        item_constant,
        item_labels,
        item_variable,
        item_fname,

        item_ArrAccess,
        item_op,
        item_newArr,
        item_newTuple,
        item_call,
        item_length
    };

    enum OpType
    {
        plus,
        minus,
        times,
        bit_and,
        shift_left,
        shift_right,
        less,
        leq,
        eq,
        great,
        geq,
        op_others
    };

    enum VarType {
        tuple,
        code, 
        int64,
        tensor,
        void_type
    };


    struct Item {
        ItemType itemtype;

        virtual std::string to_string() = 0;
        virtual void accept(ItemVisitor &) = 0;
        virtual Item * copy() = 0;
    };

    struct ItemTypeSig : Item {
        VarType vtype;
        int32_t ndim;        /* only valid when vtype == tensor*/
        
        ItemTypeSig(VarType vtype);
        ItemTypeSig(VarType vtype, int32_t ndim);

        void accept(ItemVisitor &visitor) override;
        ItemTypeSig * copy() override;
        std::string to_string() override;
    };


    struct ItemConstant : Item {
        int64_t constVal;
        bool encoded;

        ItemConstant(int64_t constVal);
        ItemConstant(int64_t constVal, bool isEncoded);

        std::string to_string() override;
        void accept(ItemVisitor & visitor) override;
        ItemConstant * copy() override;

        /**
         *  change constVal to be the encoded version
         * */
        void encodeItself();
        /**
         *  change constVal to be the decoded version
         * */
        void decodeItself();

        
    }; 

    struct ItemLabel : Item {
        std::string labelName;

        ItemLabel(std::string str);

        std::string to_string() override;
        void accept(ItemVisitor & visitor) override;
        ItemLabel * copy() override;
    }; 

    struct ItemVariable: Item {
        std::string name;
        Item * typeSig;

        ItemVariable(std::string str, Item * typeSig);

        std::string to_string() override;
        void accept(ItemVisitor & visitor) override;
        ItemVariable * copy() override;
    };

    
    struct ItemFName: Item {
        std::string name;

        /**
         *  filled out at the end of parser function
         * */
        Function * fptr;

        ItemFName(std::string str);

        std::string to_string() override;
        void accept(ItemVisitor & visitor) override;
        ItemFName * copy() override;
    }; 

    struct ItemArrAccess : Item {
        Item * addr;
        std::vector<Item *> offsets;
        int64_t lineNumber;     /* for tensor error call */

        ItemArrAccess(
            Item * addr,
            std::vector<Item *> & offsets,
            int64_t lineNumber
        );

        std::string to_string() override;
        void accept(ItemVisitor & visitor) override;
        ItemArrAccess * copy() override;

        VarType getAccessDataType();
    };

    struct ItemOp : Item {
        Item * op1;
        Item * op2;
        OpType opType;

        ItemOp(Item *op1, Item *op2, OpType opType);

        bool isComparison();

        std::string to_string() override;
        void accept(ItemVisitor & visitor) override;
        ItemOp * copy() override;
    };


    struct ItemCall : Item {
        bool isRuntime;
        bool calleeIsFName;
        /**
         *  callee expected to be a FName or var
         *      when isRuntime == true
         *          callee in {print_FName, input_FName, allocate_FName, tensor_FName}
         *      when calleeIsFunction == true
         *          callee is a FName
         * */

        Item *callee;
        std::vector<Item *> args;
        
        // ItemCall();
        ItemCall(
            bool isRuntime,
            bool calleeIsFName,
            Item *callee,
            std::vector<Item *> & args
        );

        std::string to_string() override;
        void accept(ItemVisitor &visitor) override;
        ItemCall * copy() override;

    };

    struct ItemNewArray : Item {
        std::vector<Item * > dims;
        
        ItemNewArray(std::vector<Item *> & dims);

        std::string to_string() override;
        void accept(ItemVisitor & visitor) override;
        ItemNewArray * copy() override;

    };

    struct ItemNewTuple : Item {
        Item * len;
        
        ItemNewTuple(Item * len);

        std::string to_string() override;
        void accept(ItemVisitor & visitor) override;
        ItemNewTuple * copy() override;
    };

    struct ItemLength : Item {
        Item * addr;
        Item * dim;
        
        ItemLength(Item * addr, Item * dim);

        std::string to_string() override;
        void accept(ItemVisitor & visitor) override;
        ItemLength * copy() override;
    };
    
    
    /**
     *  every type signature can predefined except tensor
     *      tensor created at compile time
     * */
    extern ItemTypeSig tupleSig;
    extern ItemTypeSig codeSig;
    extern ItemTypeSig int64Sig;
    extern ItemTypeSig voidSig;

    const std::string print_str = "print";
    const std::string input_str = "input";
    const std::string tensor_str = "tensor-error";
    extern ItemFName print_FName;
    extern ItemFName input_FName;
    extern ItemFName tensor_FName;

    extern std::set<ItemType> basicTypes;
    extern std::set<ItemType> varAndConst;
    extern std::set<ItemType> varAndLabel;
    
    bool isRuntimeFName(Item *);
    bool isBasicItem (Item *);


    enum InstType{
        inst_label,

        inst_ret,
        inst_ret_var,
        inst_goto,

        inst_if,
        inst_while,
        inst_continue,
        inst_break,

        inst_declare,
        inst_assign,
        inst_call,

        inst_scope
    };

    struct Instruction;
    

    struct Instruction_label;
    
    struct Instruction_ret;
    struct Instruction_ret_var;
    struct Instruction_goto;

    struct Instruction_if;
    struct Instruction_while;
    struct Instruction_continue;
    struct Instruction_break;


    struct Instruction_declare;
    struct Instruction_call;
    struct Instruction_assignment;

    struct Instruction_scope;


    class InstVisitor {
        public:
            virtual void visit(Instruction_label *)         = 0 ;
            
            virtual void visit(Instruction_ret *)           = 0 ;
            virtual void visit(Instruction_ret_var *)       = 0 ;
            virtual void visit(Instruction_goto *)          = 0 ;

            virtual void visit(Instruction_if *)            = 0 ;
            virtual void visit(Instruction_while *)         = 0 ;
            virtual void visit(Instruction_continue *)      = 0 ;
            virtual void visit(Instruction_break *)         = 0 ;

            virtual void visit(Instruction_declare *)       = 0 ;
            virtual void visit(Instruction_call *)          = 0 ;
            virtual void visit(Instruction_assignment *)    = 0 ;

            virtual void visit(Instruction_scope *)         = 0 ;
    };

    
    /*
     * Instruction interface.
     */
    struct Instruction{
        InstType type;
        Instruction_scope * parent;


        virtual std::string to_string() = 0;
        virtual void accept(InstVisitor &) = 0;
        // virtual Instruction * copy() = 0;
    };
    
    struct Instruction_label : Instruction{
        // std::string labelName;
        Item * item_label;

        Instruction_label(Item * label, Instruction_scope * parent);

        std::string to_string() override;
        void accept(InstVisitor & visitor) override;
        // Instruction_label * copy() override;
    };



    struct Instruction_ret : Instruction {
        
        Instruction_ret (Instruction_scope * parent);

        std::string to_string() override;
        void accept(InstVisitor & visitor) override;
        // Instruction_ret * copy() override;

        // std::set<BasicBlock *> getSuccessor(
        //     std::map<ItemLabel *, BasicBlock *> & label2BB
        // );
    };

    struct Instruction_ret_var : Instruction {
        Item *valueToReturn; /* should be variable, number, or label */

        Instruction_ret_var(Item * value_to_return, Instruction_scope * parent);
        
        std::string to_string() override;
        void accept(InstVisitor & visitor) override;
        // Instruction_ret_var * copy() override;

        // std::set<BasicBlock *> getSuccessor(
        //     std::map<ItemLabel *, BasicBlock *> & label2BB
        // );
    };

    struct Instruction_goto : Instruction {
        Item * dst;                    

        Instruction_goto(Item * dst, Instruction_scope * parent);
        
        void accept(InstVisitor & visitor) override;
        std::string to_string() override;
        // Instruction_goto *copy() override;

        // std::set<BasicBlock *> getSuccessor(
        //     std::map<ItemLabel *, BasicBlock *> & label2BB
        // );
    };

    struct Instruction_if : Instruction {
        Item * dst1;             
        Item * dst2; 
        Item * condition;       

        Instruction_if(
            Item * dst1,
            Item * dst2,
            Item * condition,
            Instruction_scope * parent    
        );
        
        void accept(InstVisitor & visitor) override;
        std::string to_string() override;
        // Instruction_if *copy() override;

        // std::set<BasicBlock *> getSuccessor(
        //     std::map<ItemLabel *, BasicBlock *> & label2BB
        // );
    };

    struct Instruction_while : Instruction {
        Item * dst1;             
        Item * dst2; 
        Item * condition;       

        Instruction_while(
            Item * dst1,
            Item * dst2,
            Item * condition,
            Instruction_scope * parent 
        );
        
        void accept(InstVisitor & visitor) override;
        std::string to_string() override;
        // Instruction_while *copy() override;

        // std::set<BasicBlock *> getSuccessor(
        //     std::map<ItemLabel *, BasicBlock *> & label2BB
        // );
    };

    struct Instruction_continue : Instruction {
        
        Instruction_continue ( Instruction_scope * parent );

        std::string to_string() override;
        void accept(InstVisitor & visitor) override;
        // Instruction_continue * copy() override;

        // std::set<BasicBlock *> getSuccessor(
        //     std::map<ItemLabel *, BasicBlock *> & label2BB
        // );
    };

    struct Instruction_break : Instruction {
        
        Instruction_break ( Instruction_scope * parent );

        std::string to_string() override;
        void accept(InstVisitor & visitor) override;
        // Instruction_break * copy() override;

        // std::set<BasicBlock *> getSuccessor(
        //     std::map<ItemLabel *, BasicBlock *> & label2BB
        // );
    };


    struct Instruction_declare : Instruction {
        Item * typeSig;
        std::vector<Item *> vars;

        Instruction_declare(
            Item * typeSig,
            std::vector<Item *> & vars,
            Instruction_scope * parent 
        );

        std::string to_string() override;
        void accept(InstVisitor & visitor) override;
        // Instruction_declare * copy() override;

        VarType getTypeSig();
    };
    
    /**
     *  call without return
     * */
    struct Instruction_call : Instruction {
        Item * call_wrap;

        Instruction_call(
            Item * call_wrap,
            Instruction_scope * parent
        );

        std::string to_string() override;
        void accept(InstVisitor & visitor) override;
        // Instruction_call * copy() override;
        
    };


    struct Instruction_assignment : Instruction {
        Item *src, *dst;

        Instruction_assignment(
            Item *src,
            Item *dst,
            Instruction_scope * parent
        );

        void accept(InstVisitor & visitor) override;
        std::string to_string() override;
        // Instruction_assignment * copy() override;
    };


    struct Instruction_scope : Instruction {        
        std::vector<Instruction *> insts;
        std::map<ItemVariable *, ItemVariable *> ScopedToUnScopedVar; 

        Instruction_scope(Instruction_scope * parent);
        
        void appendInst(Instruction * inst);

        void accept(InstVisitor & visitor) override;
        std::string to_string() override;
        // Instruction_scope * copy() override;
    };

    
    /*
     * Function.
     */
    struct Function{
        ItemTypeSig * retType; 
        ItemFName * name;

        Instruction_scope * scope;

        std::vector<Item *> arg_list;

        /**
         *  only used to remove duplicate var/label
         *      aka, every var/label with same name is the same object
         * */
        std::unordered_map<std::string, Item *> varName2ptr;
        std::unordered_map<std::string, Item *> labelName2ptr;
        
        /**
         *  Instlabels doesn't track function labels
         * */
        std::set<Item *> Instlabels;
        std::set<Item *> vars;
        // std::set<ItemConstant *> constToEncode;
        
        // Function();
        void print();
        
        
    };

    /*
     * Program.
     */
    struct Program{
        Function * mainF;
        std::vector<Function *> functions;
        
        
        /**
         * populate predecessors and sucessors for every basic blocks in the program
         *  wrapper function, calls function.populatePredsSuccs
         * */
        // void populatePredsSuccs();
    };

}
