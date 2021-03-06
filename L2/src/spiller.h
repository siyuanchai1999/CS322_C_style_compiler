#pragma once

#include "L2.h"
#include <map>
#include <unordered_map>
namespace L2
{
    void run_Spill(Program &p);

    class SpillerVisitor : public InstVisitor
    {
    public:
        void visit(Instruction_ret *) override;
        void visit(Instruction_label *) override;
        void visit(Instruction_call_runtime *) override;
        void visit(Instruction_call_user *) override;
        void visit(Instruction_aop *) override;
        void visit(Instruction_assignment *) override;
        void visit(Instruction_sop *) override;
        void visit(Instruction_lea *) override;
        void visit(Instruction_goto *) override;
        void visit(Instruction_dec *) override;
        void visit(Instruction_inc *) override;
        void visit(Instruction_cjump *) override;

        SpillerVisitor(
            ItemVariable * varToSpill,
            ItemVariable * prefix,
            Function * F
        );

        std::vector<Instruction *> new_insts;
        std::vector<ItemVariable *> var_replacements;
    private:
        Function * F;
        
        /**
         *  map contains all instructions that have variables to be spilled
         *      inst -> vector replacement instruction
         * */
        // std::map<Instruction *, std::vector<Instruction *>> inst2replacement;

        ItemVariable *varToSpill;
        ItemVariable *prefix;
        int32_t suffix_num;

        /**
         *  produce pointer to variable from prefix and suffix number
         *      e.g. prefix = %S, suffix_num = 1
         *          => %S1
         *      increase suffix_num by one in default
         * */
        ItemVariable * build_new_var_prefix_suffix();

        /**
         *  return pointer to a new memory access item based on F->locals
         *      mem rsp F->locals - 1
         * */
        ItemMemoryAccess * build_stackAccess_locals();

        void spill_Inst(
            Instruction * inst,
            bool HasRead,
            bool HasWritten,
            std::vector<Item **> & placeToWrite 
        );
    
    };


    class Spiller {
        public:
            void spill_variables();
            void output_spilled_function();

            Spiller(
                Function * F,
                ItemVariable * varToSpill,
                ItemVariable * prefix
            );

            std::vector<ItemVariable *> get_var_replacement();
        private:
            Function * F;

            ItemVariable * varToSpill;
            ItemVariable * prefix;

            SpillerVisitor * spill_visitor;
            
    };

}        