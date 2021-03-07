// #include "code_generator.h"

// // #ifdef CODE_GEN_DEBUG
// // #define DEBUG_OUT (std::cerr << "DEBUG-Code-Generator: ") // or any other ostream
// // #else
// // #define DEBUG_OUT 0 && std::cerr
// // #endif

// namespace LA {

//     /**
//      *  arg0, arg1, arg2
//      * */
//     void output_args_helper(std::ofstream & out, std::vector<Item *> & args) {
//         for (int32_t i = 0; i < args.size(); i++) {
//             if (i > 0) {
//                 out << ", ";
//             }
//             assert(args[i]->itemtype == ItemType::item_variable);
//             ItemVariable * varArg = (ItemVariable *) args[i];
//             out << varArg->typeSig->to_string();
//             out << " ";
//             out << varArg->to_string(); 
//         }
//     }

    
//     void generateCode(Program & p) {
//         LA::isOutputIR = 1;

//         std::ofstream out =  std::ofstream();
//         out.open("prog.IR");
        
//         for (Function * F : p.functions) {
//             out << "define ";
//             out << F->retType->to_string();
//             out << " ";
//             out << F->name->to_string();
            
//             out << "(";
//             output_args_helper(out, F->arg_list);
//             out << ") ";
            
//             out << "{\n";
//             for (Instruction * instruction : F->insts) {
//                 out << "\t" << instruction->to_string();
//             }

//             out << "}\n";
//         }

//         LA::isOutputIR = 0;
//     }
// }

