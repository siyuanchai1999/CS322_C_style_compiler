
#include "spiller.h"

#define IN_MAP(map, key) (map.find(key) != map.end())
#define ARG_BYTE_NUM 8 

namespace L2 {



    /**
     *  see if Item item of Instuction inst contains varToSpill
     *  
     *  figure out the address of varToSpill in inst to be written
     * */
    bool Has_Used_varToSpill(
        Item ** itemAddr,
        Item * varToSpill, 
        std::vector<Item **> & placeToReplace
    ){  
        Item * item = *itemAddr;
        switch (item->itemtype)
        {
            case ItemType::item_registers :
            {
                return false;
            }

            case ItemType::item_constant :
            {
                return false;
            }

            case ItemType::item_memory :
            {
                ItemMemoryAccess * ItemAccess = (ItemMemoryAccess *) item;
                return Has_Used_varToSpill(&ItemAccess->reg, varToSpill, placeToReplace);
            }

            case ItemType::item_labels :
            {
                return false;
            }

            case ItemType::item_aop :
            {
                return false;
            }
            case ItemType::item_cmp :
            {
                ItemCmp * ItemAccess = (ItemCmp *) item;
                bool op1Used = Has_Used_varToSpill(&ItemAccess->op1, varToSpill, placeToReplace);
                bool op2Used = Has_Used_varToSpill(&ItemAccess->op2, varToSpill, placeToReplace);

                return op1Used || op2Used;
            }

            case ItemType::item_variable :
            {
                if (varToSpill == item) {
                    placeToReplace.push_back(itemAddr);
                    // placeToWrite.push_back(&inst.item)
                    return true;
                }
                
                return false;
            }
            
            case ItemType::item_stack_arg :
            {
                return false;
            }

            default:
                std::cerr << "wrong ItemType in get_used_reg_var: " << item->itemtype << '\n';
                break;
        }

        return false;
    }


    SpillerVisitor::SpillerVisitor(
        ItemVariable * varToSpill,
        ItemVariable * prefix,
        Function * F
    ) {
        this->varToSpill = varToSpill;
        this->prefix = prefix;
        this->F = F;
        this->suffix_num = 0;
    }

    ItemVariable * SpillerVisitor::build_new_var_prefix_suffix() {
        std::string newSubstiut_str = this->prefix->name + std::to_string(suffix_num);
        ItemVariable * v = new ItemVariable(
            newSubstiut_str
        );

        this->suffix_num++;

        this->F->varName2ptr[newSubstiut_str] = v;

        this->var_replacements.push_back(v);
        return v;
    }

    ItemMemoryAccess * SpillerVisitor::build_stackAccess_locals() {
        ItemConstant * offset = new ItemConstant((this->F->locals - 1) * ARG_BYTE_NUM);
        
        ItemMemoryAccess * memA = new ItemMemoryAccess(
            & L2::reg_rsp,
            offset
        );

        return memA;
    }

    void SpillerVisitor::spill_Inst(
            Instruction * inst,
            bool HasRead,
            bool HasWritten,
            std::vector<Item **> & placeToReplace 
    ){
        ItemVariable * new_var = NULL;
        ItemMemoryAccess * stacklocal = NULL;
        
        if (HasRead){
            new_var = this->build_new_var_prefix_suffix(); 
            stacklocal = this->build_stackAccess_locals();

            /**
             *  mem rsp 0 <- %S0
             * */
            Instruction_assignment * fetchFromStack = new Instruction_assignment;
            fetchFromStack->type = inst_assign;
            fetchFromStack->dst = new_var;
            fetchFromStack->src = stacklocal;

            this->new_insts.push_back(fetchFromStack);
        }


        /**
         *  changed the place include at instruction 
         *  if it somewhat contains varToSpill
         * */
        if (new_var == NULL && !placeToReplace.empty()) {
            new_var = this->build_new_var_prefix_suffix(); 
        }

        for (Item ** wAddr : placeToReplace) {
            *wAddr = new_var;
        }

        this->new_insts.push_back(inst);


        if (HasWritten) 
        {   
            /**
             *  put a writeToStack instruction when it is purely write
             *  mem rsp 0 <- %S0
             * */
            if (new_var == NULL) {
                new_var = this->build_new_var_prefix_suffix(); 
            }

            if (stacklocal == NULL) {
                stacklocal = this->build_stackAccess_locals();
            }

            Instruction_assignment * writeToStack = new Instruction_assignment;
            writeToStack->type = inst_assign;
            writeToStack->dst = stacklocal;
            writeToStack->src = new_var;

            this->new_insts.push_back(writeToStack);

        }
    }

