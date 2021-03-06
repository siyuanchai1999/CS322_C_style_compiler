#include "code_generator.h"

#ifdef CODE_GEN_DEBUG
#define DEBUG_OUT (std::cerr << "DEBUG-Code-Generator: ") // or any other ostream
#else
#define DEBUG_OUT 0 && std::cerr
#endif


namespace IR {
    const int32_t VAL_WIDTH = 8;

    int64_t getEncoded(int64_t c) {
        return (c << 1) + 1;
    }

    int64_t getDecoded(int64_t c) {
        return c >> 1;
    }


    // void outputL3Insts(
    //     std::ofstream * out,
    //     std::vector<L3::Instruction> & L3Insts
    // ) {
    //     for (L3::Instruction & inst : L3Insts) {
    //         *out << '\t' << inst.to_string();
    //     }
    // }

    ItemVariable * InstL3GenVisitor::get_new_var() {
        ItemVariable * v = new ItemVariable(
            newVarPrefix + std::to_string(this->newVarIdx),
            & IR::int64Sig          // very hacky
        );

        this->newVarIdx++;
        this->newvars.push_back(v);

        return v;
    }

    void InstL3GenVisitor::clean_new_vars() {
        for (ItemVariable * v: this->newvars) {
            delete v;
        }
    }


    InstL3GenVisitor::InstL3GenVisitor() {
        this->out = NULL;
        this->newVarPrefix = "";
        this->newVarIdx = 0;
    }

    InstL3GenVisitor::InstL3GenVisitor(
        std::ofstream * outputFile,
        std::string & newVarPrefix
    ) {
        this->out = outputFile;
        this->newVarPrefix = newVarPrefix;
        this->newVarIdx = 0;
    }

    void InstL3GenVisitor::visit(Instruction_label * lb) {
        *this->out << '\t' << lb->to_string();
    }

    /**
    *  Terminator of Basic Block
    * */
    void InstL3GenVisitor::visit(Instruction_ret * ret) {
        *this->out << '\t' << ret->to_string();
    }

    void InstL3GenVisitor::visit(Instruction_ret_var * ret) {
        *this->out << '\t' << ret->to_string();
    }

    void InstL3GenVisitor::visit(Instruction_branch * br) {
        *this->out << '\t' << br->to_string();
    }

    void InstL3GenVisitor::visit(Instruction_branch_cond * condBr) {
        /**
         *  IR: br t labelT labelF
         *  L3: br t labelT
         *      br labelF
         * */
        std::string inst_str = "br ";
        inst_str += condBr->condition->to_string();
        inst_str += " ";
        inst_str += condBr->dst1->to_string();
        inst_str += "\n";

        *this->out << '\t' << inst_str;

        inst_str = "br ";
        inst_str += condBr->dst2->to_string();
        inst_str += "\n";

        *this->out << '\t' << inst_str;
    
    }   

    /**
     *  Normal instruction
     * */
    void InstL3GenVisitor::visit(Instruction_declare *) {
        /**
         *  nothng to do declare
         * */
    }
    void InstL3GenVisitor::visit(Instruction_call * call) {
        /**
         *  IR: call callee ( args? )
         *  L3: call callee ( args )
         * */

        *this->out << '\t' << call->to_string();
    }

    void InstL3GenVisitor::output_inst_tab(std::string & inst) {
        *this->out << '\t' << inst;
    }


    void InstL3GenVisitor::output_encode(Item * v) {
        /**
         *  %v0 <- %v0 << 1 
         *  %v0 <- %v0 + 1
         * */
        std::string inst = "";
        inst += v->to_string();
        inst += " <- ";
        inst += v->to_string();
        inst += " << ";
        inst += "1";
        inst += "\n";

        this->output_inst_tab(inst);

        inst.clear();
        inst += v->to_string();
        inst += " <- ";
        inst += v->to_string();
        inst += " + ";
        inst += "1";
        inst += "\n";

        this->output_inst_tab(inst);
    }

