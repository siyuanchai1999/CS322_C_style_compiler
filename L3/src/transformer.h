#pragma once

#include "L3.h"
#include "utils.h"
namespace L3{
    /**
     * 1. Find the longest label for the whole L3 program: LL
        2. Append “_global_” to it: LLG
        3. For every L3 label “:LABELNAME”, generate an L2 label by appending
        “LABELNAME” to LLG
     * */
    void transform_label (Program & p);
    
    std::string new_var_prefix(Program & p);

    std::string new_fRetLabel_prefix(Program & p);
}