    void SpillerVisitor::visit(Instruction_ret *ret) {
        /**
         *  push original instruction
         * */

        this->new_insts.push_back(ret);
    } 

    void SpillerVisitor::visit(Instruction_label * label_inst) {
        /**
         *  push original instruction
         * */
        this->new_insts.push_back(label_inst);
    }


    void SpillerVisitor::visit(Instruction_call_runtime *runtime_call) {
        /**
         *  push original instruction
         * */
        this->new_insts.push_back(runtime_call);
    }

    void SpillerVisitor::visit(Instruction_call_user *user_call) {
        
        /**
         * user_call cannot write
         *  can only read if condition includes varToSpill
         */
        std::vector<Item **> placeToReplace;
        bool var_used = Has_Used_varToSpill(&user_call->callee, this->varToSpill, placeToReplace);
                
        this->spill_Inst(
            user_call,
            var_used,
            false,
            placeToReplace
        );
    }

    void SpillerVisitor::visit(Instruction_aop *aop) {
        
        /**
         *      Instruction reads varToSpill whenever op1Used or op2Used = true
         *      it writes varToSpill if target is varToSpill
         * */
        std::vector<Item **> placeToReplace;
        bool op1Used = Has_Used_varToSpill(&aop->op1, this->varToSpill, placeToReplace);
        bool op2Used = Has_Used_varToSpill(&aop->op2, this->varToSpill, placeToReplace);

        bool HasRead = op1Used || op2Used;
        bool HasWrite = aop->op1 == this->varToSpill;


        // std::cerr << "HasRead = " << HasRead << "HasWrite = " << HasWrite << '\n';
        this->spill_Inst(
            aop,
            HasRead,
            HasWrite,
            placeToReplace
        );

    }


    void SpillerVisitor::visit(Instruction_assignment *assignment) {
        /**
         *  Instruction reads varToSpill if it is found in src or it is found in dst as mem Reference (assignment->dst != this->varToSpill)
         *      it writes varToSpill if target is varToSpill
         * */

        std::vector<Item **> placeToReplace;
        bool op1Used = Has_Used_varToSpill(&assignment->dst, this->varToSpill, placeToReplace);
        bool op2Used = Has_Used_varToSpill(&assignment->src, this->varToSpill, placeToReplace);

        bool HasRead = (op1Used && assignment->dst != this->varToSpill) || op2Used;
        bool HasWrite = assignment->dst == this->varToSpill;

        
        this->spill_Inst(
            assignment,
            HasRead,
            HasWrite,
            placeToReplace
        );
        

    }

