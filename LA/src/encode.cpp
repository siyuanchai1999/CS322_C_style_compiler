#include "encode.h"

#ifdef ENCODE_DEBUG
#define DEBUG_OUT (std::cerr << "DEBUG-IR-Driver: ") // or any other ostream
#else
#define DEBUG_OUT 0 && std::cerr
#endif

namespace LA {
    void encode_program(Program & p) {
        /**
         *  1. Encode all constants not used for array/tuple indices 
         *  and 2nd parameter of length
         * */
        for (Function * F : p.functions) {
            for (ItemConstant * c : F->constToEncode) {
                c->encodeItself();
            }
        }

        
        for (Function * F : p.functions) {
            InstructionEncodingVisitor instEncoder;
            
            for (Instruction * inst : F->insts) {
                inst->accept(instEncoder);
            }

            F->insts = instEncoder.instsProcessed;
        } 

    }
    void InstructionEncodingVisitor::visit(Instruction_label * lb) {
        /* does nothing except push */
        this->instsProcessed.push_back(lb);
        return;
    }

    /**
     * Terminator of Basic Block
     **/
    void InstructionEncodingVisitor::visit(Instruction_ret * ret) {
        /* does nothing except push */
        this->instsProcessed.push_back(ret);
        return;
    }
    void InstructionEncodingVisitor::visit(Instruction_ret_var * ret) {
        /* does nothing except push */
        this->instsProcessed.push_back(ret);
        return;
    }
    void InstructionEncodingVisitor::visit(Instruction_branch * br) {
        /* does nothing except push */
        this->instsProcessed.push_back(br);
        return;
    }
    
    void InstructionEncodingVisitor::visit(Instruction_branch_cond *branch_cond) {
        /** br t :true_label :false:label
         * decodes t
         * */
        if (branch_cond->condition->itemtype == ItemType::item_variable) 
        {
            ItemVariable *condition = (ItemVariable *) branch_cond->condition;
            ItemVariable *new_condition = decodeVarItem(
                this->instsProcessed,
                condition
            );
            
            branch_cond->condition = new_condition;
        } 
        else if (branch_cond->condition->itemtype == ItemType::item_constant) 
        {
            /* already encoded all constants, need to decode back  */
            ItemConstant * c = (ItemConstant *) branch_cond->condition;
            c->decodeItself();
        }

        this->instsProcessed.push_back(branch_cond);
    }

    /**
     *  Normal instruction 
     **/
    void InstructionEncodingVisitor::visit(Instruction_declare *declare)
    {
        this->instsProcessed.push_back(declare);

        VarType declaredType = declare->getTypeSig();
        switch (declaredType)
        {
        case VarType::int64:
        {   
            ItemConstant * constOne = new ItemConstant(1, true);

            Instruction_assignment *initialize_to_one = new Instruction_assignment(
                constOne,
                declare->var
            );
            this->instsProcessed.push_back(initialize_to_one);
            break;
        }
        case VarType::code: 
        case VarType::tuple:
        case VarType::tensor: {
            Instruction_assignment *initialize_to_zero = new Instruction_assignment(
               new ItemConstant(0, true),
               declare->var
            );
            this->instsProcessed.push_back(initialize_to_zero);
            break;
        }

        default:
            /* wtf */
            assert(0);
            break;
        }
    }
    void InstructionEncodingVisitor::visit(Instruction_call * call) {
        /* does nothing except push */
        this->instsProcessed.push_back(call);
        return;
    }

