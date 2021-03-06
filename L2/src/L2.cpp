#include <L2.h>

// #define PRINT_FUNC_DEBUG 1
namespace L2 {
    ItemRegister::ItemRegister(Register_type rType){
        this->itemtype = item_registers;
        this->rType = rType;
    }

    ItemConstant::ItemConstant() {
        this->itemtype = item_constant;
        this->constVal = 0;
    }
    
    ItemConstant::ItemConstant(int64_t constVal) {
        this->itemtype = item_constant;
        this->constVal = constVal;
    }

    ItemLabel::ItemLabel(std::string str) {
        this->itemtype = item_labels;
        this->labelName = str;
    }

    ItemVariable::ItemVariable(std::string str) {
        this->itemtype = item_variable;
        this->name = str;
    }

    ItemMemoryAccess::ItemMemoryAccess() {
        this->itemtype = item_memory;
        this->offset = NULL;
        this->reg = NULL;
    }

    ItemMemoryAccess::ItemMemoryAccess(Item * reg, Item * offset) {
        this->itemtype = item_memory;
        this->offset = offset;
        this->reg = reg;
    }

    ItemCmp::ItemCmp() {
        this->itemtype = item_cmp;
        this->op1 = NULL;
        this->op2 = NULL;
        this->cmptype = eq;
    }

    ItemCmp::ItemCmp(Item *op1, Item *op2, CmpType cmpType) {
        this->itemtype = item_cmp;
        this->op1 = op1;
        this->op2 = op2;
        this->cmptype = cmpType;
    }

    ItemStackArg::ItemStackArg() {
        this->itemtype = item_stack_arg;
        this->offset = NULL;
    }

    ItemStackArg::ItemStackArg(Item *offset) {
        this->itemtype = item_stack_arg;
        this->offset = offset;
    }
    

    std::string ItemRegister::to_string(){
        switch(this->rType) {
            case Register_type::rdi : 
                return "rdi";

            case Register_type::rax : 
                return "rax";

            case Register_type::rsi : 
                return "rsi";
            
            case Register_type::rdx : 
                return "rdx";
            
            case Register_type::rcx : 
                return "rcx";

            case Register_type::r8 : 
                return "r8";

            case Register_type::r9 : 
                return "r9";
            
            case Register_type::rbx : 
                return "rbx";
            
            case Register_type::rbp : 
                return "rbp";
                break;
            
            case Register_type::r10 : 
                return "r10";
            
            case Register_type::r11 : 
                return "r11";
            
            case Register_type::r12 : 
                return "r12";
                break;
            
            case Register_type::r13 : 
                return "r13";
            
            case Register_type::r14 : 
                return "r14";
            
            case Register_type::r15 : 
                return "r15";

            case Register_type::rsp : 
                return "rsp";
            
            default :
                std::cerr << "wrong register type in ItemRegister::to_string\n";
                return "";
        }
    }

    std::string ItemConstant::to_string() {
        return std::to_string(this->constVal);

    }

    std::string ItemLabel::to_string() {
        return this->labelName;        
    }

    std::string ItemMemoryAccess::to_string() {
        std::string ret;

        std::string src = this->reg->to_string();
        std::string offset = this->offset->to_string();

        ret += "mem ";
        ret += src;
        ret += " ";
        ret += offset;
        
        return ret ;
    }

    std::string ItemAop::to_string() {
        // switch (this->aopType)
        // {
        // case AopType::plus_eq :
        //     return "+=";
        
        // case AopType::minus_eq :
        //     return "-=";

        // case AopType::times_eq :
        //     return "*=";

        // case AopType::bitand_eq :
        //     return "&=";

        // default:
        //     std::cerr << "wrong aop type in ItemAop::to_string\n";
        //         return "";
        // } 
        return "";
    }

    std::string ItemCmp::to_string() {
        std::string cmp_str;
        std::string op1;
        std::string op2;
        
        op1 = this->op1->to_string();
        op2 = this->op2->to_string();

        switch (this->cmptype)
        {
        case CmpType::eq :
            cmp_str = "=";
            break;
        
        case CmpType::less :
            cmp_str = "<";
            break;
        
        case CmpType::leq :
            cmp_str = "<=";
            break;

        default:
            std::cerr << "wrong cmp type in ItemCmp::to_string\n";
            return "";
        }

        return op1 + " " + cmp_str + " " + op2;
    }

    std::string ItemVariable::to_string() {
        return this->name;
    }

    std::string ItemStackArg::to_string() {
        std::string offset = this->offset->to_string();
        return  "stack-arg " + offset;
    }


