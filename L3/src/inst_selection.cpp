
#include "inst_selection.h"

// #define INST_SELECT_DEBUG

#ifdef INST_SELECT_DEBUG
#define DEBUG_OUT (std::cerr << "DEBUG-Inst-Select: ") // or any other ostream
#else
#define DEBUG_OUT 0 && std::cerr
#endif

#define INST_SELECT_ERROR

#ifdef INST_SELECT_ERROR
#define DEBUG_ERR (std::cerr << "ERROR-Inst-Select: ") // or any other ostream
#else
#define DEBUG_ERR 0 && std::cerr
#endif



namespace L3{
    OperatorType AopType_to_opType(AopType aop) {
        switch (aop)
        {
        case plus:
            return OperatorType::op_plus;
            break;
        
        case minus:
            return OperatorType::op_minus;
            break;

        case times:
            return OperatorType::op_times;
            break;
            
        case bit_and:
            return OperatorType::op_bit_and;
            break;
        
        case shift_left:
            return OperatorType::op_shift_left;
            break;
            
        case shift_right:
            return OperatorType::op_shift_right;
            break;
        
        default:
            DEBUG_ERR << "wrong type in AopType_to_opType: " << aop << '\n';
            return OperatorType::op_others;
            break;
        }
    }

    OperatorType CmpType_to_opType(CmpType cmp) {
        switch (cmp)
        {
        case less:
            return OperatorType::op_less;
            break;
        
        case leq:
            return OperatorType::op_leq;
            break;

        case eq:
            return OperatorType::op_eq;
            break;
            
        case great:
            return OperatorType::op_great;
            break;
        
        case geq:
            return OperatorType::op_geq;
            break;
        
        default:
            DEBUG_ERR << "wrong type in CmpType_to_opType: " << cmp << '\n';
            return OperatorType::op_others;
            break;
        }
    }

    std::string OperatorType_toString(OperatorType optype) {
        switch (optype)
        {
        case noDef:
            return "noDef";
        
        case op_plus:
            return "+";
        
        case op_minus:
            return "-";

        case op_times:
            return "*";

        case op_bit_and:
            return "&";

        case op_shift_left:
            return "<<";

        case op_shift_right:
            return ">>";

        case op_less:
            return "<";

        case op_leq:
            return "<=";

        case op_eq:
            return "=";

        case op_great:
            return ">"; 

        case op_geq:
            return ">="; 

        case assign:
            return "<-"; 
        
        case load:
            return "load"; 

        case store:
            return "store"; 

        case ret:
            return "return"; 

        case br:
            return "br"; 

        case call:
            return "call"; 

        case label:
            return "label"; 

        default:
            return "";
            break;
        }
    }

    bool InstSelectNode::tiling(std::vector<Tile *> & tiles) {
        /**
         *  • Algorithm:
                • Start at root
                • Use “biggest” match (in # of nodes)
                • This is the munch
                • Use cost to break ties
                • Recursively apply maximal much
                at each subtree of this munch
         * */

        assert(!this->covered  == (this->coveredBy == NULL));
        if (this->covered) {

            return true;
        }


        int32_t maxMatchNodes = -1;
        int32_t minMatchCost = INT32_MAX;
        Tile * bestTile = NULL;
        std::vector<InstSelectNode *> next_nodes;

        for (Tile * t : tiles) {
            std::vector<InstSelectNode *> cur_next_nodes;

            if (t->match(this, cur_next_nodes)) {
                if (
                        t->nodenCnt > maxMatchNodes
                    || (t->nodenCnt == maxMatchNodes && t->cost < minMatchCost)
                ) {
                    maxMatchNodes = t->nodenCnt;
                    minMatchCost  = t->cost;
                    bestTile = t;
                    next_nodes = cur_next_nodes;
                }
            }
        }

        /**
         *  cannot be covered by any tiles. FATAL!
         * */
        assert(bestTile != NULL);

        this->nextNodes = next_nodes;

        for (InstSelectNode * nextN : next_nodes) {
            bool nextTiled = nextN->tiling(tiles);
            assert(nextTiled);
        }
        
        this->covered = true;
        this->coveredBy = bestTile;

        return bestTile != NULL;
    }