    void InstL3GenVisitor::output_newTupleInst(
        Item * dst,
        ItemNewTuple * newTuple
    ) {
        /**
         *  %t <- call allocate(newTuple->, 1) 
         * */
        ItemConstant constOne(1);
        
        std::string inst = "";
        inst += dst->to_string();
        inst += " <- ";
        inst += " call ";
        inst += " allocate ";
        inst += "(";
        inst += newTuple->len->to_string();
        inst += ", ";
        inst += "1";
        inst += ")";
        inst += "\n";
        this->output_inst_tab(inst);
    }

    void InstL3GenVisitor::output_newArrayInst(
        Item * dst,
        ItemNewArray * newArr
    ) {
        /**
         *  decode every dim arg
         * 
         *  %p1D <- %p1 >> 1
         *  %p2D <- %p2 >> 1
         * */

        assert(newArr->dims.size() > 0);
        
        std::vector<ItemVariable *> decoded_ndims (newArr->dims.size());
        std::string inst = "";

        for (int32_t i = 0; i < newArr->dims.size(); i++) {
            ItemVariable * decoded = this->get_new_var();
            decoded_ndims[i] = decoded;

            /**
             *  L3: %p1D <- %p1 >> 1
             * */
            inst.clear();
            inst += decoded->to_string();
            inst += " <- ";
            inst += newArr->dims[i]->to_string();
            inst += " >> ";
            inst += "1\n";
            
            this->output_inst_tab(inst);
        }

        /**
         *  multiply all decoded to get real length of the array
         * */
        ItemVariable * arrTotalLength = this->get_new_var();
        /**
         *  L3: %arrTotalLength <- decoded_ndims[0]
         * */
        inst.clear();
        inst += arrTotalLength->to_string();
        inst += " <- ";
        inst += decoded_ndims[0]->to_string();
        inst += "\n";
        
        this->output_inst_tab(inst);

        for (int32_t i = 1 ; i < decoded_ndims.size(); i++) {
            /**
             *  L3: %arrTotalLength <- %arrTotalLength * decoded_ndims[i]
             * */
            inst.clear();
            inst += arrTotalLength->to_string();
            inst += " <- ";
            inst += arrTotalLength->to_string();
            inst += " * ";
            inst += decoded_ndims[i]->to_string();
            inst += "\n";

            this->output_inst_tab(inst);
        }

         /**
         *  L3: %arrTotalLength <- %arrTotalLength + str(ndims + 1)
         * */
        int32_t offsets = newArr->dims.size() + 1;
        inst.clear();
        inst += arrTotalLength->to_string();
        inst += " <- ";
        inst += arrTotalLength->to_string();
        inst += " + ";
        inst += std::to_string(offsets);
        inst += "\n";

        this->output_inst_tab(inst);

        /**
         *  Encode it 
         *  %arrTotalLength <- %arrTotalLength << 1 
         *  %arrTotalLength <- %arrTotalLength + 1
         * */
        this->output_encode(arrTotalLength);

        /**
         *  %a <- call allocate(%v0, 1)
         * */
        inst.clear();
        inst += dst->to_string();
        inst += " <- ";
        inst += " call ";
        inst += " allocate ";
        inst += "(";
        inst += arrTotalLength->to_string();
        inst += ", ";
        inst += "1";
        inst += ")";
        inst += "\n";
        this->output_inst_tab(inst);

        /**
         *  %arrPtr <- src + 8
         */ 
        ItemVariable * arrPtr = this->get_new_var();
        
        inst.clear();
        inst += arrPtr->to_string();
        inst += " <- ";
        inst += dst->to_string();
        inst += " + ";
        inst += std::to_string(VAL_WIDTH);
        inst += "\n";
        this->output_inst_tab(inst);

        /**
         *  store %arrPtr <- str(encode(ndims)
         */ 
        int64_t ndimsEncoded = getEncoded(newArr->dims.size());
        inst.clear();
        inst += "store ";
        inst += arrPtr->to_string();
        inst += " <- ";
        inst += std::to_string(ndimsEncoded);
        inst += "\n";
        this->output_inst_tab(inst);

        for (int32_t i = 0; i < newArr->dims.size(); i++) {
            /**
             *  %arrPtr <- %arrPtr + 8
             */ 
            inst.clear();
            inst += arrPtr->to_string();
            inst += " <- ";
            inst += arrPtr->to_string();
            inst += " + ";
            inst += std::to_string(VAL_WIDTH);
            inst += "\n";
            this->output_inst_tab(inst);

            /**
             *  store %arrPtr <- %p[i]
             */
            inst.clear();
            inst += "store ";
            inst += arrPtr->to_string();
            inst += " <- ";
            inst += newArr->dims[i]->to_string();
            inst += "\n";
            this->output_inst_tab(inst);
        }
 
    }
        /* is this what im supposed to do */

