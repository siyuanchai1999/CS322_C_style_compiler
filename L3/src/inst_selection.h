#pragma once

#include "L3.h"
#include <queue>
#include "analysis.h"
#include <cassert>
#include "transformer.h"

namespace L3{
    
    struct InstSelectNode;
    struct InstSelectNodeOperator;
    struct InstSelectNodeOperand;
    struct InstSelectTree;

    struct PatternNode;
    struct PatternNodeOperator;
    struct PatternNodeOperand;
    struct PatternTree;

    struct Tile {
        int32_t nodenCnt;
        int32_t cost;
        std::string name;

        std::set<InstSelectNode *> matchedNodes;

        PatternTree *pattern;
        bool match(InstSelectNode *, std::vector<InstSelectNode *> &);

        virtual void generateL2Inst(InstSelectNode *, std::vector<std::string> &) = 0;
    
    };

    struct Context {
        std::vector<Instruction *> insts;
        std::unordered_map<Instruction *, int32_t> inst2idx;


        void add_inst(Instruction *);
        void print ();
        int32_t get_size();
        int32_t get_inst_idx(Instruction *);
    };

    void transform_label (Program & p);

    void tile_init(
        Program & p,
        std::vector<Tile *> & L3ToL2_tiles,
        std::string & prefix,
        std::string & FRet_prefix
    );
    
    enum OperatorType {
        noDef,
        op_plus, op_minus, op_times, op_bit_and, op_shift_left, op_shift_right,
        op_less, op_leq, op_eq, op_great, op_geq,
        assign,
        load, store,

        ret,
        br,
        call,
        label,

        op_others
    };

    std::string OperatorType_toString(OperatorType optype);

    struct genNode {
        // std::vector<genNode *> children;

    };


    struct InstSelectNode : genNode
    {
        std::vector<InstSelectNode *> children;
        bool isOperator;
        
        Tile * coveredBy;
        bool covered;

        /**
         *  pointer to subtree that has head covered by Tile @coveredBy
         *      these are the next nodes to visit during code generation
         *      populated during tiling
         * */
        std::vector<InstSelectNode *> nextNodes;


        virtual std::string to_string() = 0;
        virtual InstSelectNode * copy() = 0;

        bool tiling(std::vector<Tile *> & tiles);
        void generateCode(std::vector<std::string> &);
    };

    struct InstSelectNodeOperator : InstSelectNode
    {
        OperatorType op;
        
        /**
         *  representative var/const for subtree under this node
         *      used in codegeneration
         *      tile saves the representative here
         * */
        Item * representative;


        InstSelectNodeOperator(OperatorType op);
        void AddChild(InstSelectNode * leaf);

        std::string to_string();
        InstSelectNode * copy() override;

    };


    struct InstSelectNodeOperand : InstSelectNode
    {
        Item * data;    /*var or constant*/

        InstSelectNodeOperand(Item * data);

        InstSelectNode * copy() override;
        std::string to_string();
    };

    

    /**
    *   var <- load var
    *       <-
    *   var   load
     * */
    struct InstSelectTree
    {
        // std::vector <InstSelectNode *> heads;
        InstSelectNode * head;
        std::vector<Instruction *> insts;
        
        /**
         * set of items (vars) being defined ans used by current tree
         * */
        std::set<Item *> defs;
        std::set<Item *> used;

        /**
         *  take in collection of tiles
         *      populate 
         * */
        bool tiling(std::vector<Tile *> & tiles);

        void generateCode(std::vector<std::string> &);
        void print();
        

        bool replace_use_node (
            Item * var,
            InstSelectNode * definition_node
        );

        /**
         *  return all places that operand node that contains var
         * */
        bool get_nodeAddr_item(
            Item * var,
            std::vector<InstSelectNode **> & placeToChange
        );

        private:
            bool get_nodeAddr_item_helper(
                InstSelectNode ** curNodeAddr,
                Item * var,
                std::vector<InstSelectNode **> & placeToChange
            );

            bool replace_use_node_helper (
                InstSelectNode * curNode,
                Item * var,
                InstSelectNode * definition_node
            );
    };
    
    
    void identify_contexts(Function * F, std::vector<Context *> & CTs);

    class Inst2TreeVisitor : public InstVisitor
    {
    public:
    /**
     *  visit an instruction and 
     *      store the generated tree into this->tree
     * */
        void visit(Instruction_ret *)           override ;
        void visit(Instruction_ret_var *)       override ;
        void visit(Instruction_label *)         override ;
        void visit(Instruction_call *)          override ;
        void visit(Instruction_assignment *)    override ;
        void visit(Instruction_branch *)        override;

        /**
         *  output from visit
         * */
        InstSelectTree * tree;

        Inst2TreeVisitor();
        Inst2TreeVisitor(FunctionLivenessAnalyzer * live);
        
        private:
            FunctionLivenessAnalyzer * live;
            /**
             *  init tree->used, tree->defs with Instructions' defs/used 
             * */
            void init_tree_used_defs(Instruction *);
    };

    struct InstSelectForest {
        std::vector <InstSelectTree *> trees;
        Context * CT;

        Inst2TreeVisitor inst2tree_visitor;
        
        InstSelectForest(Context * CT, FunctionLivenessAnalyzer * live);

        void merge_trees(
            FunctionLivenessAnalyzer & analyzer
        );

        bool can_merge_trees(
            int32_t  tree_use_idx,
            int32_t  tree_def_idx,
            Item * varOverlap,
            FunctionLivenessAnalyzer & analyzer
        );

        void do_merge_trees(
            InstSelectTree * tree_use,
            InstSelectTree * tree_def,
            Item * varOverlap
        );

        bool tiling(std::vector<Tile *> & tiles);
        void generateCode(std::vector<std::string> & insts_str);
        void print();

        private:
            /**
             *  return Item * that is defined by tree_def and used by tree_use
             *  
             *  Ideally, one tree only defines one item, so overlap_items.size() <= 1 (expected)
             * */
            std::vector<Item *> get_overlap(
                InstSelectTree * tree_use,
                InstSelectTree * tree_def
            );

            bool one_time_merge(
                FunctionLivenessAnalyzer & analyzer
            );

    };


    void select_insts(
        Program & p,
        std::vector<std::vector<InstSelectForest * >> & codeGenerator
    );
}