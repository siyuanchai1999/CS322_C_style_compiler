#include "code_generator.h"
#include <fstream>
// #include <assert.h>

// #define CODE_GEN_DEBUG 1

// #ifdef CODE_GEN_DEBUG
// #define DEBUG_OUT (std::cerr << "DEBUG-Code-Generator: ") // or any other ostream
// #else
// #define DEBUG_OUT 0 && std::cerr
// #endif

// #define QUADSIZE 8
// #define REG_ARGS_NUM 6
// #define MAX(a, b) ((a) > (b) ? (a) : (b))
// #define MIN(a, b) ((a) < (b) ? (a) : (b))

namespace L3 {
    

    void generateCode(
        Program & p,
        std::vector<std::vector<InstSelectForest * >> & codeGenerator
    ) {
        std::ofstream out =  std::ofstream();
        out.open("prog.L2");
        
        out << "(" << p.mainF->name->to_string() << "\n";
        
        for (int16_t i = 0; i < p.functions.size(); i++) {
            Function * F = p.functions[i];

            out << "(";
            out << F->name->to_string();
            out << " ";
            out << F->arg_list.size();
            out << "\n";

            /**
             *  loading args
             * */
            for (int32_t i = 0; i < F->arg_list.size(); i++) {
                out << '\t';
                out << F->arg_list[i]->to_string();
                out << " <- ";
                
                if (i < L3::ARG_NUM) {
                    out << L3::arg_regs[i]->to_string();
                }
                else 
                {
                    int32_t offset = (F->arg_list.size() - i - 1) * 8;
                    out << "stack-arg " << std::to_string(offset);  
                }
                out << "\n";
            }


            for (InstSelectForest * forest : codeGenerator[i]) 
            {
                std::vector<std::string> insts_str;
                forest->generateCode(insts_str);

                for (int16_t j = 0; j < insts_str.size(); j++) {
                    out << '\t' << insts_str[j];
                }
            }

            out << ")\n\n";

        }

        out << ")\n";

        out.close();

    }
}