    void InstL3GenVisitor::output_operatorInst(
        Item * dst,
        Item * op1,
        OpType op,
        Item * op2
    )
    {   
        assert(dst != NULL);
        assert(op1 != NULL);
        assert(op2 != NULL);

        ItemOp opitem(op1, op2, op);
        
        this->output_AssignInst(
            dst,
            &opitem
        );
    }

    void InstL3GenVisitor::output_AssignInst(
        Item * dst,
        Item * src
    ) {
        assert(dst != NULL);
        assert(src != NULL);
        
        std::string inst = "";

        inst += dst->to_string();
        inst += " <- ";
        inst += src->to_string();
        inst += "\n";

        this->output_inst_tab(inst);
    }

    void InstL3GenVisitor::output_AssignCallInst(
        Item * dst,
        ItemCall * call
    ) {
        assert(dst != NULL);
        assert(call != NULL);
        
        std::string inst = "";

        inst += dst->to_string();
        inst += " <- ";
        inst += call->to_string();
        inst += "\n";

        this->output_inst_tab(inst);
    }

    void InstL3GenVisitor::output_LoadInst(
        Item *dst,
        Item *addr
    ) {
        assert(dst != NULL);
        assert(addr != NULL);
        std::string inst = "";
        inst += dst->to_string();
        inst += " <- load ";
        inst += addr->to_string();
        inst += "\n";
        this->output_inst_tab(inst);
        
    }