    void InstructionEncodingVisitor::visit(Instruction_assignment * assign) {
        if (assign->dst->itemtype == ItemType::item_ArrAccess) {
            /* var <- var([t])+ */
            this->visitArrAccessFromAll(
                (ItemArrAccess *) assign->dst,
                assign->src
            );
            this->instsProcessed.push_back(assign);
        }
        else {
            /**
             *  src must be a var
             * */
            
            switch (assign->src->itemtype)
            {   
                case ItemType::item_labels :
                {
                    /* does nothing except push */
                    this->instsProcessed.push_back(assign);
                    break;
                }

                case ItemType::item_constant :
                {
                    /* does nothing except push */
                    this->instsProcessed.push_back(assign);
                    break;
                }

                case ItemType::item_variable :
                {
                    /* does nothing except push */
                    this->instsProcessed.push_back(assign);
                    break;
                }

                case ItemType::item_fname :
                {
                    /* does nothing except push */
                    this->instsProcessed.push_back(assign);
                    break;
                }

                case ItemType::item_ArrAccess:
                {
                    this->visitVarFromArrAccess(
                        assign->dst,
                        (ItemArrAccess *) assign->src
                    );

                    this->instsProcessed.push_back(assign);
                    break;
                }
                
                case ItemType::item_op :
                {   
                    /**
                     *  i <- i + 1
                     *     
                     *  Decode oprd
                     *      newV <- i >> 1
                     *  
                     *  push assign
                     *      i <- newV + 1
                     *  
                     *  Encode i 
                     *      i <- i << 1
                     *      i <- i + 1
                     *  
                     * */

                    ItemOp * itemop = (ItemOp *) assign->src;
                    this->visitVarFromOPDecodeOprd(
                        assign->dst,
                        itemop
                    );
                    
                    this->instsProcessed.push_back(assign);

                    this->visitVarFromOPEncodeDest(
                        assign->dst,
                        itemop
                    );
                    break;
                }

                case ItemType::item_call :
                {   
                    /* does nothing except push */
                    this->instsProcessed.push_back(assign);
                    break;
                }

                case ItemType::item_newArr :
                {

                    /* does nothing except push */
                    this->instsProcessed.push_back(assign);

                    break;
                }

                case ItemType::item_newTuple :
                {
                    /* does nothing except push */
                    this->instsProcessed.push_back(assign);
                    break;
                }

                
                case ItemType::item_length :
                {
                    ItemLength * len = (ItemLength *) assign->src;
                    this->visitVarFromLength(
                        assign->dst,
                        len
                    );
                    this->instsProcessed.push_back(assign);
                    break;
                }
            default:
                /* wrong item type */
                std::cerr << "wrong item type = " << assign->src->itemtype << "\n";
                assert(false);
                break;
            }
        }

    }

    void InstructionEncodingVisitor::visitVarFromLength(
        Item * dst,
        ItemLength * len
    ) {
        if (len->dim->itemtype == ItemType::item_variable) 
        {

            ItemVariable * newDim = decodeVarItem(
                this->instsProcessed,
                (ItemVariable *) len->dim
            );

            len->dim = newDim;
            
            return ;
        } 
        else if (len->dim->itemtype == ItemType::item_constant) 
        {
            /* already encoded all constants, need to decode back  */
            ItemConstant * c = (ItemConstant *) len->dim;
            c->decodeItself();
            return;
        } else {
            /* you cannot have any other types here */
            assert(0);
        }
    }

    void InstructionEncodingVisitor::visitVarFromArrAccess(
        Item * dst,
        ItemArrAccess * arrAccess
    ) {
        /**
         *  you only decode indexes if it is var
         *      If it's a constant, it is already decoded   
         * */
        // std::vector<ItemVariable * > decoded (
        //     arrAccess->offsets
        // );
        for (uint32_t i = 0;  i < arrAccess->offsets.size(); i++) {
            Item * idx = arrAccess->offsets[i];
            
            if (idx->itemtype == ItemType::item_variable) {
                ItemVariable * newIdx = decodeVarItem(
                    this->instsProcessed,
                    (ItemVariable *) idx
                );

                arrAccess->offsets[i] = newIdx;
                
            } else {
                /* do nothing */
            }
        }

    }

    void InstructionEncodingVisitor::visitArrAccessFromAll(
        ItemArrAccess *arrAccess,
        Item *src)
    {
        /**
         *  you only decode indexes if it is var
         *      If it's a constant, it is already decoded   
         * */
        for (uint32_t i = 0;  i < arrAccess->offsets.size(); i++) {
            Item * idx = arrAccess->offsets[i];
            
            if (idx->itemtype == ItemType::item_variable) {
                ItemVariable * newIdx = decodeVarItem(
                    this->instsProcessed,
                    (ItemVariable *) idx
                );
                
                arrAccess->offsets[i] = newIdx;
            } else {
                /* do nothing*/
            }
            
        }
    }
    void InstructionEncodingVisitor::visitVarFromOPEncodeDest(
        Item *dst,
        ItemOp *OP
    ) {
        assert(dst->itemtype == ItemType::item_variable);

        encodeVarItemInPlace(
            this->instsProcessed,
            (ItemVariable *) dst
        );
    }

