
#include "new_label_var.h"

namespace LA{
    /**
     * 1. Find the longest label for the whole L3 program: LL
        2. Append “_global_” to it: LLG
        3. For every L3 label “:LABELNAME”, generate an L2 label by appending
        “LABELNAME” to LLG
     * */

    

    std::string find_longest_var (Program & p) {
        std::string longest = "";
        int32_t len = 0;

        for (Function * F: p.functions) {
            for (auto & kv : F->varName2ptr) {
                int32_t l = kv.first.length();
                if (l > len) {
                    longest = kv.first;
                    len = l;
                }
            }
        }

        return longest;
    }

    std::string new_var_prefix(Program & p) {
        std::string LV =  find_longest_var(p);
        if (LV.size() == 0) {
            LV = "%";
        }
        
        return LV + "_new_";
    }

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
        std::string LV = find_longest_var(p);
        this->varPrefix = LV + "_newV_";        /* LLN */
        this->varIdx = 0;

        std::string LL = find_longest_label(p);
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
            sig = &LA::codeSig;
            break;
        
        case VarType::tuple :
            sig = & LA::tupleSig;
            break;

        case VarType::int64 :
            sig =  & LA::int64Sig;
            break;

        case VarType::void_type :
            assert(0); /* vars can't be void, plz don't */
            sig = &LA::voidSig;
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


