#pragma once

#include "inst_selection.h"
#include "code_generator.h"

namespace L3 {

    const int16_t aopOpsNum = 6;
    const int16_t cmpOpsNum = 5;
    
    extern std::set<OperatorType> aopOps; 
    extern std::set<OperatorType> cmpOps;
    extern std::set<OperatorType> aopcmpOps;
    

    struct PatternNode: genNode
    {   
        std::vector<PatternNode *> children;
        bool isOperator;
        
        virtual bool match(InstSelectNode *, std::vector<InstSelectNode *> &) = 0;
    };
    
    struct PatternNodeOperator: PatternNode
    {
        std::set<OperatorType> possibleOps;

        /**
         *  only possible to be true if possibleOps contains call
         * */
        bool isRuntimeCall;

        PatternNodeOperator(OperatorType singleOp);
        PatternNodeOperator(std::set<OperatorType> & ops);
        PatternNodeOperator(OperatorType singleOp, bool isRuntimeCall);

        void AddChild(PatternNode * leaf);
        bool match(InstSelectNode *, std::vector<InstSelectNode *> &); 

    };


    struct PatternNodeOperand: PatternNode
    {   
        /**
         *  Check if Operands match:
         *      if the target node is also an operand: check for type match
         *      if the target is an operator:
         *          if target is aop, cmp, load, and call => match, bc they represent var
         *          if not => no match
         * */
        std::set<ItemType> possibleItemsTypes;

        PatternNodeOperand(ItemType singleType);
        PatternNodeOperand(std::set<ItemType> & types);

        bool match(InstSelectNode *, std::vector<InstSelectNode *> &); 
    };

    struct PatternTree
    {
        PatternNode * head;
        
        bool match(InstSelectNode *, std::vector<InstSelectNode *> &);

        void generateL2Inst(InstSelectNode *);
    };

    struct AopTile : Tile {
        /**
         *  aop        
         * op1 op2   
         *  =>>>
         *  %ret <- v1
         *  %ret += v2
         * */
        AopTile();
        void generateL2Inst(
            InstSelectNode *,
            std::vector<std::string> &
        ) override;
    };

    struct AssignToVarTile : Tile {
        /**
         *   <-        
         * op1 op2   
         *  =>>>
         *  %ret <- v1
         * */
        AssignToVarTile ();
        void generateL2Inst(
            InstSelectNode *,
            std::vector<std::string> &
        ) override;
    };

    struct AssignToStoreTile : Tile {
        /**
         *   <-        
         * store op2   
         * v 
         * =>>>
         *  %ret <- v1
         * */
        AssignToStoreTile ();
        void generateL2Inst(
            InstSelectNode *,
            std::vector<std::string> &
        ) override;
    };

    struct ReturnTile : Tile {
        /**
         * return 
         * */

        ReturnTile();
        void generateL2Inst(
            InstSelectNode *,
            std::vector<std::string> &
        ) override;
    };

    struct ReturnValueTile : Tile {
        /**
         * return val 
         * */
        ReturnValueTile();
        void generateL2Inst(
            InstSelectNode *,
            std::vector<std::string> &
        ) override;
    };

    struct CallTile : Tile {
        bool isRuntimeCall;
        int32_t num_args;

        CallTile(bool isRuntimeCall, int32_t num_args);
        void generateL2Inst(
            InstSelectNode *,
            std::vector<std::string> &
        ) override;
    };

    struct CmpTile : Tile {
        /**
         *  cmp        
         * op1 op2   
         *  =>>>
         *  %ret <- v1 cmp v2
         * */
        CmpTile();
        void generateL2Inst(
            InstSelectNode *,
            std::vector<std::string> &
        ) override;
    };  

    struct BrTile : Tile {

    };

    struct UncondBrTile : BrTile {
        /**
         *  br label  
         *  =>>>
         *  goto label
         *  
         * */
        UncondBrTile();


        void generateL2Inst(
            InstSelectNode *,
            std::vector<std::string> &
        ) override;
    };

    struct CondBrOnConstTile : BrTile {
        /**
         *  
         *  br N label
         *  ->
         *  goto label or do nothing
         *  
         * */
        CondBrOnConstTile();
        void generateL2Inst(
            InstSelectNode *,
            std::vector<std::string> &
        ) override;
    };

    struct CondBrOnCmpTile : BrTile {
        /**
         *  
         *  v <- v1 cmp v2
         *  br v label
         * =>
         *  cjump v1 cmp v2 label
         * 
         * */
        CondBrOnCmpTile();
        void generateL2Inst(
            InstSelectNode *,
            std::vector<std::string> &
        ) override;
    };

    struct CondBrOnVarTile : BrTile {
        /**
         *  br v label
         * =>
         *  cjump v == 1 label
         * */
        CondBrOnVarTile();
        void generateL2Inst(
            InstSelectNode *,
            std::vector<std::string> &
        ) override;
    };

    struct LabelTile : Tile {
        /**
         *  label
         *  :label
         * =>
         *  :label
         * */
        LabelTile();
        void generateL2Inst(
            InstSelectNode *,
            std::vector<std::string> &
        ) override;
    };
    
    struct LoadTile : Tile {
        /**
         *  load var
         *  
         *  load
         *  var
         * 
         * =>
         *  %newV <- mem var 0
         * */
        LoadTile();
        void generateL2Inst(
            InstSelectNode *,
            std::vector<std::string> &
        ) override;
    };


    // struct StoreTile : Tile {
    //     /**
    //      *  store var <- s
    //      *      <-
    //      *  store   s
    //      *  var 
    //      * =>
    //      *  mem var 0 <- s
    //      * */
    //     StoreTile();
    //     void generateL2Inst(
    //         InstSelectNode *,
    //         std::vector<std::string> &
    //     ) override;
    // };

}