    void InstSelectNode::generateCode(std::vector<std::string> & insts_str) {

        for (InstSelectNode * next : this->nextNodes) {
            next->generateCode(insts_str);
        }

        this->coveredBy->generateL2Inst(this, insts_str);
    }

    InstSelectNodeOperator::InstSelectNodeOperator(OperatorType op) {
        this->isOperator = true;
        this->children = std::vector<InstSelectNode *>();
        this->covered = false;
        this->coveredBy = NULL;
        this->op = op;
    }

    std::string InstSelectNodeOperator::to_string() {
        return OperatorType_toString(this->op);
    }

    void InstSelectNodeOperator::AddChild(InstSelectNode *leaf) {
        this->children.push_back(leaf);
    }

    InstSelectNode * InstSelectNodeOperator::copy() {
        InstSelectNodeOperator * newOprt = new InstSelectNodeOperator(
            this->op
        );

        newOprt->isOperator = this->isOperator;
        newOprt->coveredBy = this->coveredBy;
        newOprt->covered = this->covered;


        /**
         *  NOTE: nextNodes init to empty
         * */

        newOprt->representative = this->representative;
        for (InstSelectNode * child : this->children) {
            newOprt->AddChild(
                child->copy()
            );
        }
        return newOprt;
    }

    InstSelectNode * InstSelectNodeOperand::copy() {
        InstSelectNodeOperand * newOprd = new InstSelectNodeOperand(
            this->data
        );

        newOprd->isOperator = this->isOperator;
        newOprd->coveredBy = this->coveredBy;
        newOprd->covered = this->covered;

        return newOprd;
    }

    InstSelectNodeOperand::InstSelectNodeOperand(Item * data){
        if (data  == NULL) {
            std::cerr << "null reference passed in InstSelectNodeOperand::InstSelectNodeOperand!\n";
        }

        if (!isBasicItem(data)) {
            DEBUG_ERR << " un expected operand nodetype = " << data->itemtype << '\n';
        }

        this->isOperator = false;
        this->children = std::vector<InstSelectNode *>();
        this->covered = false;
        this->coveredBy = NULL;
        this->data = data;
    }

    std::string InstSelectNodeOperand::to_string() {
        return this->data->to_string();
    }

    
    
    bool InstSelectTree::tiling(std::vector<Tile *> & tiles) {
        /**
         *  Max Munch
         * • Algorithm:
            • Start at root
            • Use “biggest” match (in # of nodes)
            • This is the munch
            • Use cost to break ties
            • Recursively apply maximal much
            at each subtree of this munch
         * */
        return this->head->tiling(tiles);
        
    }

    void InstSelectTree::generateCode(std::vector<std::string> & insts_str) {
        this->head->generateCode(insts_str);
    }


    void InstSelectTree::print() {
        // for (InstSelectNode * head : this->heads) {
            InstSelectNode * head = this->head;
            /**
             *  <node, level>
             * */
            std::queue<std::pair<InstSelectNode *, int32_t>> q; 
            q.push(std::make_pair(head, 0));
            int32_t curlevel = 0;
            std::string outBuffer = "";

            while (!q.empty()) {
                InstSelectNode * thisNode = q.front().first;
                int32_t thislevel = q.front().second;
                
                q.pop();

                if (thislevel > curlevel) {
                    DEBUG_OUT << outBuffer << "\n";
                    curlevel = thislevel;
                    outBuffer = "";
                }

                outBuffer +=  thisNode->to_string();
                outBuffer += "\t";

                for (InstSelectNode * child : thisNode->children) {
                    q.push(
                        std::make_pair(
                            child,
                            thislevel + 1
                        )
                    );
                }

            }

            
            DEBUG_OUT <<  outBuffer << "\n";
        // }
    }

    bool InstSelectTree::replace_use_node (
        Item * var,
        InstSelectNode * definition_node
    ) {
        return this->replace_use_node_helper(
            this->head,
            var,
            definition_node
        );
    }


