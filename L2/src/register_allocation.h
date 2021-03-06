#pragma once

#include <stack>
#include <unordered_map>
#include "analysis.h"
#include "spiller.h"

namespace L2 {
    void run_register_allocation(Program &p);

    const int32_t COLOR_NUM = 15;

    enum Color {
        rdi_color, 
        rsi_color, 
        rdx_color, 
        rcx_color,    
        r8_color, 
        r9_color, 
        rax_color, 
        r10_color,
        r11_color,
        r12_color,
        r13_color,
        r14_color, 
        r15_color, 
        rbp_color, 
        rbx_color
    };


    extern std::unordered_map<Item *, Color> reg2color;

    extern std::unordered_map<Color, Item *> color2reg;

    class NodeSelector {
        public:
        /**
         *  Constructor of node selector
         *      will copy intGraph to its own member
         *      so pass by reference but no change to intGraph
         * */
            NodeSelector(
                InterferenceGraph & intGraph, 
                int32_t color_num
            );

            void populate_stack(std::stack<Item *> & nodeStack);
                    
        private:

            /**
             *  local copy of interference graph
             * */
            InterferenceGraph intGraph;
            int32_t color_num;

            /**
             *  • Remove the node with the most edges
                  that’s smaller than then number of colors (15 in L1)
                • After all nodes with <= 15 edges are removed,
                  remove the remaining ones starting from the one
                  with the highest number of edges
             * */
            Item * select_next_node();    
    };


    class ColorSelector {
        public:
            /**
             *  ColorSelector only reads the inteference graph
             *      keep pointer type to save performance and memory
             * */
            ColorSelector(
                InterferenceGraph * intGraph, 
                std::stack<Item *> *nodeStack
            );
            /**
             *  Try to assign colors for all variables
             *      if failed return false; otherwise true
             *      @varsToSpill will contain the variables that cannot be colored
             *      @item2color will contain mapping from variable to its color
             * */
            bool assignColorForAll(
                std::unordered_map<Item *, Color> & item2color,
                std::set<Item *> & varsToSpill
            );
        
        private:
            std::stack<Item *> * nodeStack;
            InterferenceGraph * intGraph;
            // std::unordered_map<Item *, Color> item2color;
            

            Color sorted_color[L2::COLOR_NUM] = {
                /**
                 *  Caller Saved
                 * */
                
                r10_color,
                r11_color,
                r8_color,
                r9_color,
                rax_color,
                rcx_color,
                rdi_color,
                rdx_color,
                rsi_color,

                /**
                 * Callee Saved
                 */
                rbx_color,
                rbp_color,
                r12_color,
                r13_color,
                r14_color,
                r15_color
            };

            void pre_color_registers(std::unordered_map<Item *, Color> & item2color);
            bool select_color(Item * toColor, std::unordered_map<Item *, Color> & item2color);
    };

    class SpillVarSelector {
        public:
            SpillVarSelector(
                std::set<Item *> * NonColorItems,
                std::set<Item *> * prevSpillReplace
            );

            /**
             *  spill all right now
             *      except those created by previous spills
             * */
            void selectVarsToSpill(std::vector<Item *> &varsToSpill);

        private:
            std::set<Item *> * NonColorItems;
            std::set<Item *> * prevSpillReplace;
    };  

    class VarColorVisitor : public InstVisitor
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

        VarColorVisitor(
            std::unordered_map<Item *, Color> & item2color,
            Function * F
        );

    private:
        Function * F;
        std::unordered_map<Item *, Color> item2color;

        bool contains_varToColor(
            Item ** itemAddr,
            std::vector<Item **> & placeToReplace
        );
    };


    class RegisterAllocator {
        public:
            RegisterAllocator(Function * F, Function ** F_addr);

            void allcoate();

        private:
            Function * F;
            Function ** F_addr;
            // VarColorVisitor * var_color_visitor;
            
            void color_variables(Function * F, std::unordered_map<Item *, Color> & item2color);
            bool analysisAndColoring(Function * , std::set<Item *> & NonColorItems);
    };

}