    void InstructionEncodingVisitor::visitVarFromOPDecodeOprd(
        Item *dst,
        ItemOp *OP
    ) {
        
        if (OP->op1->itemtype == ItemType::item_variable) {
            ItemVariable * newOp1 = decodeVarItem(
                this->instsProcessed,
                (ItemVariable *) OP->op1
            );
            OP->op1 = newOp1;

            
        } else {
            ItemConstant * c = (ItemConstant *) OP->op1;
            c->decodeItself();
        }

        if (OP->op2->itemtype == ItemType::item_variable) {
            ItemVariable * newOp2 = decodeVarItem(
                this->instsProcessed,
                (ItemVariable *) OP->op2
            );
            OP->op2 = newOp2;

            
        } else {
            ItemConstant * c = (ItemConstant *) OP->op2;
            c->decodeItself();
        }
    }




    ItemVariable * decodeVarItem(
        std::vector<Instruction *> & instsProcessed,
        Item * v
    ) {
        
        ItemVariable * newV = LA::GENLV->get_new_var(VarType::int64);
        Instruction_declare * dec = new Instruction_declare(&LA::int64Sig, newV);
        instsProcessed.push_back(dec);
        /**
         *  newV <- v << 1
         * */
        Instruction_assignment * shift_right = new Instruction_assignment(
            new ItemOp(                         /* src */
                v,
                new ItemConstant(1),
                OpType::shift_right
            ),
            newV                                /* dst */
        );

        instsProcessed.push_back(shift_right);
        
        return newV;
    }

    void encodeVarItemInPlace(
        std::vector<Instruction *> & instsProcessed,
        Item * v
    ) {

        /**
         *  v <- v << 1
         * */
        Instruction_assignment * shift_left = new Instruction_assignment(
            new ItemOp(                     /* src */
                v,
                new ItemConstant(1),
                OpType::shift_left
            ),
            v                               /* dst */
        );

        /**
         *  v <- v + 1
         * */
        Instruction_assignment * plus_one = new Instruction_assignment(
            new ItemOp(
                v,
                new ItemConstant(1),
                OpType::plus
            ),          
            v
        );
        
        instsProcessed.push_back(shift_left);
        instsProcessed.push_back(plus_one);
        
    }

    void decodeVarItemInPlace(
        std::vector<Instruction *> &instsProcessed,
        Item *v)
    {
        /**
         *  v <- v >> 1
         * */
        Instruction_assignment *shift_right = new Instruction_assignment(
            new ItemOp(/* src */
                       v,
                       new ItemConstant(1),
                       OpType::shift_right),
            v /* dst */
        );

        instsProcessed.push_back(shift_right);

    }

    ItemVariable * encodeVarItem(
        std::vector<Instruction *> & instsProcessed,
        Item * v
    ) {

        ItemVariable * newV = LA::GENLV->get_new_var(VarType::int64);
        Instruction_declare * dec = new Instruction_declare(&LA::int64Sig, newV);
        instsProcessed.push_back(dec);
        /**
         *  newV <- v << 1
         * */
        Instruction_assignment * shift_left = new Instruction_assignment(
            new ItemOp(                     /* src */
                v,
                new ItemConstant(1),
                OpType::shift_left
            ),
            newV                               /* dst */
        );

        /**
         *  newV <- newV + 1
         * */
        Instruction_assignment * plus_one = new Instruction_assignment(
            new ItemOp(
                newV,
                new ItemConstant(1),
                OpType::plus
            ),          
            newV
        );
        
        instsProcessed.push_back(shift_left);
        instsProcessed.push_back(plus_one);
        
        return newV;
    }

    
}