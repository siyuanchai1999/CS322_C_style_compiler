
#include "transformer.h"

namespace L3{
    static std::string init_longest = "";

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

    std::string find_longest (Program & p) {
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

    // void transform_function_labels(Function * F, std::string & LLG) {
    //     for (Item * lb : F->labels) {
    //         ItemLabel * l = (ItemLabel *) lb;

    //         std::string substitute = LLG + l->labelName.substr(1, l->labelName.length() - 1);
        
    //         F->labelName2ptr.erase(l->labelName);
    //         F->labelName2ptr[substitute] = l;
    //         l->labelName = substitute;

            
            
    //     }
    // }

    /**
     * 1. Find the longest label for the whole L3 program: LL
        2. Append “_global_” to it: LLG
        3. For every L3 label “:LABELNAME”, generate an L2 label by appending
        “LABELNAME” to LLG
     * */
    void transform_label (Program & p) {
        // if (init_longest.size() == 0) init_longest = find_longest(p);

        // std::string toAppend = "_global_";
        // std::string LLG = init_longest + toAppend;

        std::set<std::string> function_names;
        for (Function * F : p.functions) 
        {
            function_names.insert(F->name->labelName);
        }

        for (Function * F : p.functions) 
        {
            for (Item * lb : F->Instlabels) {
                ItemLabel * l = (ItemLabel *) lb;

                // if (!IN_SET(function_names, l->labelName)){
                    std::string substitute = 
                            F->name->labelName 
                        +   "_"
                        +   l->labelName.substr(1, l->labelName.length() - 1);
            
                    F->labelName2ptr.erase(l->labelName);
                    F->labelName2ptr[substitute] = l;
                    l->labelName = substitute;
                // }
            }
        }

    }

    std::string new_fRetLabel_prefix(Program & p) {
        if (init_longest.size() == 0) init_longest = find_longest(p);

        std::string longest = find_longest(p);

        std::string toAppend = "_Fret_";

        std::string LFret = longest + toAppend;

        return LFret;
    }    
    
}