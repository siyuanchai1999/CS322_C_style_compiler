
#include "tile.h"

namespace L3 {
    
    static int32_t new_var_cnt = 0;
    static std::string prefix;

    static int32_t FRet_cnt = 0;
    static std::string FRet_prefix;
    
    std::set<OperatorType> aopOps = 
    {
        op_plus,
        op_minus, 
        op_times, 
        op_bit_and, 
        op_shift_left, 
        op_shift_right
    };

    std::set<OperatorType> cmpOps = 
    {
        op_less,
        op_leq, 
        op_eq, 
        op_great, 
        op_geq
    };

    std::set<OperatorType> aopcmpOps = 
    {
        op_plus,
        op_minus, 
        op_times, 
        op_bit_and, 
        op_shift_left, 
        op_shift_right,
        op_less,
        op_leq, 
        op_eq, 
        op_great, 
        op_geq
    };    


    PatternNodeOperator::PatternNodeOperator(OperatorType singleOp, bool isRuntimeCall) {
        if (isRuntimeCall) {
            /**
             *  must be a call operator when isRuntimeCall = True 
             * */
            assert(singleOp == OperatorType::call);
        }

        this->isOperator = true;
        this->isRuntimeCall = isRuntimeCall; 

        this->possibleOps.insert(
            singleOp
        );
    }

    PatternNodeOperator::PatternNodeOperator(OperatorType singleOp) {
        this->isOperator = true;
        this->isRuntimeCall = false;   
        // this->children default init

        this->possibleOps.insert(
            singleOp
        );
    }

    PatternNodeOperator::PatternNodeOperator(std::set<OperatorType> & ops) {
        this->isOperator = true;
        this->isRuntimeCall = false;  
        // this->children default init

        this->possibleOps.insert(
            ops.begin(),
            ops.end()
        );
    }  

    void PatternNodeOperator::AddChild(PatternNode * leaf) {
        this->children.push_back(leaf);
    }

    bool PatternNodeOperator::match(InstSelectNode * instNode, std::vector<InstSelectNode *> & next_nodes) {
        
        
        /**
         *  type match: PatternNodeOperator must match with InstSelectNodeOperator 
         * */

        if (this->isOperator != instNode->isOperator) return false;
        
        /**
         *  operator match
         *      safe to cast
         * */

        InstSelectNodeOperator * instOperator = (InstSelectNodeOperator *) instNode;
        if (!IN_SET(this->possibleOps, instOperator->op)) {
            return false;
        }

        if (instOperator->op == OperatorType::call) {
            /**
             *  callee can be either a label (operand) or operator
             *      it can be a return value from other function
             *      call
             *   call   2
             *    myF
             * */
            bool nodeCallRuntime = false;
            if (!instNode->children[0]->isOperator){
                /**
                 *  is an operand, so its callee data defines if it's a runtime call
                 * */
                InstSelectNodeOperand * callee = (InstSelectNodeOperand *) instNode->children[0];
            
                nodeCallRuntime = isRuntimeLabel(callee->data);
            } else {
                /**
                 *  Is an operator must not be a runtime call
                 * */
                nodeCallRuntime = false;
            }

            if(this->isRuntimeCall != nodeCallRuntime) {
                /**
                 *      Tile is runtime call but node is not 
                 *  or
                 *      Tile is not runtime call but node is
                 * */
                return false;
            }

            
            
        } 
        // else  {
            
        // }
        if(this->children.size() != instNode->children.size()) {
            return false;
        }

        /**
         *  recursively check for children
         * 
         *  TODO:
         * */
        for (int16_t i = 0; i < this->children.size(); i++) {
            PatternNode * patternChild = this->children[i];
            InstSelectNode * instChild = instOperator->children[i];
            
            if (!patternChild->match(instChild, next_nodes)) {
                return false;
            }
        }

        return true;
        
    }

    PatternNodeOperand::PatternNodeOperand(ItemType singleType) {
        this->isOperator = false;
        
        // this->children default init

        this->possibleItemsTypes.insert(
            singleType
        );
    }

