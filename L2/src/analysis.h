#pragma once

#include <unordered_map>
#include <unordered_set>
#include <map>
#include <set>
#include <vector>
#include "L2.h"
#include "utils.h"

namespace L2
{
    void run_liveAnalysis(Program &p);
    void run_Interference(Program &p);

    // typedef std::unordered_map<Instruction *, std::unordered_set<Item *>> LiveSet;

    class ItemOutputVisitor : public ItemVisitor
    {
        void visit(ItemRegister *) override;
        void visit(ItemConstant *) override;
        void visit(ItemLabel *) override;
        void visit(ItemMemoryAccess *) override;
        void visit(ItemAop *) override;
        void visit(ItemCmp *) override;
        void visit(ItemVariable *) override;
        void visit(ItemStackArg *) override;
    };

    typedef std::map<Instruction *, std::set<Item *>> LiveSet;

    class LivenessVisitor : public InstVisitor
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
        
        LiveSet GEN;
        LiveSet KILL;
        // std::unordered_map<Instruction *, std::unordered_set<Item *>> GEN;
        // std::unordered_map<Instruction *, std::unordered_set<Item *>> KILL;
        /*
                ItemID -> registers/variables
                 registers/variables -> ItemID
        */
    };

    class SuccessorVisitor : public InstVisitor
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
    private:
        Function *F;

        LiveSet IN;
        LiveSet OUT;

        LivenessVisitor live_visitor;
        ItemOutputVisitor item_output_visitor;

        SuccessorVisitor succVisitor;
    };
    
    class InterferenceGraph {
        public:
            InterferenceGraph();
            
            InterferenceGraph(
                std::set<Item *> & nodes,
                std::map<Item *, std::set<Item *>> & edges
            );

            bool add_edge(Item *v1, Item *v2);
            bool remove_edge();
            bool remove_node(Item * toRemove);

            /**
             *  populate the stats map
             *      where @edgeCnt2nodes will be populated as a map from number of edges 
             *          to a set of nodes that have such number of edges
             * */
            void get_nodes_stats(std::map<int32_t, std::set<Item * >> & edgeCnt2nodes);

            std::set<Item *> & get_neighbors(Item * node);
        private:
            std::set<Item *> nodes;
            std::map<Item *, std::set<Item *>> edges;
    };
    

    class FunctionInterferenceAnalyzer {
    public:    
        FunctionInterferenceAnalyzer(
            Function * F,
            LiveSet & KILL,
            LiveSet & IN,
            LiveSet & OUT
        );

        void build_Inteference_graph();
        void output_Inteference();

        InterferenceGraph getIntGraph();
        
        private:
            
            Function * F;

            /**
             *  Input from FunctionAnalyzer
             * */
            // LiveSet GEN;
            LiveSet KILL;
            LiveSet IN;
            LiveSet OUT;

            ItemOutputVisitor item_output_visitor;

            /**
             *  Inteference Graph data structure
             * */
            std::set<Item *> nodes;
            std::map<Item *, std::set<Item *>> edges;

            /**
             *  connect everything in varsA with everything in VarsB
             * */
            void connect_two_sets(std::set<Item *> & varsA, std::set<Item *> & varsB);

            /**
             *  fully connect everything in vars with every other variables.
             * */
            void full_connect(std::set<Item *> & vars);

            /**
             *  Add connections within GP registers
             * */
            void add_GPRegisters_edges();

            /**
             *  Connect two variable if they are in the same IN/OUT set
             * */
            void add_INOUT_edges();

            /**
             *  Connect every vars KILL[i] to OUT[i]
             * */
            void add_KILLOUT_edges();

            /**
             * Connect rcx with offset of shift
             */
            void add_shift_edges();
            
    };
} // namespace L2