    bool InstSelectTree::replace_use_node_helper (
        InstSelectNode * curNode,
        Item * var,
        InstSelectNode * definition_node
    ) {

        if (curNode->isOperator) {
            InstSelectNodeOperator * operatorNode = (InstSelectNodeOperator *) curNode;
            
            uint32_t i = 0;
            if (
                    operatorNode->op == OperatorType::assign 
                &&  !operatorNode->children[0]->isOperator
            ) {
                /**
                 *  assign operator
                 *      if children[0] is a operand
                 *          it must be defined rather than used
                 *          we skip it becuse we only look at usage
                 * */
                i = 1;
            }

            bool contained = false;

            for (; i < operatorNode->children.size(); i++) {
                InstSelectNode * nextNode = operatorNode->children[i];

                if (nextNode->isOperator) 
                {
                    /**
                     *  recurse on operator node
                     * */
                    bool childContained = this->replace_use_node_helper(
                        nextNode,
                        var,
                        definition_node
                    );
                    contained = contained || childContained;
                } 
                else 
                {   
                    /**
                     * found node to replace
                     * */
                    InstSelectNodeOperand * operandNode = (InstSelectNodeOperand * ) nextNode;
                    if (operandNode->data == var) {
                        contained = true;
                        operatorNode->children[i] = definition_node->copy();
                    }

                    
                
                }

            }

            return contained;
        } 
        else 
        {   
            std::cerr << "InstSelectNodeOperand shouldn't be called\n";
            assert(0);

            return false;
        }
    }

    bool InstSelectTree::get_nodeAddr_item(
        Item * var,
        std::vector<InstSelectNode **> & placeToChange
    ) {
        return this->get_nodeAddr_item_helper(&this->head, var, placeToChange);
    }

    bool InstSelectTree::get_nodeAddr_item_helper(
        InstSelectNode ** curNodeAddr,
        Item * var,
        std::vector<InstSelectNode **> & placeToChange
    ) {
        InstSelectNode * curNode = *curNodeAddr;
        if (curNode->isOperator) {
            InstSelectNodeOperator * operatorNode = (InstSelectNodeOperator *) curNode;

            bool contained = false;

            for (uint32_t i = 0; i < operatorNode->children.size(); i++) {
                bool childContained =  this->get_nodeAddr_item_helper(
                    &operatorNode->children[i],
                    var,
                    placeToChange
                );

                contained = contained || childContained;

            }

            return contained;
        } 
        else 
        {
            InstSelectNodeOperand * operandNode = (InstSelectNodeOperand *) curNode; 
            if (operandNode->data == var) {
                placeToChange.push_back(curNodeAddr);
                return true;
            }

            return false;
        }
    }
    

    // void PatternNodeOperator::AddChild(InstSelectNode *leaf) {
    //     this->children.push_back(leaf);
    // }

        

    InstSelectNode * Item2Nodes(Item * item){
        if (item  == NULL) {
            std::cerr << "null reference passed in Item2Nodes!\n";
        }

        switch (item->itemtype)
        {
            case ItemType::item_constant :
            {
                return new InstSelectNodeOperand (item);
            }


            case ItemType::item_labels :
            {
                return new InstSelectNodeOperand (item);
            }

            case ItemType::item_variable :
            {
                return new InstSelectNodeOperand (item);
            }
            
            case ItemType::item_aop :
            {
                ItemAop * aop = (ItemAop *) item;
                InstSelectNodeOperator * aopnode = new InstSelectNodeOperator(
                    AopType_to_opType(aop->aopType)
                );
                
                aopnode->AddChild(
                   Item2Nodes(aop->op1) 
                );

                aopnode->AddChild(
                   Item2Nodes(aop->op2) 
                );

                return aopnode;
                break;
            }
            
            case ItemType::item_cmp :
            {
                ItemCmp * cmp = (ItemCmp *) item;
                InstSelectNodeOperator * cmpnode = new InstSelectNodeOperator(
                    CmpType_to_opType(cmp->cmptype)
                );
                
                cmpnode->AddChild(
                   Item2Nodes(cmp->op1) 
                );

                cmpnode->AddChild(
                   Item2Nodes(cmp->op2) 
                );

                return cmpnode;
                
                break;
            }

            
            
            case ItemType::item_load :
            {
                ItemLoad * load = (ItemLoad *) item;
                InstSelectNodeOperator * loadnode = new InstSelectNodeOperator(
                    OperatorType::load
                );
                
                loadnode->AddChild(
                   Item2Nodes(load->varToLoad) 
                );

                return loadnode;
                break;
            }

            case ItemType::item_store :
            {
                ItemStore * store = (ItemStore *) item;
                InstSelectNodeOperator * storenode = new InstSelectNodeOperator(
                    OperatorType::store
                );
                
                storenode->AddChild(
                   Item2Nodes(store->dst) 
                );

                return storenode;
                
                break;
            }

            case ItemType::item_call :
            {
                ItemCall * call = (ItemCall *) item;
                InstSelectNodeOperator * callnode = new InstSelectNodeOperator(
                    OperatorType::call
                );
                
                callnode->AddChild(
                   Item2Nodes(call->callee) 
                );

                for (Item * arg : call->args) {
                    callnode->AddChild(
                        Item2Nodes(arg) 
                    );
                }


                return callnode;
                break;
            }

            default:
                DEBUG_ERR << "wrong type in Item2Nodes: " << item->itemtype << '\n';
                return NULL;
                break;
        }
    }
    

