#pragma once

#include "LA.h"
#include "utils.h"
#include "new_label_var.h"

namespace LA
{
    void encode_program(Program &p);

    /**
     * Encodes and decodes and initializes variables 
     */
    class InstructionEncodingVisitor : public InstVisitor
    {
    public:
        std::vector<Instruction *> instsProcessed;
       

        void visit(Instruction_label *) override;

        /*  *
            *  Terminator of Basic Block
            * */
        void visit(Instruction_ret *) override;
        void visit(Instruction_ret_var *) override;
        void visit(Instruction_branch *) override;
        void visit(Instruction_branch_cond *) override;

        /*  *
            *  Normal instruction
            * */
        void visit(Instruction_declare *) override;
        void visit(Instruction_call *) override;
        void visit(Instruction_assignment *) override;


        /**
         *  sub visit function for assignment
         * */   
            void visitVarFromLength(
                Item * dst,
                ItemLength * len
            );

            void visitVarFromOPEncodeDest(
                Item * dst,
                ItemOp * OP
            );

            void visitVarFromOPDecodeOprd(
                Item * dst,
                ItemOp * OP
            );

            void visitVarFromArrAccess(
                Item * dst,
                ItemArrAccess * arrAccess
            );

            void visitArrAccessFromAll(
                ItemArrAccess * newArr,
                Item * src
            );

        
    private:
        
    };


    /**
     *  generate instruction to decode @v
     *  push such instruction into the insts list
     *  return the new variable after decoded
     *  */
    ItemVariable * encodeVarItem(
        std::vector<Instruction *> & instsProcessed,
        Item * v
    );
    
    ItemVariable * decodeVarItem(
        std::vector<Instruction *> & instsProcessed,
        Item * v
    );

    /**
     *  Do not generate new var
     *  append operations like
     *          v <- v << 1
     *          v <- v + 1
     * */
    void  encodeVarItemInPlace(
        std::vector<Instruction *> & instsProcessed,
        Item * v
    );

    void  decodeVarItemInPlace(
        std::vector<Instruction *> & instsProcessed,
        Item * v
    );
}