    PatternNodeOperand::PatternNodeOperand (std::set<ItemType> & types) {
        this->isOperator = false;
        
        // this->children default init

        this->possibleItemsTypes.insert(
            types.begin(),
            types.end()
        );
    }

    bool PatternNodeOperand::match(InstSelectNode *instNode, std::vector<InstSelectNode *> & next_nodes)
    {
        if (this->isOperator == instNode->isOperator)
        {
            /**
             *  operand matched with operand
             *      check type directly
             * */
            InstSelectNodeOperand *instOperand = (InstSelectNodeOperand *)instNode;
            bool operandTypeMatched = IN_SET(
                this->possibleItemsTypes,
                instOperand->data->itemtype
            );

            return operandTypeMatched;
        }
        else
        {
            InstSelectNodeOperator *instOperator = (InstSelectNodeOperator *)instNode;
            /**
             *  aop cmp operators, load operators and call operators return var
             * */
            bool canReturnVar =
                IN_SET(this->possibleItemsTypes, ItemType::item_variable) &&
                (IN_SET(L3::aopcmpOps, instOperator->op) || instOperator->op == OperatorType::load || instOperator->op == OperatorType::call);
            
            if (canReturnVar) {
                next_nodes.push_back(instOperator);
            }

            return canReturnVar;
        }
    }

    bool PatternTree::match(InstSelectNode * instNode, std::vector<InstSelectNode *> & next_nodes) {
        return this->head->match(instNode, next_nodes);
    }


    bool Tile::match(InstSelectNode * instNode, std::vector<InstSelectNode *> & next_nodes) {

        bool matched =  this->pattern->match(instNode, next_nodes);
        
        if (matched){
            this->matchedNodes.insert(instNode);
        }

        return matched;
    }


    
    AopTile::AopTile() {
        /**
         *  initialize pattern
         * */
        
        /**
         *  %ret <- v1
         *  %ret += v2 
         * */   
        this->cost = 2;
        this->nodenCnt = 3;
        this->name = "AopTile";
        this->matchedNodes = std::set<InstSelectNode *>();
        this->pattern = new PatternTree;

        PatternNodeOperator * head = new PatternNodeOperator(L3::aopOps);
        
        head->AddChild(
            new PatternNodeOperand(L3::varAndConst)
        );

        head->AddChild(
            new PatternNodeOperand(L3::varAndConst)
        );

        this->pattern->head = head;
    }   

    /**
     *  Should be used to generate any node that can be a var    
     *      if it's an operator, expect representative item has been put
     * 
     *      valid for operandNode and operatorNode on the edge of a tile
     * */
    std::string varNodeToString(InstSelectNode * instNode) {
        if (instNode->isOperator) {
            InstSelectNodeOperator * oprt = (InstSelectNodeOperator *) instNode;
            assert(oprt->representative != NULL);
            return oprt->representative->to_string();
        }
        else 
        {
            InstSelectNodeOperand * oprd = (InstSelectNodeOperand *) instNode;

            return oprd->data->to_string();
        }
    }


    void AopTile::generateL2Inst(
        InstSelectNode * instNode,
        std::vector<std::string> & insts_str
    ) {
        
        /**
         *  WTF if it's not matched???
         * */
        assert(IN_SET(this->matchedNodes, instNode));

        assert(instNode->isOperator);

        InstSelectNodeOperator * oprt = (InstSelectNodeOperator *) instNode;
        oprt->representative = new ItemVariable(
            L3::prefix + std::to_string(L3::new_var_cnt++)
        );

        /**
         *  aop        
         * op1 op2   
         *  =>>>
         *  %ret <- v1
         *  %ret += v2
         * */
        
        assert(oprt->children.size() == 2);

        std::string inst = "";
        inst += varNodeToString(oprt);
        inst += " ";
        inst += OperatorType_toString(OperatorType::assign);     /*  <- */
        inst += " ";
        inst += varNodeToString(oprt->children[0]);
        inst += "\n";

        insts_str.push_back(inst);
        inst.clear();

        inst += varNodeToString(oprt);
        inst += " ";
        inst += OperatorType_toString(oprt->op) + "=";     /*  + -> += */
        inst += " ";
        inst += varNodeToString(oprt->children[1]);
        inst += "\n";

        insts_str.push_back(inst);
    }
    
