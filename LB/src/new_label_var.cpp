
#include "new_label_var.h"

namespace LB {
    /**
     * 1. Find the longest label for the whole L3 program: LL
        2. Append “_global_” to it: LLG
        3. For every L3 label “:LABELNAME”, generate an L2 label by appending
        “LABELNAME” to LLG
     * */

    std::string find_longest_var_scope(Instruction_scope * scope ) {
        std::string longest = "";
        int32_t len = 0;

        for (auto & kv : scope->varName2ptr) {
            int32_t l = kv.first.length();
            if (l > len) {
                longest = kv.first;
                len = l;
            }
        }

        for (Instruction * inst : scope->insts) {
            if (inst->type == inst_scope) {
                Instruction_scope * nextScope = (Instruction_scope *) inst;
                std::string LV_scope = find_longest_var_scope(nextScope);

                int32_t l = LV_scope.length();
                if (l > len) {
                    longest = LV_scope;
                    len = l;
                } 
            }
        }

        return longest;
    }



    std::string find_longest_var (Program & p) {
        std::string longest = "";
        int32_t len = 0;

        for (Function * F: p.functions) {
            

            for (auto & kv : F->argName2ptr) {
                int32_t l = kv.first.length();
                if (l > len) {
                    longest = kv.first;
                    len = l;
                }
            }

            std::string LV_main_scope = find_longest_var_scope(F->scope);
            int32_t l = LV_main_scope.length();
            if (l > len) {
                longest = LV_main_scope;
                len = l;
            } 

        }

        return longest;
    }

    // std::string new_var_prefix(Program & p) {
    //     // std::string LV =  find_longest_var(p);
    //     std::string LV =  "";
    //     if (LV.size() == 0) {
    //         LV = "%";
    //     }
        
    //     return LV + "_new_";
    // }

    std::string find_longest_label (Program & p) {
        std::string longest = "";
        int32_t len = 0;

        for (Function * F: p.functions) {
            for (auto & kv : F->labelName2ptr) {
                int32_t l = kv.first.length();
                if (l > len) {
                    longest = kv.first;
                    len = l;
                }
            }
        }

        return longest;
    }

    LabelVarGen::LabelVarGen() {
        this->varPrefix = "";        
        this->varIdx = 0;

        this->labelPrefix = "";   
        this->labelIdx = 0;

        /* vars default initialization */
        /* labels default initialization */
    }

    LabelVarGen::LabelVarGen(Program &p) {
        std::string LV =  find_longest_var(p);
        this->varPrefix = LV + "_newV_";        /* LLN */
        this->varIdx = 0;

        std::string LL = find_longest_label(p);

        /**
         * if LL is empty we need to pad a ":"
         * */
        if (LL.length() == 0) {
            LL = ":empty";
        }
        this->labelPrefix = LL + "_globalL_";   /* LLG */
        this->labelIdx = 0;

        /* vars default initialization */
        /* labels default initialization */
    }

    ItemVariable * LabelVarGen::get_new_var(VarType vtype) {
        ItemTypeSig * sig = NULL;
        
        switch (vtype)
        {
        case VarType::code :
            sig = &LB::codeSig;
            break;
        
        case VarType::tuple :
            sig = & LB::tupleSig;
            break;

        case VarType::int64 :
            sig =  & LB::int64Sig;
            break;

        case VarType::void_type :
            assert(0); /* vars can't be void, plz don't */
            sig = &LB::voidSig;
            break;
        
        case VarType::tensor :
            assert(0); /* doesn't support tensor right now */
            break;

        default:
            assert(0); /* wtf */
            break;
        }
        
        assert(sig != NULL);

        ItemVariable * newV = new ItemVariable(
            this->varPrefix + std::to_string(this->varIdx),
            sig
        );

        this->varIdx++;
        this->vars.push_back(newV);

        return newV;
    }

    std::string LabelVarGen::get_new_var_str() {
        std::string ret = this->varPrefix + std::to_string(this->varIdx);
        this->varIdx++;
        return ret;
    }

    ItemLabel * LabelVarGen::get_new_label() {

        ItemLabel * newLabel = new ItemLabel(
            this->labelPrefix + std::to_string(this->labelIdx)
        );

        this->labelIdx++;
        this->labels.push_back(newLabel);

        return newLabel;
    }

    void LabelVarGen::clean() {
        for (ItemVariable * v : this->vars) {
            delete v;
        }

        for (ItemLabel * l : this->labels) {
            delete l;
        }
    }

    
    LabelVarGen * GENLV = NULL;
    void new_var_label_init(Program & p) {
        GENLV = new LabelVarGen(p);
    }
}


