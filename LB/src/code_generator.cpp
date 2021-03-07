#include "code_generator.h"

#ifdef CODE_GEN_DEBUG
#define DEBUG_OUT (std::cerr << "DEBUG-Code-Generator: ") // or any other ostream
#else
#define DEBUG_OUT 0 && std::cerr
#endif

namespace LB {
    void InstLBGenVisitor::tabIn() {
        std::string tabs(tabTimes * 4, ' ');
        *this->out << tabs;
    }
    
    void InstLBGenVisitor::visit(Instruction_label *label) {
        // this->tabIn();
        *this->out << label->to_string();
    }

    void InstLBGenVisitor::visit(Instruction_ret *ret) {
        // this->tabIn();
        *this->out << ret->to_string();
    }
    void InstLBGenVisitor::visit(Instruction_ret_var *ret_var) {
        // this->tabIn();
        *this->out << ret_var->to_string();
    }
    void InstLBGenVisitor::visit(Instruction_goto *inst_goto) {
        // this->tabIn();
        *this->out << "br ";
        *this->out << inst_goto->dst->to_string();
        *this->out << "\n";
    }

    void InstLBGenVisitor::visit(Instruction_if *inst_if) {
        /**
         *  if (v1 = p1) :true :false
         * */

        /**
         *  int64 condvar
         * */
        ItemVariable * condvar = LB::GENLV->get_new_var(VarType::int64);
        std::vector<Item *> toDeclare {condvar};
        Instruction_declare dec(
            &LB::int64Sig,      /* type singature */
            toDeclare,          /* variables */
            NULL                /* dummy parent */
        );
        dec.accept(*this);

        /**
         *  condvar <- v1 = p1
         * */
        Instruction_assignment assign (
            inst_if->condition,      /* src */
            condvar,                /*  destion */
            NULL                    /* dummy parent */
        );
        assign.accept(*this);

        /**
         *  br newV :true :false
         * */
        // this->tabIn();
        *this->out << "br ";
        *this->out << condvar->to_string();
        *this->out << " ";
        *this->out << inst_if->dst1->to_string();
        *this->out << " ";
        *this->out << inst_if->dst2->to_string();
        *this->out << "\n";
    }
    
    void InstLBGenVisitor::visit(Instruction_while * instWhile) {
        /**
         *  while (v1 = p1) :true :false
         * */
        
        /**
         *  :condlabel
         * */
        ItemLabel * lb = (*this->condlb)[instWhile];
        Instruction_label instlb(
            lb,                 /* label */
            NULL                /* parent */
        );
        instlb.accept(*this);

        /**
         *  int64 condvar
         * */
        ItemVariable * condvar = LB::GENLV->get_new_var(VarType::int64);
        std::vector<Item *> toDeclare {condvar};
        Instruction_declare dec(
            &LB::int64Sig,      /* type singature */
            toDeclare,          /* variables */
            NULL                /* dummy parent */
        );
        dec.accept(*this);

        /**
         *  condvar <- v1 = p1
         * */
        Instruction_assignment assign (
            instWhile->condition,      /* src */
            condvar,                /*  destion */
            NULL                    /* dummy parent */
        );
        assign.accept(*this);

        /**
         *  br newV :true :false
         * */
        // this->tabIn();
        *this->out << "br ";
        *this->out << condvar->to_string();
        *this->out << " ";
        *this->out << instWhile->dst1->to_string();
        *this->out << " ";
        *this->out << instWhile->dst2->to_string();
        *this->out << "\n";
    }

    void InstLBGenVisitor::visit(Instruction_continue *cont) {
        Instruction_while * whileInst = (*this->inst2loop)[cont];
        ItemLabel * lb = (*this->condlb)[whileInst]; 

        /**
         *  br :before_while_comp
         * */
        // this->tabIn();
        *this->out << "br ";
        *this->out << lb->to_string();
        *this->out << "\n";
    }

    void InstLBGenVisitor::visit(Instruction_break *brk) {
        Instruction_while * whileInst = (*this->inst2loop)[brk];
        Item * endlabel = whileInst->dst2;
        /**
         *  br :while_exit
         * */
        // this->tabIn();
        *this->out << "br ";
        *this->out << endlabel->to_string();
        *this->out << "\n";
    }

    void InstLBGenVisitor::visit(Instruction_declare *declare) {
        for (Item * varToDeclare: declare->vars){
            assert(varToDeclare->itemtype == ItemType::item_variable);
            ItemVariable *newVar = (ItemVariable *) varToDeclare;

            // this->tabIn();
            *this->out << newVar->typeSig->to_string() << " ";
            *this->out << newVar->to_string() << "\n";
        }
    }

    void InstLBGenVisitor::visit(Instruction_call *call) {
        // this->tabIn();
        *this->out << call->to_string();
    }
    void InstLBGenVisitor::visit(Instruction_assignment *assignment) {
        // this->tabIn();
        *this->out << assignment->to_string();
    }

    void InstLBGenVisitor::visit(Instruction_scope * scope) {
        // this->tabIn();
        *this->out << "\n";
        // this->tabTimes++;

        for (Instruction * inst: scope->insts) {
            inst->accept(*this);            
        }

        // this->tabTimes--;
        // this->tabIn();
        *this->out << "\n";
    }



    InstLBGenVisitor::InstLBGenVisitor(
        std::ofstream *out,
        std::map<Instruction_while *, ItemLabel *> * condlb,
        std::map<Instruction *, Instruction_while *> * inst2loop
    ) {
       this->out = out;
       this->condlb = condlb;
       this->inst2loop = inst2loop;
       this->tabTimes = 0;
    }


    /**
     *  arg0, arg1, arg2
     * */
    void output_args_helper(std::ofstream & out, std::vector<Item *> & args) {
        for (int32_t i = 0; i < args.size(); i++) {
            if (i > 0) {
                out << ", ";
            }
            assert(args[i]->itemtype == ItemType::item_variable);
            ItemVariable * varArg = (ItemVariable *) args[i];
            out << varArg->typeSig->to_string();
            out << " ";
            out << varArg->to_string(); 
        }
    }

    
    void generateCode(Program & p) {

        std::ofstream out =  std::ofstream();
        out.open("prog.a");
        
        

        for (Function * F : p.functions) {

            
            std::map<Instruction_while *, ItemLabel *> condlb = get_cond_labels(F);
            std::map<Instruction *, Instruction_while *> inst2loop = get_inst2loop(F);
            
            InstLBGenVisitor LB_gen = InstLBGenVisitor(
                &out,
                &condlb,
                &inst2loop  
            );
            
            out << F->retType->to_string();
            out << " ";
            out << F->name->to_string();
            
            out << "(";
            output_args_helper(out, F->arg_list);
            out << ") ";
            
            out << "{\n";
            F->scope->accept(LB_gen);
            out << "}\n";
        }

    }
}

