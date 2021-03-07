#pragma once

#include <stack>

#include "LB.h"
#include "utils.h"
#include "new_label_var.h"

namespace LB { 
    std::map<Instruction_while *, ItemLabel *> 
        get_cond_labels(Function * F);

    std::map<Instruction *, Instruction_while *> 
        get_inst2loop (Function * F);

}