    int32_t Context::get_size() {
        return this->insts.size();
    }

    void Context::print () {
        for (Instruction * inst : this->insts) {
            DEBUG_OUT << inst->to_string();
        }

        DEBUG_OUT << "\n";
    }

    void Context::add_inst (Instruction * inst) {
        this->inst2idx[inst] = this->insts.size();

        this->insts.push_back(inst);
    }

    int32_t Context::get_inst_idx(Instruction * inst) {
        if (IN_SET(this->inst2idx, inst)) {
            return this->inst2idx[inst];
        } else {
            std::cerr << "Cannot find " << inst->to_string() << " in Context::get_inst_idx\n";
            return -1;
        }
    }

    void identify_contexts(Function * F, std::vector<Context *> & CTs) {
        CTs.clear();
        Context * c = new Context;
        CTs.push_back(c);
        
        for (Instruction * inst: F->instructions) {
            /**
             * TODO: think about label insts
             * */
            c->add_inst(inst);
            // if (inst->type != InstType::inst_label) {
            //     c->add_inst(inst);
            // }

            if (inst->type == InstType::inst_label ||
                inst->type == InstType::inst_branch  ||
                inst->type == InstType::inst_ret ||
                inst->type == InstType::inst_ret_var
            ) {

                /**
                 *  :label1
                 *  :label2
                 *      there will be empty contexts 
                 * */
                if (c->get_size() > 0) {
                    c = new Context;
                    CTs.push_back(c);
                }
                
            }
        }
        
        /**
         * delete empty blocks
        */
        if (CTs.back()->get_size() == 0) {
            delete CTs.back();
            CTs.pop_back();
        }
    }


    Inst2TreeVisitor::Inst2TreeVisitor() {
        this->live = NULL;
        this->tree = NULL;
    }

    Inst2TreeVisitor::Inst2TreeVisitor(FunctionLivenessAnalyzer * live) {
        this->live = live;
    }

    void Inst2TreeVisitor::init_tree_used_defs(Instruction * inst) {
        this->tree->used = this->live->get_used(inst);
        this->tree->defs = this->live->get_defs(inst);
    }


    void Inst2TreeVisitor::visit(Instruction_ret *ret) {
        this->tree =  new InstSelectTree;
        
        InstSelectNodeOperator *head = new InstSelectNodeOperator(
                                                OperatorType::ret
                                            );
        
        // this->tree->heads.push_back(head);
        this->tree->head = head;


        /**
         *  init tree->used, tree->defs with Instructions' defs/used 
         * */
        this->init_tree_used_defs(ret);

        /***
         *  assocaite tree with intrusction
         * */
        this->tree->insts.push_back(ret);
    }

    void Inst2TreeVisitor::visit(Instruction_ret_var *ret_with_var) {
        this->tree = new InstSelectTree;
        
        InstSelectNodeOperator * head = new InstSelectNodeOperator(
                                                OperatorType::ret
                                            );
        
        head->AddChild(
            Item2Nodes(ret_with_var->valueToReturn)
        );
        
        // this->tree->heads.push_back(head);
        this->tree->head = head;

        /**
         *  init tree->used, tree->defs with Instructions' defs/used 
         * */
        this->init_tree_used_defs(ret_with_var);

        /***
         *  assocaite tree with intrusction
         * */
        this->tree->insts.push_back(ret_with_var);

    }