    void InstL3GenVisitor::output_StoreInst(
        Item *addr, 
        Item *src
    ) {
        assert(addr != NULL);
        assert(src != NULL);
        std::string inst = "";
        inst += "store ";
        inst += addr->to_string();
        inst += " <- ";
        inst += src->to_string();
        inst += "\n";
        this->output_inst_tab(inst);
    }

    
    Item * InstL3GenVisitor::output_addr_loadStore(ItemArrAccess *arr_acc)
    {
        assert(arr_acc->addr->itemtype == ItemType::item_variable);
        ItemVariable * addrVar = (ItemVariable *) arr_acc->addr;
        assert(addrVar->typeSig->itemtype == ItemType::item_type_sig);
        ItemTypeSig * addrVarType = (ItemTypeSig *) addrVar->typeSig;
        
        if (addrVarType->vtype == VarType::tuple) 
        {
            
            /**
             *  Tuple acces should only have one offset
             * */
            assert(arr_acc->offsets.size() == 1);

            ItemConstant constValWidth(VAL_WIDTH);
            ItemVariable * addr = get_new_var();

            /**
             * %addr <- %addrVar + 8
             * %offset <- arr_acc->offsets[0] * 8 
             * %addr <- %addr + %offset
             */
            
            this->output_operatorInst(
                addr,                               /* dst */
                addrVar,                               /* op1 */
                OpType::plus,                       /* op */
                &constValWidth                      /* op2 */
            );

            ItemVariable * offset = get_new_var();
            this->output_operatorInst(
                offset,                               /* dst */
                arr_acc->offsets[0],                /* op1 */
                OpType::times,                      /* op */
                &constValWidth                      /* op2 */
            );

            this->output_operatorInst(
                addr,                               /* dst */
                addr,                               /* op1 */
                OpType::plus,                       /* op */
                offset                      /* op2 */
            );

            

            return addr;

        } 
        else if (addrVarType->vtype == VarType::tensor) 
        {

            int32_t ndims = arr_acc->offsets.size();
        
            
            ItemVariable * addr = get_new_var();

            /**
             *  %addr <- %arr + str(2 * VAL_WIDTH)
             * */
            ItemConstant init2BytesLen( 2 * VAL_WIDTH);
            this->output_operatorInst(
                addr,
                arr_acc->addr,
                OpType::plus,
                &init2BytesLen
            );


            std::vector<ItemVariable *> decodeArrDimLen (ndims);
            ItemConstant constOne(1);
            ItemConstant constValWidth(VAL_WIDTH);

            for (int32_t i = 0; i < ndims; i++) {
                /**
                 *  %decodeArrDimLen[i] <- load %addr
                 *  
                 *   %decodeArrDimLen[i] <- %decodeArrDimLen[i] >> 1
                 * 
                 *  %addr <- %addr + 8
                 * */
                ItemVariable * decoded = get_new_var();
                decodeArrDimLen[i] = decoded;

                this->output_LoadInst(
                    decoded,    /* dst */
                    addr        /* load address */
                );
                
                this->output_operatorInst(
                    decoded,                /* dst */
                    decoded,                /* op1 */
                    OpType::shift_right,    /* op */
                    &constOne               /* op2 */
                );

                this->output_operatorInst(
                    addr,                /* dst */
                    addr,                /* op1 */
                    OpType::plus,       /* op */
                    &constValWidth      /* op2 */
                );
            }
            

            /**
             *  arrDimBufSize[i] = decodeArrDimLen[i + 1] * ... decodeArrDimLen[nDims - 1]
             * 
             *  In other words,
             *      arrDimBufSize[i] =  arrDimBufSize[i + 1] * decodeArrDimLen[i + 1] if i < nDims - 1
             *      
             *      arrDimBufSize[nDims - 1] = 1 
             *      
             * 
             * */
            std::vector<ItemVariable *> arrDimBufSize (ndims);
            
            for (int32_t i = ndims - 1; i >= 0; i--) {
                /**
                 *  arrDimBufSize[i] =  arrDimBufSize[i + 1] * decodeArrDimLen[i + 1] if i < nDims - 1
                 *  arrDimBufSize[nDims - 1] = 1 
                 * */
                ItemVariable * arrBufSize = get_new_var();
                arrDimBufSize[i] = arrBufSize;

                if (i == ndims - 1) {
                    this->output_AssignInst(
                        arrBufSize,         /* dst */
                        &constOne           /* src */
                    );
                } 
                else 
                {
                    this->output_operatorInst(
                        arrBufSize,                             /* dst */
                        arrDimBufSize[i + 1],                /* op1 */
                        OpType::times,                          /* op */
                        decodeArrDimLen[i + 1]                 /* op2 */
                    );
                }
            }

            ItemVariable * dimOffset = get_new_var();
            for (int32_t i = 0 ; i < ndims; i++) {
                /**
                 *  %dimOffset <- arrDimBufSize[i] * 8
                 *  %dimOffset <- %dimOffset * arr_acc->offsets[i]
                 *  %addr <- %arr + %dimOffset
                 * 
                 *  Note:
                 *      offsets don't need to be decoded. It is already encoded
                 * */
                this->output_operatorInst(
                    dimOffset,                             /* dst */
                    arrDimBufSize[i],                /* op1 */
                    OpType::times,                          /* op */
                    &constValWidth                 /* op2 */
                );
                
                this->output_operatorInst(
                    dimOffset,                             /* dst */
                    dimOffset,                              /* op1 */
                    OpType::times,                          /* op */
                    arr_acc->offsets[i]                 /* op2 */
                );

                this->output_operatorInst(
                    addr,                             /* dst */
                    addr,                              /* op1 */
                    OpType::plus,                          /* op */
                    dimOffset                 /* op2 */
                );
                
            }


            return addr;

        } 
        else 
        { 
            DEBUG_OUT << "errot type: " << addrVarType->vtype << '\n';
            assert(0);
        }

        
        return NULL;
    }

    void InstL3GenVisitor::output_varFromArrAccess (
        Item * dst,
        ItemArrAccess * arrAccess
    ) {
        assert(arrAccess->itemtype == ItemType::item_ArrAccess);
        Item * addr = this->output_addr_loadStore(
            arrAccess
        );

        assert(addr != NULL);

        this->output_LoadInst(
            dst,
            addr
        );
    }

    void InstL3GenVisitor::output_ArrAccessFromAll (
        ItemArrAccess * arrAccess,
        Item * src
    ) {

        assert(arrAccess->itemtype == ItemType::item_ArrAccess);

        Item * addr = this->output_addr_loadStore(
            arrAccess
        );

        assert(addr != NULL);

        this->output_StoreInst(
            addr,
            src
        );
    }