    void ItemRegister::accept(ItemVisitor & visitor){
        visitor.visit(this);
    }

    void ItemConstant::accept(ItemVisitor & visitor){
        visitor.visit(this);
    }

    void ItemLabel::accept(ItemVisitor & visitor){
        visitor.visit(this);
    }

    void ItemMemoryAccess::accept(ItemVisitor & visitor){
        visitor.visit(this);
    }

    void ItemAop::accept(ItemVisitor & visitor){
        visitor.visit(this);
    }

    void ItemCmp::accept(ItemVisitor & visitor){
        visitor.visit(this);
    }

    void ItemVariable::accept(ItemVisitor & visitor){
        visitor.visit(this);
    }

    void ItemStackArg::accept(ItemVisitor & visitor){
        visitor.visit(this);
    }
    
    ItemRegister * ItemRegister::copy(){
        /**
         *  Global return itself 
         * */
        return this;
    }

    ItemConstant * ItemConstant::copy(){
        return new ItemConstant(this->constVal);
    }

    ItemLabel * ItemLabel::copy(){

        return new ItemLabel(this->labelName);
    }

    ItemMemoryAccess * ItemMemoryAccess::copy(){

        return new ItemMemoryAccess(this->reg->copy(), this->offset->copy());
    }

    ItemAop * ItemAop::copy(){
        ItemAop * aop = new ItemAop();
        aop->itemtype = item_aop;
        aop->aopType = this->aopType;
        return aop;
    }
    
    ItemCmp * ItemCmp::copy(){

        return new ItemCmp(
            this->op1->copy(),
            this->op2->copy(),
            this->cmptype
        );
    }

    ItemVariable * ItemVariable::copy(){

        // return new ItemVariable(
        //     this->name
        // );
        return this;
    }

    ItemStackArg * ItemStackArg::copy(){

        return new ItemStackArg(
            this->offset->copy()
        );
    }



    ItemRegister reg_rdi(Register_type::rdi);
    ItemRegister reg_rax(Register_type::rax);
    ItemRegister reg_rsi(Register_type::rsi);
    ItemRegister reg_rdx(Register_type::rdx);
    ItemRegister reg_rcx(Register_type::rcx);
    ItemRegister reg_r8(Register_type::r8);
    ItemRegister reg_r9(Register_type::r9);
    ItemRegister reg_rbx(Register_type::rbx);
    ItemRegister reg_rbp(Register_type::rbp);
    ItemRegister reg_r10(Register_type::r10);
    ItemRegister reg_r11(Register_type::r11);
    ItemRegister reg_r12(Register_type::r12);
    ItemRegister reg_r13(Register_type::r13);
    ItemRegister reg_r14(Register_type::r14);
    ItemRegister reg_r15(Register_type::r15);
    ItemRegister reg_rsp(Register_type::rsp);

    ItemRegister *caller_saved[L2::CALLER_NUM] = {
        &reg_r10,
        &reg_r11,
        &reg_r8,
        &reg_r9,
        &reg_rax,
        &reg_rcx,
        &reg_rdi,
        &reg_rdx,
        &reg_rsi
    };

    ItemRegister *callee_saved[L2::CALLEE_NUM] = {
        &reg_r12,
        &reg_r13,
        &reg_r14,
        &reg_r15,
        &reg_rbp,
        &reg_rbx
    };

    ItemRegister *arg_regs[L2::ARG_NUM] = {
        &reg_rdi,
        &reg_rsi,
        &reg_rdx,
        &reg_rcx,
        &reg_r8,
        &reg_r9
    };

    ItemRegister *GP_regs[L2::GP_NUM] = {
        &reg_rdi,
        &reg_rax,
        &reg_rsi,
        &reg_rdx,
        &reg_rcx,
        &reg_r8,
        &reg_r9,
        &reg_rbx,
        &reg_rbp,
        &reg_r10,
        &reg_r11,
        &reg_r12,
        &reg_r13,
        &reg_r14,
        &reg_r15
    };

    ItemLabel print_label(L2::print_str);
    ItemLabel input_label(L2::input_str);
    ItemLabel allocate_label(L2::allocate_str);
    ItemLabel tensor_label(L2::tensor_str);

    std::string Instruction_ret::to_string() {        
        return "return\n";
    }
    
    std::string Instruction_label::to_string() {
        
        return this->item_label->to_string() + '\n';
    }
    
    std::string Instruction_call_runtime::to_string() {
        std::string ret = "";
        
        std::string op1_str = this->runtime_callee->to_string();
        std::string op2_str = this->num_args->to_string();
        
        ret += "call ";
        ret +=  op1_str;
        ret += " ";
        ret += op2_str;
        ret += '\n';

        return ret;
    }
    