    AssignToVarTile::AssignToVarTile () {
        this->cost = 1;
        this->nodenCnt = 3;
        this->name = "AssignToVarTile";
        this->matchedNodes = std::set<InstSelectNode *>();
        this->pattern = new PatternTree;
        
        PatternNodeOperator * head = new PatternNodeOperator(OperatorType::assign);

        /**
         *  LHS: var
         * */
        head->AddChild(
            new PatternNodeOperand(ItemType::item_variable)
        );

        /**
         *  RHS: general var/label/const or anything that's compatible with var
         * */
        head->AddChild(
            new PatternNodeOperand(L3::basicTypes)
        );
        
        this->pattern->head = head;
    }

    void AssignToVarTile::generateL2Inst(
        InstSelectNode * instNode,
        std::vector<std::string> & insts_str
    ) {
         /**
         *  WTF if it's not matched???
         * */
        assert(IN_SET(this->matchedNodes, instNode));

        assert(instNode->isOperator);

        InstSelectNodeOperator * oprt = (InstSelectNodeOperator *) instNode;

        /**
         *   <-        
         * op1 op2   
         *  =>>>
         *  %ret <- v1
         * */
        
        assert(oprt->children.size() == 2);

        std::string inst = "";

        inst += varNodeToString(oprt->children[0]);
        inst += " ";
        inst += OperatorType_toString(OperatorType::assign);     /*  <- */
        inst += " ";
        inst += varNodeToString(oprt->children[1]);
        inst += "\n";

        insts_str.push_back(inst);
    }

    AssignToStoreTile::AssignToStoreTile () {
         /**
         *  store var <- s
         *      <-
         *  store   s
         *  var 
         * =>
         *  mem var 0 <- s
         * */

        this->cost = 1;
        this->nodenCnt = 4;
        this->name = "AssignToStoreTile";
        this->matchedNodes = std::set<InstSelectNode *>();
        this->pattern = new PatternTree;
        
        PatternNodeOperator * head = new PatternNodeOperator(OperatorType::assign);

        PatternNodeOperator * store = new PatternNodeOperator(OperatorType::store);
        store->AddChild(new PatternNodeOperand(ItemType::item_variable) );
       
        /**
         *  LHS: store var <- RHS
         * */
        head->AddChild(
            store
        );

        /**
         *  RHS: general var/label/const or anything that's compatible with var
         * */
        head->AddChild(
            new PatternNodeOperand(L3::basicTypes)
        );

        this->pattern->head = head;
    
    }

    void AssignToStoreTile::generateL2Inst(
        InstSelectNode * instNode,
        std::vector<std::string> & insts_str
    ) {
         /**
         *  WTF if it's not matched???
         * */
        assert(IN_SET(this->matchedNodes, instNode));

        assert(instNode->isOperator);

        InstSelectNodeOperator * oprt = (InstSelectNodeOperator *) instNode;

        /**
         *   <-        
         * store op2   
         * v 
         * =>>>
         *  %ret <- v1
         * */
        
        assert(oprt->children.size() == 2);

        /**
         *  must be store operator
         * */
        assert(oprt->children[0]->isOperator);
        InstSelectNodeOperator * store = (InstSelectNodeOperator *) oprt->children[0];

        assert(store->children.size() == 1);
        assert(store->op == OperatorType::store);
        
        int32_t offset = 0;
        
        std::string inst = "";

        inst += "mem";
        inst += " ";
        inst += varNodeToString(store->children[0]);            /* assign */
        inst += " ";
        inst += std::to_string(offset);                         
        inst += " ";
        inst += OperatorType_toString(OperatorType::assign);     /*  <- */
        inst += " ";
        inst += varNodeToString(oprt->children[1]);                 
        inst += "\n";

        insts_str.push_back(inst);
    }

    ReturnTile::ReturnTile() {

        this->cost = 1;
        this->nodenCnt = 1;
        this->name = "ReturnTile";
        this->matchedNodes = std::set<InstSelectNode *>();
        this->pattern = new PatternTree;

        PatternNodeOperator * head = new PatternNodeOperator(OperatorType::ret);
        this->pattern->head = head;

    }

