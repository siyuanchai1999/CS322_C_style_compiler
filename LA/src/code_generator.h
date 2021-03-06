#pragma once

#include <fstream>
#include <string>
#include <iostream>


// #include "trace.h"
#include "config.h"
#include "utils.h"
#include "LA.h"

// // #include <L3.h>

namespace LA {

    void generateCode(Program & p);

//     class InstL3GenVisitor : public InstVisitor {
//         public:
//             void visit(Instruction_label *)         override;
            
//             /**
//             *  Terminator of Basic Block
//             * */
//             void visit(Instruction_ret *)           override;
//             void visit(Instruction_ret_var *)       override;
//             void visit(Instruction_branch *)        override;
//             void visit(Instruction_branch_cond *)   override;
            
//             /**
//              *  Normal instruction
//              * */
//             void visit(Instruction_declare *)       override;
//             void visit(Instruction_call *)          override;
//             void visit(Instruction_assignment *)    override;

//             InstL3GenVisitor();
//             InstL3GenVisitor(std::ofstream * outputFile, std::string & newVarPrefix);

//             void clean_new_vars();
//         private:
//             ItemVariable * get_new_var();

            

//             std::ofstream *out;
//             std::string newVarPrefix;
//             int32_t newVarIdx;

//             std::vector<ItemVariable *> newvars;

//             void output_inst_tab(std::string & inst);

//             void output_encode(Item * v);

//             /**
//              *  return the addr to load/store
//              * */
//             Item * output_addr_loadStore(
//                 /* wo kun le. hao le jiao wo */
//                 ItemArrAccess * arrAcc
//             );

//             void output_varFromArrAccess(
//                 Item * dst,
//                 ItemArrAccess * arrAccess
//             );

//             void output_ArrAccessFromAll (
//                 ItemArrAccess * newArr,
//                 Item * dst
//             );

//             void output_newArrayInst(
//                 Item * dst,
//                 ItemNewArray * newArr
//             );

//             void output_newTupleInst(
//                 Item * dst,
//                 ItemNewTuple * newTuple
//             );

//             void output_AssignCallInst(
//                 Item * dst,
//                 ItemCall * call
//             );

//             void output_AssignLengthInst(
//                 Item * dst,
//                 ItemLength * len
//             );

//             void output_operatorInst(
//                 Item * dst,
//                 Item * op1,
//                 OpType op,
//                 Item * op2
//             );

//             void output_AssignInst(
//                 Item * dst,
//                 Item * src
//             );
            
//             void output_LoadInst(
//                 Item *dst,
//                 Item *addr
//             );

//             void output_StoreInst(
//                 Item *dst,
//                 Item *addr
//             );

            
            
            
//     };


//     class generateCodeForTraces {
//         public:
//             void generateL3code(std::vector<Trace *> & traces);

//             generateCodeForTraces(std::ofstream * outputFile, std::string & newVarPrefix);

//         private:
//             InstL3GenVisitor L3InstGen;
//             std::ofstream *out;
//     }; 
}
