#pragma once

#include "LA.h"
#include "utils.h"
#include "new_label_var.h"
#include "encode.h"

namespace LA
{
    void insertMemCheck(Program & p);
    
    // /**
    //  * 
    //  * */
    void checkAllocation(
        std::vector<Instruction *> & instsProcessed,
        ItemArrAccess * arrAccess
    );

    void checkBoundary(
        std::vector<Instruction *> & instsProcessed,
        ItemArrAccess * arrAccess
    );
}