    void ReturnTile::generateL2Inst(
        InstSelectNode * instNode,
        std::vector<std::string> & insts_str
    ) {
         /**
         *  WTF if it's not matched???
         * */
        assert(IN_SET(this->matchedNodes, instNode));

        assert(instNode->isOperator);
        InstSelectNodeOperator * oprt = (InstSelectNodeOperator *) instNode;
        

        std::string inst = "";


        inst += oprt->to_string();
        inst += "\n";
        
        insts_str.push_back(inst);

    }

    ReturnValueTile::ReturnValueTile() {
        this->cost = 2;
        this->nodenCnt = 2;
        this->name = "ReturnValueTile";
        this->matchedNodes = std::set<InstSelectNode *>();
        this->pattern = new PatternTree;

        PatternNodeOperator * head = new PatternNodeOperator(OperatorType::ret);

        
        head->AddChild(
            new PatternNodeOperand(L3::basicTypes)
        );
        
        this->pattern->head = head;

    }

    void ReturnValueTile::generateL2Inst(
        InstSelectNode * instNode,
        std::vector<std::string> & insts_str
    ) {
         /**
         *  WTF if it's not matched???
         * */
        assert(IN_SET(this->matchedNodes, instNode));

        assert(instNode->isOperator);
        InstSelectNodeOperator * oprt = (InstSelectNodeOperator *) instNode;
        

        assert(oprt->children.size() == 1);

        std::string inst = "";

        inst += "rax <- ";
        inst += varNodeToString(oprt->children[0]);
        inst += "\n";

        insts_str.push_back(inst);
        
        inst.clear();
        inst += oprt->to_string();
        inst += "\n";
        
        insts_str.push_back(inst);
    }

    CallTile::CallTile(bool isRuntimeCall, int32_t num_args) {
        this->isRuntimeCall = isRuntimeCall;
        this->num_args = num_args;
        this->name = "CallTile";
        this->name += "_" + std::to_string(isRuntimeCall);
        this->name += "_" + std::to_string(num_args);
        
        if (isRuntimeCall) {
            /**
                rdi <- 3
                call :print 1
             * */
            this->cost = num_args + 1;

            /**
             *  one for callee,
             *  one for call op node
             * */
            this->nodenCnt = num_args + 2;
        }
        else {
            /**
             *  mem rsp -8 <- :myF_ret
                rdi <- 3
                call :myF 1
                :myF_ret
             * */
            /**
             *  one for save return addr
             *  one for the call
             *  one for return label
             * */
            this->cost = num_args + 3;

            /**
             *  one for callee,
             *  one for call op node
             * */
            this->nodenCnt = num_args + 2;
        }
    
        this->matchedNodes = std::set<InstSelectNode *>();
        this->pattern = new PatternTree;

        PatternNodeOperator * head = new PatternNodeOperator(OperatorType::call, isRuntimeCall);

        /**
         *  callee node
         * */
        head->AddChild(
            new PatternNodeOperand(L3::varAndLabel)
        );
        
        for (int32_t i = 0; i < num_args; i++) {
            head->AddChild(
                new PatternNodeOperand(L3::basicTypes)
            );
        }

        this->pattern->head = head;
    }

