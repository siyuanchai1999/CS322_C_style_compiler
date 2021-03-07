
#include "trans_scope_var.h"

namespace LB {
    void translate_var_scope(Instruction_scope * scope) {
        std::unordered_map<std::string, ItemVariable *> newStr2Var;

        /**
         *  transform new var in each scope to a unique name
         * */
        for (auto & kv : scope->varName2ptr) {
            /**
             *  
             * */        
            ItemVariable * var = kv.second;
            std::string newVarStr =  LB::GENLV->get_new_var_str();

            var->name = newVarStr;
            newStr2Var[newVarStr] = var;
        }


        /**
         *  recursively transform the sub scope
         * */
        for (Instruction * inst : scope->insts) {
            if (inst->type == inst_scope) {
                Instruction_scope * nextScope = (Instruction_scope *) inst;
                translate_var_scope(nextScope);
                
            }
        }

        scope->varName2ptr = newStr2Var;
    }

    
    void translate_LB_vars(Program & p) {
        for (Function * F : p.functions) {
            
            for (auto & kv : F->argName2ptr) {
                /* nothing to do now */
                /* we don't transform function args */
            }

            translate_var_scope(F->scope);

        }   
    }

    
    
}