    std::string Instruction_call_user::to_string() {
        std::string ret = "";
        
        std::string callee = this->callee->to_string();
        std::string num_args = this->num_args->to_string();

        ret += "call ";
        ret += callee;
        ret += " ";
        ret += num_args;
        ret += '\n';

        return ret;

    }
    
    std::string Instruction_aop::to_string() {
        std::string aop_op = "";
        switch (this->aopType)
        {
        case AopType::plus_eq :
            aop_op = "+=";
            break;
        
        case AopType::minus_eq :
            aop_op = "-=";
            break;
    
        case AopType::times_eq :
            aop_op = "*=";
            break;

        case AopType::bitand_eq :
            aop_op = "&=";
            break;

        default:
            std::cerr << "wrong aop type in Instruction_aop::to_string\n";
            std::cerr << "aop = " << this->aopType << '\n';
            return "";
        } 

        std::string op1_str = this->op1->to_string();
        std::string op2_str = this->op2->to_string();

        std::string ret = "";
        ret += op1_str;
        ret += " ";
        ret += aop_op;
        ret += " ";
        ret += op2_str;
        ret += "\n";

        return ret;
    }
    
    
    std::string Instruction_assignment::to_string() {
        
        std::string dst = this->dst->to_string();
        std::string src = this->src->to_string();

        std::string ret = "";
        ret += dst;
        ret += " <- ";
        ret += src;
        ret += "\n";

        return ret;
        
    }
    
    
    std::string Instruction_sop::to_string() {
        std::string dirStr;
        switch (this->direction)
        {
        case ShiftType::left :
            dirStr = " <<= ";
            break;
        
        case ShiftType::right :
            dirStr = " >>= ";
            break;

        default:
            std::cerr << "wrong sop type in Instruction_sop::to_string\n";
            return "";
        }

        std::string target = this->target->to_string();
        std::string offset = this->offset->to_string();

        std::string ret;

        ret += target;
        ret += dirStr;
        ret += offset;
        ret += "\n";
        
        return ret;
    }

    std::string Instruction_lea::to_string() {
        std::string ret;
        
        std::string dest_str = this->dst->to_string();
        std::string addr = this->addr->to_string();
        std::string multr = this->multr->to_string();
        std::string const_multr = this->const_multr->to_string();

        ret += dest_str;
        ret += " @ ";
        ret += addr;
        ret += " ";
        ret += multr;
        ret += " ";
        ret += const_multr;
        ret += "\n";
        
        return  ret;

    }

    std::string Instruction_goto::to_string() {
        std::string ret;
        
        std::string label_str = this->gotoLabel->to_string();
        
        ret +=  "goto ";
        ret += label_str;
        ret += '\n';
        
        return ret;
    }

    std::string Instruction_dec::to_string() {
        std::string ret;
        
        std::string op_str = this->op->to_string();
        
        ret += op_str;
        ret += "--\n";
        
        return ret;
    }

    std::string Instruction_inc::to_string() {
        std::string ret;
        
        std::string op_str = this->op->to_string();
        
        ret += op_str;
        ret += "++\n";
        
        return ret;
    }
    
    std::string Instruction_cjump::to_string() {
        std::string ret;
        
        std::string cond_str = this->condition->to_string();
        std::string dest_str = this->dst->to_string();

        ret += "cjump ";
        ret += cond_str;
        ret += " ";
        ret += dest_str;
        ret += "\n";
        
        return ret ;
    }
    


    void Instruction_ret::accept(InstVisitor & visitor) {
        visitor.visit(this);
    }

    void Instruction_label::accept(InstVisitor &visitor) {
        visitor.visit(this);
    }

    void Instruction_call_runtime::accept(InstVisitor &visitor) {
        visitor.visit(this);
    }

    void Instruction_call_user::accept(InstVisitor &visitor) {
        visitor.visit(this);
    }

    void Instruction_aop::accept(InstVisitor &visitor) {
        visitor.visit(this);
    }

    void Instruction_assignment::accept(InstVisitor &visitor) {
        visitor.visit(this);
    }

    void Instruction_sop::accept(InstVisitor &visitor) {
        visitor.visit(this);
    }

    void Instruction_lea::accept(InstVisitor &visitor) {
        visitor.visit(this);
    }
    void Instruction_goto::accept(InstVisitor &visitor) {
        visitor.visit(this);
    }

    void Instruction_dec::accept(InstVisitor &visitor) {
        visitor.visit(this);
    }

    void Instruction_inc::accept(InstVisitor &visitor) {
        visitor.visit(this);
    }