    void CallTile::generateL2Inst(
        InstSelectNode * instNode,
        std::vector<std::string> & insts_str
    ) {

        assert(IN_SET(this->matchedNodes, instNode));
        assert(instNode->isOperator);
        
        InstSelectNodeOperator * oprt = (InstSelectNodeOperator *) instNode;
        assert(oprt->children.size() == (this->num_args + 1));

        /**
         *  call returns in rax
         * */
        oprt->representative = new ItemVariable(
            L3::prefix + std::to_string(L3::new_var_cnt++)
        );

        std::string inst = "";
        std::string new_ret_label = "";

        /**
         *  Technically speaking callee CAN be merged
         * */       
        InstSelectNode * callee = oprt->children[0];
        // assert(!callee->isOperator);
        
        std::string fname = callee->to_string();
        std::string fname_noColon = fname.substr(1, fname.length() - 1); 

        
        
        for (int32_t i = 0; i < this->num_args; i++) {
            inst.clear();

            if (i < L3::ARG_NUM) {
                inst += L3::arg_regs[i]->to_string();
            } else {
                /**
                 *  i = 6, 7th arg, offset = -16
                 * */
                int32_t offset = -8 + (i - L3::ARG_NUM + 1) * (-8);
                
                inst += "mem rsp " + std::to_string(offset);
            }

            inst += " <- ";
            inst += varNodeToString(oprt->children[i + 1]);
            inst += "\n";
            insts_str.push_back(inst);
        }

        if (!this->isRuntimeCall) {
            new_ret_label = FRet_prefix 
                        + "_" 
                        + fname_noColon
                        + "_"
                        + std::to_string(L3::FRet_cnt++);  

            /**
             *  Sample output:
             *      mem rsp -8 <- :new_ret_label
             * */

            inst.clear();
            inst += "mem rsp -8 <- " ;
            inst += new_ret_label;
            inst += "\n";
            insts_str.push_back(inst);
        }


        /**
         *  Sample output:
         *      call :myF 1
         * */
        inst.clear();
        inst += "call ";
        inst += varNodeToString(callee);
        inst += " ";
        inst += std::to_string(this->num_args);
        inst += "\n";
        insts_str.push_back(inst);

        
        if (!this->isRuntimeCall) {
             /**
             *  Sample output:
             *      :new_ret_label
             * */

            inst.clear();
            inst += new_ret_label;
            inst += "\n";
            insts_str.push_back(inst);
        }
        
        /**
         *  %newVar <- rax
         * */
        inst.clear();
        inst += varNodeToString(oprt);
        inst += " ";
        inst += OperatorType_toString(OperatorType::assign);
        inst += " ";
        inst += reg_rax.to_string();
        inst += "\n";
        insts_str.push_back(inst);


    }

    CmpTile::CmpTile() {
        /**
         *  initialize pattern
         * */
        
        /**
         *          cmp
         *      v1      v2
         * =>
         *  %newvar = v1 cmp v2
         * */   
        this->cost = 1;
        this->nodenCnt = 3;
        this->name = "CmpTile";
        this->matchedNodes = std::set<InstSelectNode *>();
        this->pattern = new PatternTree;

        PatternNodeOperator * head = new PatternNodeOperator(L3::cmpOps);
        
        head->AddChild(
            new PatternNodeOperand(L3::varAndConst)
        );

        head->AddChild(
            new PatternNodeOperand(L3::varAndConst)
        );

        this->pattern->head = head;
    }   

    void CmpTile::generateL2Inst(
        InstSelectNode * instNode,
        std::vector<std::string> & insts_str
    ) {
        
        /**
         *  WTF if it's not matched???
         * */
        assert(IN_SET(this->matchedNodes, instNode));
        assert(instNode->isOperator);
        InstSelectNodeOperator * oprt = (InstSelectNodeOperator *) instNode;

        oprt->representative = new ItemVariable(
            L3::prefix + std::to_string(L3::new_var_cnt++)
        );

        /**
         *  cmp        
         * op1 op2   
         *  =>>>
         *  %ret <- v1 cmp v2
         * */
        
        assert(oprt->children.size() == 2);

        std::string inst = "";
        /**
         * Flips >= and > because only < exists in L2
         * */
        if (oprt->op == op_geq)
        {
            inst += varNodeToString(oprt);
            inst += " ";
            inst += OperatorType_toString(OperatorType::assign); /*  <- */
            inst += " ";
            inst += varNodeToString(oprt->children[1]);
            inst += " ";
            inst += OperatorType_toString(OperatorType::op_leq);
            inst += " ";
            inst += varNodeToString(oprt->children[0]);
        }
        else if (oprt->op == op_great)
        {
            inst += varNodeToString(oprt);
            inst += " ";
            inst += OperatorType_toString(OperatorType::assign); /*  <- */
            inst += " ";
            inst += varNodeToString(oprt->children[1]);
            inst += " ";
            inst += OperatorType_toString(OperatorType::op_less);
            inst += " ";
            inst += varNodeToString(oprt->children[0]);
        }
        else
        {
            inst += varNodeToString(oprt);
            inst += " ";
            inst += OperatorType_toString(OperatorType::assign); /*  <- */
            inst += " ";
            inst += varNodeToString(oprt->children[0]);
            inst += " ";
            inst += OperatorType_toString(oprt->op);
            inst += " ";
            inst += varNodeToString(oprt->children[1]);
        }
        inst += "\n";
        insts_str.push_back(inst);
    }

