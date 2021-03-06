
#include "check_memAccess.h"

namespace LA {
    void insertMemCheck(Program & p) {
        for (Function * F : p.functions) {
            
            std::vector<Instruction *>  instsProcessed;
            
            for (Instruction * inst : F->insts) {
                if (inst->type == InstType::inst_assign) {
                    Instruction_assignment * assign = (Instruction_assignment *) inst;
                    
                    if( assign->dst->itemtype == ItemType::item_ArrAccess ) {
                        
                        checkAllocation(
                            instsProcessed,
                            (ItemArrAccess *) assign->dst
                        );

                        checkBoundary(
                            instsProcessed,
                            (ItemArrAccess *) assign->dst
                        );

                    }

                    if( assign->src->itemtype == ItemType::item_ArrAccess ) {
        
                        checkAllocation(
                            instsProcessed,
                            (ItemArrAccess *) assign->src
                        );

                        checkBoundary(
                            instsProcessed,
                            (ItemArrAccess *) assign->src
                        );
                    }

                }
                instsProcessed.push_back(inst);
            }

            F->insts = instsProcessed;
        }
    }


    void checkAllocation(
        std::vector<Instruction *> & instsProcessed,
        ItemArrAccess * arrAccess
    ) {
        /**
         * int64 %newV
         * %newV <- %v1 = 0
         * br %newV :F :C
         * :F
         * tensor-error(%LineNumber)
         * :C
         * 
         * */ 

        ItemVariable * isArrAllocated =  LA::GENLV->get_new_var(VarType::int64);
        
        /**
         * int64 %newV
         * */   
        Instruction_declare * dec = new Instruction_declare(
            &LA::int64Sig,
            isArrAllocated
        );
        instsProcessed.push_back(dec);

        /**
         * %newV <- %v1 = 0
         * */
        Instruction_assignment * check = new Instruction_assignment(
            new ItemOp( 
                arrAccess->addr,
                new ItemConstant(0),
                OpType::eq
            ),
            isArrAllocated
        );
        instsProcessed.push_back(check);

        /**
         * br %newV :F :C
         * */
        ItemLabel * falseLabel =  LA::GENLV->get_new_label();
        ItemLabel * contLabel =  LA::GENLV->get_new_label();

        Instruction_branch_cond * br = new Instruction_branch_cond(
            falseLabel,         /* dst1 */
            contLabel,          /* dst2 */
            isArrAllocated      /* cond */
        );  
        instsProcessed.push_back(br);

        /**
         *  :F
         * */
        Instruction_label * falseLbInst = new Instruction_label(falseLabel);
        instsProcessed.push_back(falseLbInst);


        /**
         *  tensor-error(%LineNumber)
         * */
        ItemConstant * lineN = new ItemConstant(arrAccess->lineNumber);
        /**
         *  TODO: encode here?
         * */
        lineN->encodeItself();
        
        std::vector<Item *> args =  {lineN};
        ItemCall * callwrap = new ItemCall(
            true,               /* isRuntime */
            true,                /* call on function name */
            &LA::tensor_FName,   /* callee */
            args                /* args */
        );

        Instruction_call * call = new Instruction_call( callwrap );
        instsProcessed.push_back(call);


        /**
         *  :C
         * */
        Instruction_label * contLbInst = new Instruction_label(contLabel);
        instsProcessed.push_back(contLbInst);
    }