    void Instruction_cjump::accept(InstVisitor &visitor) {
        visitor.visit(this);
    }

    Instruction_ret * Instruction_ret::copy() {
        Instruction_ret * ret = new Instruction_ret();
        ret->type = InstType::inst_ret;
        return ret;
    }

    Instruction_label * Instruction_label::copy() {
        Instruction_label * inst_label = new Instruction_label();
        
        inst_label->item_label = this->item_label->copy();
        inst_label->type = InstType::inst_label;
        return inst_label;
    }

    Instruction_call_runtime * Instruction_call_runtime::copy() {
        Instruction_call_runtime * call_run = new Instruction_call_runtime();
        
        call_run->runtime_callee = this->runtime_callee->copy();
        call_run->num_args = this->num_args->copy();
        call_run->type = InstType::inst_call;
        call_run->isRuntimeCall = true;

        return call_run;
    }

    Instruction_call_user * Instruction_call_user::copy() {
        Instruction_call_user * call_user = new Instruction_call_user();
        
        call_user->callee = this->callee->copy();
        call_user->num_args = this->num_args->copy();
        call_user->type = InstType::inst_call;
        call_user->isRuntimeCall = false;

        return call_user;
    }

    Instruction_aop * Instruction_aop::copy() {
        Instruction_aop * aop = new Instruction_aop();
        
        aop->op1 = this->op1->copy();
        aop->op2 = this->op2->copy();
        aop->aopType = this->aopType;
        aop->type = InstType::inst_aop;

        return aop;
    }

    Instruction_assignment * Instruction_assignment::copy() {
        Instruction_assignment * assign = new Instruction_assignment();
        
        assign->src = this->src->copy();
        assign->dst = this->dst->copy();
        assign->type = InstType::inst_assign;

        return assign;
    }

    Instruction_sop * Instruction_sop::copy() {
        Instruction_sop * sop = new Instruction_sop();
        
        sop->target = this->target->copy();
        sop->offset = this->offset->copy();
        sop->direction = this->direction;
        sop->type = InstType::inst_sop;

        return sop;
    }

    Instruction_lea * Instruction_lea::copy() {
        Instruction_lea * lea = new Instruction_lea();

        lea->dst = this->dst->copy();
        lea->addr = this->addr->copy();
        lea->multr = this->multr->copy();
        lea->const_multr = this->const_multr->copy();
        lea->type = InstType::inst_lea;

        return lea;
    }

    Instruction_goto * Instruction_goto::copy() {
        Instruction_goto * gt = new Instruction_goto();

        gt->gotoLabel = this->gotoLabel->copy();
        gt->type = InstType::inst_goto;

        return gt;
    }

    Instruction_dec * Instruction_dec::copy() {
        Instruction_dec * dec = new Instruction_dec();

        dec->op = this->op->copy();
        dec->type = InstType::inst_dec;
        
        return dec;
    }

    Instruction_inc * Instruction_inc::copy() {
        Instruction_inc * inc = new Instruction_inc();

        inc->op = this->op->copy();
        inc->type = InstType::inst_inc;

        return inc;
    }


    Instruction_cjump * Instruction_cjump::copy() {
        Instruction_cjump *new_cjump = new Instruction_cjump();
        new_cjump->dst = this->dst->copy();
        new_cjump->condition = this->condition->copy();
        new_cjump->type = InstType::inst_cjump;

        return new_cjump;
    }

    Function::Function() {
        this->name = "";
        this->arguments = 0;
        this->locals = 0;
        this->instructions = std::vector<Instruction *>();
        this->labelName2ptr = std::unordered_map<std::string, Item *>();
        this->varName2ptr = std::unordered_map<std::string, Item *>();
    }

    void Function::print() {
#ifdef PRINT_FUNC_DEBUG

        std::cout << "(";
        std::cout << this->name << '\n';
        std::cout << this->arguments << ' ' ;
        std::cout << this->locals;
        std::cout << '\n';

        for (Instruction * inst : this->instructions) {
            std::cout << inst->to_string();
        }

        std::cout << ")\n";
#endif
    }

    Function * Function::copy() {
        Function * cp = new Function();
        cp->name = this->name;
        cp->arguments = this->arguments;
        cp->locals = this->locals;
        
        /**
         *  copy instructions
         * */
        cp->instructions.clear();
        for (Instruction * inst : this->instructions) {
            cp->instructions.push_back(inst->copy());
        }
        

        cp->varName2ptr = this->varName2ptr;

        cp->labelName2ptr = this->labelName2ptr;

        return cp;
    }
}