    UncondBrTile::UncondBrTile () {
        /**
         *  br label  
         *  =>>>
         *  goto label
         *  
         * */
        this->cost = 1;
        this->nodenCnt = 2;
        this->matchedNodes = std::set<InstSelectNode *>();
        this->pattern = new PatternTree;
        this->name = "Unconditional jump";

        PatternNodeOperator * head = new PatternNodeOperator(OperatorType::br);
        
        head->AddChild(
            new PatternNodeOperand(ItemType::item_labels)
        );

        this->pattern->head = head;
    }

    void UncondBrTile::generateL2Inst(
        InstSelectNode * instNode,
        std::vector<std::string> & insts_str
    ) {
        assert(IN_SET(this->matchedNodes, instNode));
        assert(instNode->isOperator);
        InstSelectNodeOperator * oprt = (InstSelectNodeOperator *) instNode;

        assert(oprt->children.size() == 1);
        
        std::string inst = "";
        inst += "goto ";
        inst += oprt->children[0]->to_string();
        inst += "\n";

        insts_str.push_back(inst);
    }

    CondBrOnConstTile::CondBrOnConstTile() {
        /**
         * br N label
         * =>>
         * goto label OR nothing
         * */
        this->cost = 1;
        this->nodenCnt = 3;

        this->matchedNodes = std::set<InstSelectNode *> ();
        this->pattern = new PatternTree();
        this->name = "Branch conditioned on const";

        PatternNodeOperator *head = new PatternNodeOperator(OperatorType::br);
        
        head->AddChild(
            new PatternNodeOperand(ItemType::item_constant)
        );
        head->AddChild(
            new PatternNodeOperand(ItemType::item_labels)
        );
        
        this->pattern->head = head;
    }
    
    void CondBrOnConstTile::generateL2Inst(
        InstSelectNode * instNode,
        std::vector<std::string> & insts_str
    ){
        assert(IN_SET(this->matchedNodes, instNode));
        assert(instNode->isOperator);
        InstSelectNodeOperator * oprt = (InstSelectNodeOperator *) instNode;

        assert(oprt->children.size() == 2);

        InstSelectNodeOperand * Noprd = (InstSelectNodeOperand *) oprt->children[0];
        assert(Noprd->data->itemtype == ItemType::item_constant);

        ItemConstant * constData =  (ItemConstant *) Noprd->data;
        
        /**
         *  only goto's if 1
         * */
        if (constData->constVal == 1) {
            std::string inst = "";
            inst += "goto ";
            inst += oprt->children[1]->to_string();
            inst += "\n";
            insts_str.push_back(inst);
        }

    }

    CondBrOnCmpTile::CondBrOnCmpTile () {
        /**
         *  L3 node tree
         *          br
         *      cmp   label
         *     v1  v2
         * =>>
         *  cjump v1 cmp v2 label
         * 
         * */

        this->cost = 1;
        /**
         *  br, cmp, v1, v2, label
         * */
        this->nodenCnt = 5;

        this->matchedNodes = std::set<InstSelectNode *> ();
        this->pattern = new PatternTree();
        this->name = "Branch conditioned on Cmp";

        PatternNodeOperator *head = new PatternNodeOperator(OperatorType::br);
        
        PatternNodeOperator *cmp = new PatternNodeOperator(L3::cmpOps);
        
        cmp->AddChild(
            new PatternNodeOperand(L3::varAndConst)
        );
        cmp->AddChild(
            new PatternNodeOperand(L3::varAndConst)
        );

        head->AddChild(cmp);
        head->AddChild(
            new PatternNodeOperand(ItemType::item_labels)
        );
        
        this->pattern->head = head;
    }

