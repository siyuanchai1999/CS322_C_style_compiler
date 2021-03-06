#pragma once

#include <L2.h>
#include <fstream>
#include <string>
#include <iostream>


namespace L2{

    void generate_code(Program & p);

    class L2ToL1_GeneratorVisitor : public InstVisitor
    {
        public:
            void visit(Instruction_ret *) override;
            void visit(Instruction_label *) override;
            void visit(Instruction_call_runtime *) override;
            void visit(Instruction_call_user *) override;
            void visit(Instruction_aop *) override;
            void visit(Instruction_assignment *) override;
            void visit(Instruction_sop *) override;
            void visit(Instruction_lea *) override;
            void visit(Instruction_goto *) override;
            void visit(Instruction_dec *) override;
            void visit(Instruction_inc *) override;
            void visit(Instruction_cjump *) override;

            L2ToL1_GeneratorVisitor();
            L2ToL1_GeneratorVisitor(std::ofstream * outputFile);

            void set_numlocals(int32_t numlocals);
        private:
            std::ofstream *out;
            int32_t numlocals;

    };

    class CodeGenerator_L2ToL1 {
        public:
            // CodeGenerator_L2ToL1 (std::string outFilename, Program * p);
            CodeGenerator_L2ToL1(std::ofstream * out, Program * p);

            void generate();
            void close();
        private:
            std::ofstream *out;
            L2ToL1_GeneratorVisitor instGenerator;
            Program * p;
    };

}