    void Inst2TreeVisitor::visit(Instruction_label * label) {
        this->tree = new InstSelectTree;
        
        InstSelectNodeOperator *labelNode = new InstSelectNodeOperator(
                                                    OperatorType::label
                                                );
        labelNode->AddChild(
            Item2Nodes(label->item_label)
        );
        
        // this->tree->heads.push_back(labelNode);
        this->tree->head = labelNode;
        /**
         *  init tree->used, tree->defs with Instructions' defs/used 
         * */
        this->init_tree_used_defs(label);
        
        /***
         *  assocaite tree with intrusction
         * */
        this->tree->insts.push_back(label);
        
    }
    
    void Inst2TreeVisitor::visit(Instruction_call *call) {
        this->tree = new InstSelectTree;
        
        InstSelectNode * callnode = Item2Nodes(call->call_wrap);
        
        
        if (call->ret) {
            InstSelectNodeOperator *assignNode = new InstSelectNodeOperator(OperatorType::assign);
            
            assignNode->AddChild(
                Item2Nodes(call->ret)
            );

            assignNode->AddChild(
                callnode
            );
            
            // this->tree->heads.push_back(assignNode);
            this->tree->head = assignNode;
                         
        } else {

            // this->tree->heads.push_back(callnode);
            this->tree->head = callnode;
        }

        /**
         *  init tree->used, tree->defs with Instructions' defs/used 
         * */
        this->init_tree_used_defs(call);

        /***
         *  assocaite tree with intrusction
         * */
        this->tree->insts.push_back(call);

    }
    
    void Inst2TreeVisitor::visit(Instruction_assignment *assignment) {
        this->tree = new InstSelectTree;
        InstSelectNodeOperator *assignmentNode = new InstSelectNodeOperator(
                                                        OperatorType::assign
                                                    );
        
        InstSelectNode *srcNode = Item2Nodes(assignment->src);
        InstSelectNode *dstNode = Item2Nodes(assignment->dst);
        
        assignmentNode->AddChild(dstNode);
        assignmentNode->AddChild(srcNode);
        
        // this->tree->heads.push_back(assignmentNode);
        this->tree->head = assignmentNode;
        /**
         *  init tree->used, tree->defs with Instructions' defs/used 
         * */
        this->init_tree_used_defs(assignment);
        
        /***
         *  assocaite tree with intrusction
         * */
        this->tree->insts.push_back(assignment);
        
    }
    
    void Inst2TreeVisitor::visit(Instruction_branch *branch) {
        this->tree = new InstSelectTree;
        InstSelectNodeOperator *branchNode = new InstSelectNodeOperator(
                                                    OperatorType::br
                                                );
        
        if (branch->condition) {
            InstSelectNode *conditionNode = Item2Nodes(branch->condition);
            branchNode->AddChild(conditionNode);
        }

        InstSelectNode *dstNode = Item2Nodes(branch->dst);
        branchNode->AddChild(dstNode);
        
        // this->tree->heads.push_back(branchNode);
        this->tree->head = branchNode;
        /**
         *  init tree->used, tree->defs with Instructions' defs/used 
         * */
        this->init_tree_used_defs(branch);

         /***
         *  assocaite tree with intrusction
         * */
        this->tree->insts.push_back(branch);
    }


    InstSelectForest::InstSelectForest(Context * CT, FunctionLivenessAnalyzer * live) {
        this->CT = CT;

        this->inst2tree_visitor = Inst2TreeVisitor(live);

        for (Instruction * inst : CT->insts) {
            inst->accept(this->inst2tree_visitor);

            // this->inst2tree_visitor.tree->print();

            this->trees.push_back(this->inst2tree_visitor.tree);    
        }
    }


