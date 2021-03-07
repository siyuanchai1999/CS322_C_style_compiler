#include "trans_while.h"

namespace LB {

    void get_cond_labels_scope(
        Instruction_scope * scope,
        std::map<Instruction_while *, ItemLabel *> & while2CondLabel
    ) {
        for (Instruction * inst : scope->insts) {
            if (inst->type == inst_scope) {
                Instruction_scope * nextScope = (Instruction_scope *) inst;
                get_cond_labels_scope(nextScope, while2CondLabel);
                
            }
            else if (inst->type == inst_while) {
                Instruction_while * whileInst = (Instruction_while *) inst;
                ItemLabel * condlb = LB::GENLV->get_new_label();
                while2CondLabel[whileInst] = condlb;
            }
            else 
            {
                /* nothing to do  */
            }
        }
    }

    std::map<Instruction_while *, ItemLabel *> get_cond_labels(Function * F) {
        std::map<Instruction_while *, ItemLabel *> while2CondLabel;

        get_cond_labels_scope(F->scope, while2CondLabel);

        return while2CondLabel;
    }


    void get_begin_end_labels_scope(
        Instruction_scope * scope,
        std::map<ItemLabel *, Instruction_while *> & beginToWhile,
        std::map<ItemLabel *, Instruction_while *> & endToWhile
    ) {
        for (Instruction * inst : scope->insts) {
            if (inst->type == inst_scope) {
                Instruction_scope * nextScope = (Instruction_scope *) inst;
                get_begin_end_labels_scope(
                    nextScope,
                    beginToWhile,
                    endToWhile
                );
                
            }
            else if (inst->type == inst_while) {
                Instruction_while * whileInst = (Instruction_while *) inst;
                
                assert (whileInst->dst1->itemtype == ItemType::item_labels);
                
                beginToWhile[(ItemLabel *) whileInst->dst1] = whileInst;

                assert (whileInst->dst2->itemtype == ItemType::item_labels);
                endToWhile[(ItemLabel *) whileInst->dst2] = whileInst;

            }
            else 
            {
                /* nothing to do  */
            }
        }
    }




    void get_inst2loop_scope(
        Instruction_scope * scope,
        std::map<Instruction *, Instruction_while *>  & inst2loop,
        std::stack<Instruction_while *> & loopStack,
        std::map<ItemLabel *, Instruction_while *> & beginToWhile,
        std::map<ItemLabel *, Instruction_while *> & endToWhile
    ) {

        for (Instruction * inst : scope->insts) {
            if (loopStack.size() > 0) {
                Instruction_while * whileInst = loopStack.top();
                inst2loop[inst] = whileInst;
            }

            if (inst->type == inst_label) {
                Instruction_label * instlb = (Instruction_label *) inst;

                assert(instlb->item_label->itemtype == ItemType::item_labels);
                ItemLabel * lb = (ItemLabel *) instlb->item_label;


                if (IN_MAP(beginToWhile, lb)) {
                    /**
                     *  inst is the beginning of a while loop
                     * */
                    loopStack.push(beginToWhile[lb]);
                } 
                else if (IN_MAP(endToWhile, lb)) {
                    /**
                     *  inst is the end of a while loop
                     * */
                    loopStack.pop();
                } 
                else {
                    /**
                     *  normal label; nothing to do 
                     * */
                }

                
            }
            
            
            if (inst->type == inst_scope) {
                Instruction_scope * nextScope = (Instruction_scope *) inst;
                /**
                 *  if this is a scope, next instructions are in the nest scope
                 * */
                get_inst2loop_scope(
                    nextScope,
                    inst2loop,
                    loopStack,
                    beginToWhile,
                    endToWhile
                );
            }
        }
    }


    std::map<Instruction *, Instruction_while *> get_inst2loop (Function * F) {
        
        std::map<Instruction *, Instruction_while *>  inst2loop;
        std::stack<Instruction_while *> loopStack;
        std::map<ItemLabel *, Instruction_while *> beginToWhile;
        std::map<ItemLabel *, Instruction_while *> endToWhile;
        
        get_begin_end_labels_scope(
            F->scope,           /* main scope */    
            beginToWhile,       /* begin labels */
            endToWhile          /* end labels */
        );

        get_inst2loop_scope(
            F->scope,           /* main scope */  
            inst2loop,          /* output of the function map from map to its nested loop */
            loopStack,          /* helper data structure */
            beginToWhile,       /* begin labels */
            endToWhile          /* end labels */
        );

        return inst2loop;
    }
}



