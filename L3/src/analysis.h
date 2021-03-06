#pragma once

#include <unordered_map>
#include <unordered_set>
#include <map>
#include <set>
#include <vector>
#include "L3.h"
#include "utils.h"

namespace L3
{
    void run_liveAnalysis(Program &p);

    // typedef std::unordered_map<Instruction *, std::unordered_set<Item *>> LiveSet 

    typedef std::map<Instruction *, std::set<Item *>> LiveSet;

    class LivenessVisitor : public InstVisitor
    {
    public:
        void visit(Instruction_ret *)           override ;
        void visit(Instruction_ret_var *)       override ;
        void visit(Instruction_label *)         override ;
        void visit(Instruction_call *)          override ;
        void visit(Instruction_assignment *)    override ;
        void visit(Instruction_branch *)        override ;

        LiveSet GEN;
        LiveSet KILL;
        /*
                ItemID -> registers/variables
                 registers/variables -> ItemID
        */
    };

    class SuccessorVisitor : public InstVisitor
    {
    public:
        void visit(Instruction_ret *)           override ;
        void visit(Instruction_ret_var *)       override ;
        void visit(Instruction_label *)         override ;
        void visit(Instruction_call *)          override ;
        void visit(Instruction_assignment *)    override ;
        void visit(Instruction_branch *)        override ;

        std::unordered_map<Instruction* , std::vector<Instruction*>> successor;
        void find_successors(Function *F);
        
    private:
        void associate_label(Function *F);
        std::unordered_map<Item *, Instruction*> label2Inst;
    };

    class FunctionLivenessAnalyzer
    {
    public:
        void calculate_INOUT();
        void calculate_GENKILL();

        void output_INOUT();
        void output_GENKILL();

        FunctionLivenessAnalyzer(Function *F);
        
        LiveSet & get_live_KILL();
        LiveSet & get_live_IN();
        LiveSet & get_live_OUT();

        std::set<Item *> get_used(Instruction *);
        std::set<Item *> get_defs(Instruction *);
        std::set<Item *> get_live_after(Instruction *);
        std::set<Item *> get_live_since(Instruction *);
    private:
        Function *F;

        LiveSet IN;
        LiveSet OUT;
        LiveSet GEN;
        LiveSet KILL;

        LivenessVisitor live_visitor;

        SuccessorVisitor succVisitor;
    };
    
} // namespace L2