    bool InstSelectForest::one_time_merge(FunctionLivenessAnalyzer & analyzer) {
        

        /**
         *  tree_use starts from the behind
         *  tree_def is at the front
         * */
        for (int32_t i = this->trees.size() - 1 ; i >= 0  ; i--) 
        {   
            InstSelectTree * tree_use = this->trees[i];
            
            for (int32_t j = i - 1; j >= 0; j-- ) 
            { 
                InstSelectTree * tree_def = this->trees[j];

                std::vector<Item *> overlap_items = this->get_overlap(
                                                            tree_use, 
                                                            tree_def                    
                                                        );

                if (overlap_items.size() > 1) {
                        std::cerr << "WARNING! tree_use and tree_def has multiple overlap" << "\n";
                }

                for (Item * var : overlap_items) {
                    bool can_merge = this->can_merge_trees(
                        i,
                        j, 
                        var, 
                        analyzer
                    );

                    if (can_merge) {
                        this->do_merge_trees(tree_use, tree_def, var);

                        /**
                         *  remove the j th tree from this->trees
                         *  indexes will be changed, so directly return
                         *      can be optimized in the future
                         * */
                        this->trees.erase(this->trees.begin() + j);

                        delete tree_def;

                        return true;
                    }
                }
            }   
        }

        return false;
    }

    void InstSelectForest::merge_trees(FunctionLivenessAnalyzer & analyzer) {
        /**
         *  merge as much as possible
         * */
        
        
        bool hasMerged = false;


        /**
         *  newTree
         * */  
        do {
            hasMerged = this->one_time_merge(analyzer);

        } while (hasMerged); 
    }


    /**
     *  return Item * that is defined by tree_def and used by tree_use
     *  
     *  Ideally, one tree only defines one item, so overlap_items.size() <= 1 (expected)
     * */
    std::vector<Item *> InstSelectForest::get_overlap(
        InstSelectTree * tree_use,
        InstSelectTree * tree_def
    ) {
        std::set<Item *>  DefUseIntersect;
        
        set_intersect(
            tree_use->used,         /*srcA*/
            tree_def->defs,         /*srcB*/
            DefUseIntersect         /* srcA intersect src B*/
        );

        return std::vector<Item *>(
            DefUseIntersect.begin(),
            DefUseIntersect.end()
        );
        
    }


    bool InstSelectForest::can_merge_trees(
        int32_t  tree_use_idx,
        int32_t  tree_def_idx,
        Item * varOverlap,
        FunctionLivenessAnalyzer & analyzer
    ) {
        InstSelectTree * tree_use = this->trees[tree_use_idx];
        InstSelectTree * tree_def = this->trees[tree_def_idx];

        /**
         *  if tree_def defines varOverlap with load
         *      DONOT merge
         * */
        InstSelectNodeOperator * assign_oprt = (InstSelectNodeOperator *)tree_def->head;
        assert(assign_oprt->op == OperatorType::assign);
        
        if (assign_oprt->children[1]->isOperator) {
            InstSelectNodeOperator * childOprt = (InstSelectNodeOperator *) assign_oprt->children[1];
            if (childOprt->op == load) {
                return false;
            }
        }

        /**
         * A. %V is dead after the instruction attached to T1 or  (only one inst associate with )
         * %V is only used by T1 
         */
        
        /**
         * A. %V is dead after the last instruction attached to tree_use
         */

        /**
         *  Find last inst associated with tree_use
         * */
        int32_t latestIdx = -1; 
        Instruction * latestInst = NULL;
        for (Instruction * inst : tree_use->insts) {
            int32_t curIdx = this->CT->get_inst_idx(inst);
            
            if (curIdx > latestIdx) {
                latestInst = inst;
                latestIdx = curIdx;
            }
        }

        if (latestInst == NULL){
            std::cerr << "latestInst = NULL in InstSelectForest::can_merge_trees\n";
            assert(latestInst != NULL);
        }

        std::set<Item *> var_used_after = analyzer.get_live_after(latestInst);

        if (IN_SET(var_used_after, varOverlap)) {
            return false;
        }

        /**
         *  v should be only used by tree_use and tree_def 
         *      at least at this stage
         * */
        for (int32_t idx = 0; idx < this->trees.size(); idx++) {
            if (idx != tree_use_idx && idx != tree_def_idx) {
                InstSelectTree * t = this->trees[idx];
                if (IN_SET(t->used, varOverlap)) {
                    return false;
                }
            }
        }

        /**
         *  B. No other uses/defs of %V between tree_def and tree_use
         * */
        
        for (int32_t idx = tree_def_idx + 1; idx < tree_use_idx; idx++) 
        {
            InstSelectTree * t = this->trees[idx];
            if (IN_SET(t->used, varOverlap)) {
                /**
                 *  t has used varOverlap
                 * */
                return false;
            }

            if (IN_SET(t->defs, varOverlap)) {
                /**
                 *  t has defined varOverlap
                 * */
                return false;
            }
        }

        /**
         *  C. No definitions of variables used by tree_def between tree_use and tree_def
         * */
        
        for (int32_t idx = tree_def_idx + 1; idx < tree_use_idx; idx++) 
        {
            InstSelectTree * t = this->trees[idx];
            if (has_intersect(t->defs, tree_def->used)) {
                return false;
            }
        }

        return true;
        
    }
    
