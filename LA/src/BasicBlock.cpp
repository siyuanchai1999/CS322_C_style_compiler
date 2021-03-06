#include "BasicBlock.h"

namespace LA
{
    void enforceBasicBlock(Program & p){
        
        
        for (Function * F : p.functions) {
            bool startBB = true;
            std::vector<Instruction *>  instsProcessed;

            for (int32_t i = 0; i < F->insts.size(); i++) {
                Instruction * curInst = F->insts[i];
                if (startBB) {
                    if (curInst->type != InstType::inst_label) {
                        /**
                         * if (Inst is not Label) {
                         * L = new Label() ; newInsts.append(L) ;
                         * }
                         * */
                        ItemLabel * BBstartLb = LA::GENLV->get_new_label();
                        Instruction_label * BBstartLbInst = new Instruction_label(BBstartLb);
                        instsProcessed.push_back(BBstartLbInst);
                    }
                    startBB = false;
                    
                } else if (curInst->type == InstType::inst_label) {         /* Lb1 inst+.. Lb2 */
                    /**
                     *  g = new Goto(Inst); newInsts.append(g) ;
                     * */
                    Instruction_label * lbInst = (Instruction_label *) curInst;
                    
                    assert(lbInst->item_label->itemtype == ItemType::item_labels);
                    Instruction_branch * jmpToNextBB = new Instruction_branch(lbInst->item_label);
                    instsProcessed.push_back(jmpToNextBB);

                }
                
                instsProcessed.push_back(curInst);
                
                if (isTerminator(curInst)) {
                    startBB = true;
                }
                
            }
            
            if (!startBB) {
                Instruction * ret;
                if (F->retType->vtype == VarType::void_type) {
                    ret = new Instruction_ret();
                } else {
                    ret = new Instruction_ret_var(new ItemConstant(0));
                }

                instsProcessed.push_back( ret );
            }

            F->insts = instsProcessed;

        }

    }

}