    void InstL3GenVisitor::output_AssignLengthInst(
        Item * dst,
        ItemLength * len
    ) {
        assert(dst != NULL);
        assert(len != NULL);

        assert(len->addr->itemtype == ItemType::item_variable);
        ItemVariable * addrVar = (ItemVariable *) len->addr;
        assert(addrVar->typeSig->itemtype == ItemType::item_type_sig);
        ItemTypeSig * addrVarType = (ItemTypeSig *) addrVar->typeSig;
        
        assert(addrVarType->vtype == VarType::tensor || addrVarType->vtype == VarType::tuple);
        if (addrVarType->vtype == VarType::tuple) {
            this->output_LoadInst(
                dst,
                len->addr
            );
            return;
        }
        /**
         *  %addr <- %len->addr + str(2 * VAL_WIDTH)
         *  %offset <- len->ndim * str(VAL_WIDTH)
         *  %addr <- %addr + %offset
         * */
        ItemVariable * addr = get_new_var();


        ItemConstant init2BytesLen(2 * VAL_WIDTH);
        this->output_operatorInst(
            addr,
            len->addr,
            OpType::plus,
            &init2BytesLen
        );

        ItemVariable * offset = get_new_var();
        ItemConstant constValWidth(VAL_WIDTH);

        this->output_operatorInst(
            offset,                               /* dst */
            len->dim,                           /* op1 */
            OpType::times,                      /* op */
            &constValWidth                      /* op2 */
        );

        this->output_operatorInst(
            addr,                               /* dst */
            addr,                               /* op1 */
            OpType::plus,                       /* op */
            offset                              /* op2 */
        );

        this->output_LoadInst(
            dst,
            addr
        );

    }


    void InstL3GenVisitor::visit(Instruction_assignment * assign) {

        if (assign->dst->itemtype == ItemType::item_ArrAccess) {
            /* var <- var([t])+ */
            this->output_ArrAccessFromAll(
                (ItemArrAccess *) assign->dst,
                assign->src
            );

        }
        else {
            /**
             *  src must be a var
             * */
            
            switch (assign->src->itemtype)
            {   
                case ItemType::item_labels :
                {
                    this->output_AssignInst(
                        assign->dst,
                        assign->src
                    );
                    break;
                }

                case ItemType::item_constant :
                {
                    this->output_AssignInst(
                        assign->dst,
                        assign->src
                    );
                    break;
                }

                case ItemType::item_variable :
                {
                    this->output_AssignInst(
                        assign->dst,
                        assign->src
                    );
                    break;
                }

                case ItemType::item_ArrAccess:
                {
                    this->output_varFromArrAccess(
                        assign->dst,
                        (ItemArrAccess *) assign->src
                    );
                    break;
                }
                
                case ItemType::item_op :
                {   
                    ItemOp * itemop = (ItemOp *) assign->src;
                    this->output_operatorInst(
                        assign->dst,
                        itemop->op1,
                        itemop->opType,
                        itemop->op2
                    );
                    break;
                }

                case ItemType::item_call :
                {   
                    ItemCall * call = (ItemCall *) assign->src;
                    this->output_AssignCallInst(
                        assign->dst,
                        call
                    );
                    break;
                }

                case ItemType::item_newArr :
                {

                    this->output_newArrayInst(
                        assign->dst,
                        (ItemNewArray *) assign->src
                    );

                    break;
                }

                case ItemType::item_newTuple :
                {
                    this->output_newTupleInst(
                        assign->dst,
                        (ItemNewTuple *) assign->src
                    );
                    break;
                }

                
                case ItemType::item_length :
                {
                    ItemLength * len = (ItemLength *) assign->src;
                    this->output_AssignLengthInst(
                        assign->dst,
                        len
                    );
                    break;
                }

                

                

            
            default:
                break;
            }
        }
    }

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

    /**
     *  arg0, arg1, arg2
     * */
    void output_args_helper(std::ofstream & out, std::vector<Item *> & args) {
        for (int32_t i = 0; i < args.size(); i++) {
            if (i > 0) {
                out << ", ";
            }
            
            out << args[i]->to_string(); 
        }
    }