    void SpillerVisitor::visit(Instruction_sop *sop) {
        /**
         *  same logic with aop instruction
         *      Instruction reads varToSpill whenever targetUsed or offsetUsed = true
         *      it writes varToSpill if target is varToSpill
         * */
        std::vector<Item **> placeToReplace;
        bool targetUsed = Has_Used_varToSpill(&sop->target, this->varToSpill, placeToReplace);
        bool offsetUsed = Has_Used_varToSpill(&sop->offset, this->varToSpill, placeToReplace);

        bool HasRead = targetUsed || offsetUsed;
        bool HasWrite = sop->target == this->varToSpill;


        // std::cerr << "HasRead = " << HasRead << "HasWrite = " << HasWrite << '\n';
        this->spill_Inst(
            sop,
            HasRead,
            HasWrite,
            placeToReplace
        );

    }
    void SpillerVisitor::visit(Instruction_lea *lea) {
        std::vector<Item **> placeToReplace;

        bool spill_in_dst = Has_Used_varToSpill(&lea->dst, this->varToSpill, placeToReplace);
        bool spill_in_addr = Has_Used_varToSpill(&lea->addr, this->varToSpill, placeToReplace);
        bool spill_in_multr = Has_Used_varToSpill(&lea->multr, this->varToSpill, placeToReplace);
        
        bool has_read = spill_in_addr || spill_in_multr;
        bool has_write = spill_in_dst;
        
        this->spill_Inst(
            lea,
            has_read,
            has_write,
            placeToReplace
        );
    }
    
    void SpillerVisitor::visit(Instruction_goto *inst_goto) {
         /**
         *  push original instruction
         * */

        this->new_insts.push_back(inst_goto);
    }
    
    void SpillerVisitor::visit(Instruction_dec *dec) {
        std::vector<Item **> placeToReplace;
        bool varUsed = Has_Used_varToSpill(&dec->op, this->varToSpill, placeToReplace);
        
        /**
         *  dec instruction read and write the variable at the same time 
         *      if varUsed = true
         * */
        bool HasRead = varUsed;
        bool HasWrite = varUsed;

        this->spill_Inst(
            dec,
            HasRead,
            HasWrite,
            placeToReplace
        );

    }

    void SpillerVisitor::visit(Instruction_inc *inc) {
        std::vector<Item **> placeToReplace;
        bool var_used = Has_Used_varToSpill(&inc->op, this->varToSpill, placeToReplace);

        /**
         *  inc instruction read and write the variable at the same time 
         *  if var_used = true
         * */
        bool HasRead = var_used;
        bool HasWrite = var_used;

        this->spill_Inst(
            inc,
            HasRead,
            HasRead,
            placeToReplace
        );
    }
    void SpillerVisitor::visit(Instruction_cjump *cjump) {
        /**
         * cjump cannot write
         *  can only read if condition includes varToSpill
         */
        std::vector<Item **> placeToReplace;
        bool var_used = Has_Used_varToSpill(&cjump->condition, this->varToSpill, placeToReplace);
                
        this->spill_Inst(
            cjump,
            var_used,
            false,
            placeToReplace
        );
    }

    Spiller::Spiller(
            Function * F,
            ItemVariable * varToSpill,
            ItemVariable * prefix
    ){
        this->F = F;
        this->varToSpill = varToSpill;
        this->prefix = prefix;

        this->spill_visitor = new SpillerVisitor(
            varToSpill,
            prefix,
            F
        );
    }

    void Spiller::spill_variables()
    {
        

        if (!IN_MAP(this->F->varName2ptr, this->varToSpill->to_string())) {
            return;
        }


        this->F->locals += 1;
        
        for (Instruction *inst : this->F->instructions) {
            inst->accept(*this->spill_visitor);
        }

        this->F->instructions = this->spill_visitor->new_insts;

        // delete this->varToSpill;
        /**
         *  Copy new set of instructions to 
         * */
    }

    std::vector<ItemVariable *> Spiller::get_var_replacement() {
        return this->spill_visitor->var_replacements;
    }


    void Spiller::output_spilled_function()
    {   
        std::cout << '(';
        std::cout << this->F->name << '\n';

        std::cout << '\t' <<  this->F->arguments << ' ' << this->F->locals << '\n';

        for (Instruction * inst : this->F->instructions) {
            std::cout << '\t' <<  inst->to_string();
        }

        std::cout << ')';
        std::cout << '\n';

    }

    void run_Spill(Program &p) {
        for (Function * f : p.functions){
            Spiller sp(
                f,
                p.varToSpill,
                p.prefix
            );
            
            sp.spill_variables();
            sp.output_spilled_function();
        }
    }
} // namespace L2