    void checkBoundarySingle(
        std::vector<Instruction *> & instsProcessed,
        ItemArrAccess * arrAccess
    ) {
        assert(arrAccess->offsets.size() == 1);

        /**
         *  l_i <- length ar 0
         *  check <- arrAccess->offsets[0] >= l_i)
         *  br check :False :Cont
         *  
         *  :False
         *      tensor-error(int64 line, int64 length, int64 index)
         *  :Cont        
         * */
        

        /**
         *  arrLength <- length ar 0
         * */
        int dim = 0;
        /* dimensions not encoded, never encoded */
        ItemConstant *itemDim = new ItemConstant (dim);
        ItemLength * lenQ = new ItemLength(
            arrAccess->addr,        /* addr */
            itemDim
        );

        /**
         *  arrlength is already encoded
         * */
        ItemVariable * arrLength = LA::GENLV->get_new_var(VarType::int64);
        Instruction_declare * decArrlen = new Instruction_declare(&LA::int64Sig, arrLength);
        instsProcessed.push_back(decArrlen);
        
        Instruction_assignment * assignlen = new Instruction_assignment(
            lenQ,           /* src */
            arrLength       /* dst */
        );
        instsProcessed.push_back(assignlen);

        /**
         *  encode idx from arrAccess->offsets[0]
         * */
        ItemVariable * idxEncoded = encodeVarItem(
            instsProcessed, 
            arrAccess->offsets[0]
        );

        /**
         * isOutOfBound <- idxEncoded >= arrLength
         * */
        ItemVariable *isOutOfBound = LA::GENLV->get_new_var(VarType::int64);
        Instruction_declare * decisOutOfBound = new Instruction_declare(&LA::int64Sig, isOutOfBound);
        instsProcessed.push_back(decisOutOfBound);

        Instruction_assignment * check = new Instruction_assignment(
            new ItemOp( 
                idxEncoded ,             /* op1 */
                arrLength,                          /* op2 */
                OpType::geq                        /* >= */
            ),
            isOutOfBound
        );
        instsProcessed.push_back(check);
        
        
        /**
         * br %isOutOfBound :F :C
         * */
        ItemLabel * errorLabel =  LA::GENLV->get_new_label();
        ItemLabel * contLabel =  LA::GENLV->get_new_label();

        Instruction_branch_cond * br = new Instruction_branch_cond(
            errorLabel,         /* dst1 */
            contLabel,          /* dst2 */
            isOutOfBound      /* cond */
        );  
        instsProcessed.push_back(br);

        /**
         *  :F
         * */
        Instruction_label * errorLbInst = new Instruction_label(errorLabel);
        instsProcessed.push_back(errorLbInst);


        /**
         *  tensor-error(%LineNumber)
         * */
        ItemConstant * lineN = new ItemConstant(arrAccess->lineNumber);
        /**
         *  TODO: encode here?
         * */
        lineN->encodeItself();
        
        std::vector<Item *> args =  {
            lineN,                      /* linenumber */
            arrLength,                  /* length of arr */
            idxEncoded                  /* index */
        };
        
        ItemCall * callwrap = new ItemCall(
            true,               /* isRuntime */
            true,                /* call on function name */
            &LA::tensor_FName,   /* callee */
            args                /* args */
        );

        Instruction_call * call = new Instruction_call( callwrap );
        instsProcessed.push_back(call);

        /**
         *  :C
         * */
        Instruction_label * contLbInst = new Instruction_label(contLabel);
        instsProcessed.push_back(contLbInst);
    }