    generateCodeForTraces::generateCodeForTraces(std::ofstream * outputFile, std::string & newVarPrefix) {
        
        this->L3InstGen = InstL3GenVisitor(outputFile, newVarPrefix);
        this->out = outputFile;
    }

    bool canMerge(BasicBlock * curBB, BasicBlock * nextBB) {
        bool hasOneSucc = curBB->succs.size() == 1;
        if (hasOneSucc){
            /* wo kun le hao le jiao wo */
            if (IN_SET(curBB->succs, nextBB)) {
                if (nextBB->preds.size() == 1 && *nextBB->preds.begin() == curBB) {
                    return true;
                }
            }
        }

        return false;

    }

    bool canRemoveLabel(BasicBlock * curBB, BasicBlock * prevBB) {

        bool hasOneSucc = curBB->preds.size() == 1;
        if (hasOneSucc){
            if (IN_SET(curBB->preds, prevBB)) {
                return true;
            }
        }

        return false;

    }

    bool canRemoveTE(BasicBlock * curBB, BasicBlock * nextBB) {

        /* uncond branch */
        bool hasOneSucc = curBB->succs.size() == 1;
        if (hasOneSucc){
            if (IN_SET(curBB->succs, nextBB)) {
                return true;
            }
        }
        return false;

    }

    void generateCodeForTraces::generateL3code(std::vector<Trace *> & traces) {
        
        for (Trace * tr : traces) {
            
            bool noNeedNextLabel = false;
            bool noNeedCurTE = false;
            
            for (int32_t i = 0; i < tr->jointBBs.size(); i++) {
                noNeedCurTE = false;
                BasicBlock * curBB = tr->jointBBs[i];

                if (!noNeedNextLabel) {
                    curBB->label->accept(this->L3InstGen);
                }

                for (Instruction * inst : curBB->insts) {
                    inst->accept(this->L3InstGen);
                }

                if (i < tr->jointBBs.size() - 1) {
                    BasicBlock * nextBB = tr->jointBBs[i + 1];
                    bool twoBBCanMerge = canMerge(curBB, nextBB);
                    noNeedNextLabel = twoBBCanMerge;
                    noNeedCurTE = twoBBCanMerge;
                }

                if (!noNeedCurTE) {
                    curBB->te->accept(this->L3InstGen);
                }
                *this->out << "\n";
            }

        }
    }

    // void generateCodeForTraces::generateL3code(std::vector<Trace *> & traces) {
        
    //     for (Trace * tr : traces) {
            
            
    //         for (int32_t i = 0; i < tr->jointBBs.size(); i++) {
    //             BasicBlock * curBB = tr->jointBBs[i];

    //             if (i > 1) {
    //                 bool canRemove = canRemoveLabel(curBB, tr->jointBBs[i - 1]);
    //                 if (!canRemove) {
    //                     curBB->label->accept(this->L3InstGen);
    //                 }
    //             } else {
    //                 curBB->label->accept(this->L3InstGen);
    //             }
            

    //             for (Instruction * inst : curBB->insts) {
    //                 inst->accept(this->L3InstGen);
    //             }

    //             if (i < tr->jointBBs.size() - 1) {
    //                 BasicBlock * nextBB = tr->jointBBs[i + 1];
    //                 bool canRemove = canRemoveTE(curBB, nextBB);
    //                 if (!canRemove) {
    //                     curBB->te->accept(this->L3InstGen);
    //                 }
    //             } else {
    //                 curBB->te->accept(this->L3InstGen);
    //             }
    //         }

    //     }

    //     this->L3InstGen.clean_new_vars();
    // }

    void generateCode(
        Program & p
    ) {
        std::string varPrefix = new_var_prefix(p);

        std::ofstream out =  std::ofstream();
        out.open("prog.L3");
        
        for (Function * F : p.functions) {
            out << "define ";
            out << F->name->to_string();
            
            out << "(";
            output_args_helper(out, F->arg_list);
            out << ")";
            
            out << "{\n";

            std::vector<Trace *> traces = runGenerateTrace(F);            
            generateCodeForTraces traceCodeGen(&out, varPrefix);
            traceCodeGen.generateL3code(traces);

            out << "}\n";

        }

        out.close();

    }
}