    void CondBrOnCmpTile::generateL2Inst(
        InstSelectNode * instNode,
        std::vector<std::string> & insts_str
    ) {
        assert(IN_SET(this->matchedNodes, instNode));
        assert(instNode->isOperator);
        InstSelectNodeOperator * oprt = (InstSelectNodeOperator *) instNode;

        /**
         *  L3 node tree
         *          br
         *      cmp   label
         *     v1  v2
         * =>
         *  cjump v1 cmp v2 label
         * 
         * */
        assert(oprt->children.size() == 2);

        InstSelectNodeOperator * cmp = (InstSelectNodeOperator *) oprt->children[0];
        assert(cmp->children.size() == 2);
        std::string inst = "";
        inst += "cjump ";

        if (cmp->op == op_geq)
        {
            inst += varNodeToString(cmp->children[1]);
            inst += " ";
            inst += OperatorType_toString(OperatorType::op_leq);
            inst += " ";
            inst += varNodeToString(cmp->children[0]);
        }
        else if (cmp->op == op_great)
        {
            inst += varNodeToString(cmp->children[1]);
            inst += " ";
            inst += OperatorType_toString(OperatorType::op_less);
            inst += " ";
            inst += varNodeToString(cmp->children[0]);
        }
        else
        {
            inst += varNodeToString(cmp->children[0]);
            inst += " ";
            inst += OperatorType_toString(cmp->op);
            inst += " ";
            inst += varNodeToString(cmp->children[1]);
            inst += " ";
        }

        inst += oprt->children[1]->to_string();
        inst += "\n";

        insts_str.push_back(inst);
    
    }

    CondBrOnVarTile::CondBrOnVarTile () {
        /**
         *        br 
         *      v   label
         * =>
         * cjump v=1 label
         * */

        this->cost = 2;
        /**
         *  br, cmp, v1, v2, label
         * */
        this->nodenCnt = 3;
        this->cost = 1;
        this->matchedNodes = std::set<InstSelectNode *> ();
        this->pattern = new PatternTree();
        this->name = "Branch conditioned on variable"; 

        PatternNodeOperator *head = new PatternNodeOperator(OperatorType::br);
                
        head->AddChild(
            new PatternNodeOperand(ItemType::item_variable)
        );
        head->AddChild(
            new PatternNodeOperand(L3::item_labels)
        );
        
        this->pattern->head = head;
    }

    void CondBrOnVarTile::generateL2Inst(
        InstSelectNode * instNode,
        std::vector<std::string> & insts_str
    ) {
        assert(IN_SET(this->matchedNodes, instNode));
        assert(instNode->isOperator);
        InstSelectNodeOperator * oprt = (InstSelectNodeOperator *) instNode;

        /**
         *        br 
         *      v   label
         * =>
         *  cjump v < 0 label
         * */
        assert(oprt->children.size() == 2);

        std::string inst = "";
        inst += "cjump "; 
        inst += varNodeToString(oprt->children[0]);
        inst += " = ";
        inst += " 1 ";
        inst += oprt->children[1]->to_string();
        inst += "\n";
        insts_str.push_back(inst);
    }

    LabelTile::LabelTile () {
        /**
         *  label
         *  :label
         * =>
         *  :label
         * */

        this->cost = 1;
        this->nodenCnt = 2;
        this->matchedNodes = std::set<InstSelectNode *> ();
        this->pattern = new PatternTree();
        this->name = "Label";

        PatternNodeOperator *head = new PatternNodeOperator(OperatorType::label);
                
        head->AddChild(
            new PatternNodeOperand(ItemType::item_labels)
        );
        
        this->pattern->head = head;
    }

    void LabelTile::generateL2Inst(
        InstSelectNode * instNode,
        std::vector<std::string> & insts_str
    ) {
        assert(IN_SET(this->matchedNodes, instNode));
        assert(instNode->isOperator);
        InstSelectNodeOperator * oprt = (InstSelectNodeOperator *) instNode;

        /**
         *  label
         *  :label
         * =>>
         *  :label
         * */
        assert(oprt->children.size() == 1);

        std::string inst = "";
        inst += oprt->children[0]->to_string();
        inst += "\n";
        insts_str.push_back(inst);
    }