    void checkBoundaryMultiple(
        std::vector<Instruction *> &instsProcessed,
        ItemArrAccess *arrAccess
        )
    {
        assert(arrAccess->offsets.size() > 1);
        
        /**
         *  LA/IR code  
         *  
         *  itemDim <- 0
         *  arrLength <- length ar 0
         *  encodeIdx  <-      encode(arrAccess->offsets[0])
         *  isOutOfBound <- encodeIdx >= arrLength
         *  br isOutOfBound :errorFinal :cont1
         *  :cont1
         * 
         *  itemDim <- 1
         *  arrLength <- length ar 1
         *  encodeIdx  <-      encode(arrAccess->offsets[1])
         *  isOutOfBound <- encodeIdx >= arrLength)
         *  br isOutOfBound :errorFinal :cont2
         *  :cont2
         * 
         *  itemDim <- 2
         *  arrLength <- length ar 2
         *  encodeIdx  <-      encode(arrAccess->offsets[2])
         *  isOutOfBound <- encodeIdx >= arrLength)
         *  br isOutOfBound :errorFinal :cont3
         *  :cont3 
         *  br contFinal
         *  
         *  ...
         *  :errorFinal
         *      tensor-error(
         *          int64 line,         <- figure out at compile time
         *          dim,                <- figure out at run time
         *          arrLen,            <- figure out at run time
         *          encodeIdx         <- figure out at run time
         *      )
         *  :contFinal        
         * 
         * */
        ItemVariable * itemDim   = LA::GENLV->get_new_var(VarType::int64);
        ItemVariable * arrLength = LA::GENLV->get_new_var(VarType::int64);
        ItemVariable * encodeIdx = LA::GENLV->get_new_var(VarType::int64);
        ItemVariable * isOutOfBound = LA::GENLV->get_new_var(VarType::int64);
        Instruction_declare * dec1 = new Instruction_declare(&LA::int64Sig, itemDim);
        instsProcessed.push_back(dec1);
        Instruction_declare * dec2 = new Instruction_declare(&LA::int64Sig, arrLength);
        instsProcessed.push_back(dec2);
        Instruction_declare * dec3 = new Instruction_declare(&LA::int64Sig, encodeIdx);
        instsProcessed.push_back(dec3);
        Instruction_declare * dec4 = new Instruction_declare(&LA::int64Sig, isOutOfBound);
        instsProcessed.push_back(dec4);

        ItemLabel * finalErrorLabel =  LA::GENLV->get_new_label();
        ItemLabel * finalContLabel =  LA::GENLV->get_new_label();

        for (int32_t dim = 0; dim  < arrAccess->offsets.size(); dim++) {
            /* dimensions not encoded, never encoded */


            /**
             *  itemDim <- dim(encoded)
             * */
            ItemConstant * curDim = new ItemConstant(dim);
            curDim->encodeItself();
            Instruction_assignment * assignDim = new Instruction_assignment(
                curDim,             /* src */
                itemDim              /* dst */
            );
            instsProcessed.push_back(assignDim);


            /**
             *  arrLength <- length ar dim(raw)
             * */
            ItemConstant *itemDim = new ItemConstant (dim);
            ItemLength * lenQ = new ItemLength(
                arrAccess->addr,        /* addr */
                itemDim
            );
            Instruction_assignment * assignlen = new Instruction_assignment(
                lenQ,           /* src */
                arrLength       /* dst */
            );
            instsProcessed.push_back(assignlen);


            /**
             *  encodeIdx  <-      encode(arrAccess->offsets[0])
             * */
            ItemVariable * encodedRes =  encodeVarItem(
                instsProcessed,
                arrAccess->offsets[dim]
            );
            Instruction_assignment * assignEncodeIdx = new Instruction_assignment(
                encodedRes,              /* src */
                encodeIdx               /* dst */
            );
            instsProcessed.push_back(assignEncodeIdx);


            /**
             *  check <- encodeIdx >= arrLength
             * */
            Instruction_assignment * check = new Instruction_assignment(
                new ItemOp( 
                    encodeIdx ,                        /* op1 */
                    arrLength,                          /* op2 */
                    OpType::geq                        /* >= */
                ),
                isOutOfBound
            );
            instsProcessed.push_back(check);

            /**
             *  br isOutOfBound :F :c        if dim is not the last dimension
             *  br isOutOfBound :F :finalCont   if dim is the last dimension 
             * */
            ItemLabel * currCont = LA::GENLV->get_new_label();

            Instruction_branch_cond * br = new Instruction_branch_cond(
                    finalErrorLabel,            /* dst1 */
                    currCont,                  /* dst2 */
                    isOutOfBound                /* cond */
                );  
            instsProcessed.push_back(br);

            /**
             *  :cont{dim}
             * */
            Instruction_label * contLbInst = new Instruction_label(currCont);
            instsProcessed.push_back(contLbInst);
        }

        Instruction_branch * gotoFinalCont = new Instruction_branch(finalContLabel);
        instsProcessed.push_back(gotoFinalCont);


        /**
         *  :errorFinal
         * */
        Instruction_label * finalErrorLbInst = new Instruction_label(finalErrorLabel);
        instsProcessed.push_back(finalErrorLbInst);



        /**
         * :errorFinal
         *      tensor-error(
         *          int64 line,         <- figure out at compile time
         *          dim,                <- figure out at run time
         *          arrLen,            <- figure out at run time
         *          encodeIdx         <- figure out at run time
         *      )
         * 
         * */
        ItemConstant * lineN = new ItemConstant(arrAccess->lineNumber);
        /**
         *  encode here!
         * */
        lineN->encodeItself();
        
        std::vector<Item *> args =  {
            lineN,                      /* linenumber */
            itemDim,                     /* dimension d */
            arrLength,                  /* length of arr */
            encodeIdx                  /* index */
        };
        
        ItemCall * callwrap = new ItemCall(
            true,               /* isRuntime */
            true,                /* call on function name */
            &LA::tensor_FName,   /* callee */
            args                /* args */
        );

        Instruction_call * call = new Instruction_call( callwrap );
        instsProcessed.push_back(call);

        /**
         *  :errorCont
         * */
        Instruction_label * finalContLbInst = new Instruction_label(finalContLabel);
        instsProcessed.push_back(finalContLbInst);
    }
    
    void checkBoundary(
        std::vector<Instruction *> & instsProcessed,
        ItemArrAccess * arrAccess
    ) {
        // If tuple, do nothing (tuple no rights)
        ItemVariable * arr_accessed = (ItemVariable *) arrAccess->addr;
        ItemTypeSig * arr_accessed_type = (ItemTypeSig *) arr_accessed->typeSig;
        if(arr_accessed_type->vtype == VarType::tuple) {
            return;
        }
        if (arrAccess->offsets.size() == 1) {
            checkBoundarySingle(
                instsProcessed,
                arrAccess
            );
        }  
        else {
            checkBoundaryMultiple(
                instsProcessed,
                arrAccess
            );   
        }
        
    }

    
}