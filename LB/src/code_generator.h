#pragma once

#include <fstream>
#include <string>
#include <iostream>


// #include "trace.h"
#include "config.h"
#include "utils.h"
#include "LB.h"
#include "trans_while.h"
// // #include <L3.h>

namespace LB {

    void generateCode(Program & p);

    class InstLBGenVisitor  : public InstVisitor {
        public:
            void visit(Instruction_label *)         override;
            
            void visit(Instruction_ret *)           override;
            void visit(Instruction_ret_var *)       override;
            void visit(Instruction_goto *)          override;

            void visit(Instruction_if *)            override;
            void visit(Instruction_while *)         override;
            void visit(Instruction_continue *)      override;
            void visit(Instruction_break *)         override;

            void visit(Instruction_declare *)       override;
            void visit(Instruction_call *)          override;
            void visit(Instruction_assignment *)    override;

            void visit(Instruction_scope *)         override;

            InstLBGenVisitor(
                std::ofstream *out,
                std::map<Instruction_while *, ItemLabel *> * condlb,
                std::map<Instruction *, Instruction_while *> * inst2loop
            );
        private:
            std::ofstream *out;

            std::map<Instruction_while *, ItemLabel *> * condlb;
            std::map<Instruction *, Instruction_while *> * inst2loop;

            int32_t tabTimes; 

            void tabIn();
             
    };



}