    LoadTile::LoadTile () {
        /**
         *  load var
         *  
         *  load
         *  var
         * 
         * =>
         *  %newV <- mem var 0
         * */
        this->cost = 1;
        this->nodenCnt = 2;
        this->matchedNodes = std::set<InstSelectNode *> ();
        this->pattern = new PatternTree();
        this->name = "LoadTile";

        PatternNodeOperator *head = new PatternNodeOperator(OperatorType::load);
        
        head->AddChild(
            new PatternNodeOperand(ItemType::item_variable)
        );
        this->pattern->head = head;
    }


    void LoadTile::generateL2Inst(
        InstSelectNode * instNode,
        std::vector<std::string> & insts_str
    ) {
        assert(IN_SET(this->matchedNodes, instNode));
        assert(instNode->isOperator);
        InstSelectNodeOperator * oprt = (InstSelectNodeOperator *) instNode;
        
        /**
         *  load var
         *  
         *  load
         *  var
         * 
         * =>>
         *  %newV <- mem var 0
         * */
        assert(oprt->children.size() == 1);
        oprt->representative = new ItemVariable(
            L3::prefix + std::to_string(L3::new_var_cnt++)
        );


        std::string inst = "";
        inst += varNodeToString(oprt);
        inst += " ";
        inst += OperatorType_toString(OperatorType::assign);     /*  <- */
        inst += " mem ";
        inst += varNodeToString(oprt->children[0]);
        inst += " 0 ";
        inst += "\n";

        insts_str.push_back(inst);
    }
    


    void tile_init(
        Program & p,
        std::vector<Tile *> & L3ToL2_tiles,
        std::string & prefix,
        std::string & FRet_prefix
    ) {
        L3::prefix = prefix;
        L3::new_var_cnt = 0;

        L3::FRet_prefix = FRet_prefix;
        L3::FRet_cnt = 0;

        L3ToL2_tiles.push_back(
            new AopTile()
        );
        
        
        /**
         * var <- {var | const | label | op | cmp | load}
         * */
        L3ToL2_tiles.push_back(
            new AssignToVarTile()
        );

        /**
         * store var <- {var | const | label | op | cmp | load}
         * */
        L3ToL2_tiles.push_back(
            new AssignToStoreTile()
        );

        L3ToL2_tiles.push_back(
            new ReturnTile()
        );

        L3ToL2_tiles.push_back(
            new ReturnValueTile()
        );


        /**
         *  push custom function tiles
         *      CallTiles only relies on isRuntimeCall and num_args
         * */
        std::set<int32_t> num_args_set = {
            0,          /* input */
            1,          /* print, tensor-error */
            2,          /* allocate, tensor-error */
            3,          /* tensor-error */
            4           /* tensor-error */
        };

        for (int32_t n : num_args_set) {
            L3ToL2_tiles.push_back(
                new CallTile(true, n)
            );
        }

        /**
         *  push custom function tiles
         *      CallTiles only relies on isRuntimeCall and num_args
         * */
        num_args_set.clear();
        for (Function * F : p.functions) {
            num_args_set.insert(F->arg_list.size());
        }

        for (int32_t n : num_args_set) {
            L3ToL2_tiles.push_back(
                new CallTile(false, n)
            );
        }


        
        L3ToL2_tiles.push_back(
            new CmpTile()
        );

        /**
         *  push branch tiles
         * */
        L3ToL2_tiles.push_back(
            new UncondBrTile()
        );
        L3ToL2_tiles.push_back(
            new CondBrOnCmpTile()
        );
        L3ToL2_tiles.push_back(
            new CondBrOnVarTile()
        );
        L3ToL2_tiles.push_back(
            new CondBrOnConstTile()
        );

        L3ToL2_tiles.push_back(
            new LabelTile()
        );

        L3ToL2_tiles.push_back(
            new LoadTile()
        );

    }


}