    void InstSelectForest::do_merge_trees(
        InstSelectTree * tree_use,
        InstSelectTree * tree_def,
        Item * varOverlap
    ) {
        /**
         *  tree_def
         *              <-
         *  varOverlap      definition
         * 
         *  tree_use
         *          x op
         *        /     \
         *  varOverlap?
         * */       

        assert(varOverlap->itemtype == ItemType::item_variable);

        /**
         *  tree_def head must have assign op type
         * */
        InstSelectNodeOperator * assign_node = (InstSelectNodeOperator *) tree_def->head;

        assert(assign_node->op == OperatorType::assign);
        assert(assign_node->children.size() == 2);
        
        InstSelectNodeOperand * definition_node = (InstSelectNodeOperand *) assign_node->children[1];

        

        /**
         *  find place to replace
         * */
        
        bool replaced = tree_use->replace_use_node(
            varOverlap, 
            definition_node
        );
        
        assert(replaced);

        /**
         *  update tree_use use/def
         *      tree_use no longer use varOverlap
         *          but will use tree_def->used
         * */
        tree_use->used.erase(varOverlap);
        tree_use->used.insert(
            tree_def->used.begin(),
            tree_def->used.end()
        );  

        /**
         *  expand tree_use
         * */

        tree_use->insts.insert(
            tree_use->insts.end(),
            tree_def->insts.begin(),
            tree_def->insts.end()
        );

              
    }

    bool InstSelectForest::tiling(std::vector<Tile *> & tiles) {
        for (auto tree: this->trees) {
            bool tiled = tree->tiling(tiles);

            assert(tiled);
        }

        return true;   
    }

    void InstSelectForest::print() {
        for (auto tree: this->trees) {
            tree->print();
            DEBUG_OUT << std::endl;
        }
    }

    void InstSelectForest::generateCode(std::vector<std::string> & insts_str) {
        
        for (auto tree: this->trees) {
            tree->generateCode(insts_str);
        }
    }




    void select_insts(Program & p, std::vector<std::vector<InstSelectForest * >> & codeGenerator) {
        
        codeGenerator = std::vector<std::vector<InstSelectForest * >>(
            p.functions.size(),
            std::vector<InstSelectForest * >()
        );

        std::vector<Tile *> L3ToL2_tiles;
        std::string prefix =  new_var_prefix(p);
        std::string FRet_prefix = new_fRetLabel_prefix(p);

        tile_init(p, L3ToL2_tiles, prefix, FRet_prefix);

        // for  (Function * F : p.functions) {
        for (int16_t i = 0; i < p.functions.size(); i++) {
            Function * F = p.functions[i];
            /**
             *  run liveness analysis
             * */
            FunctionLivenessAnalyzer live_analyzer(F);
            live_analyzer.calculate_GENKILL();
            live_analyzer.calculate_INOUT();

            // live_analyzer.output_GENKILL();
            // live_analyzer.output_INOUT();

            std::vector<Context *> CTs;
            identify_contexts(F, CTs);

            for(Context * c : CTs) {
                c->print();
                InstSelectForest * forest = new InstSelectForest(c, &live_analyzer);
                
                // DEBUG_OUT << "Before merge!\n";
                // forest->print();

                forest->merge_trees(live_analyzer);

                DEBUG_OUT << "After merge!\n";
                forest->print();

                forest->tiling(L3ToL2_tiles);

                DEBUG_OUT << "Done Tiling!\n";

                codeGenerator[i].push_back(forest);

            }

        }
    }

}