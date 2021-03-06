
#include "register_allocation.h"

#define IN_MAP(map, key) (map.find(key) != map.end())
#define IN_SET(set, key) (set.find(key) != set.end())

// #define REG_DEBUG 0

#ifdef REG_DEBUG
#define DEBUG_OUT (std::cerr << "DEBUG-Register-Allocation: ") // or any other ostream
#else
#define DEBUG_OUT 0 && std::cerr
#endif

namespace L2 {
    std::unordered_map<Item *, Color> reg2color = {
        {&reg_rdi, rdi_color},
        {&reg_rax, rax_color},
        {&reg_rsi, rsi_color},
        {&reg_rdx, rdx_color},
        {&reg_rcx, rcx_color},
        {&reg_r8 , r8_color},
        {&reg_r9 , r9_color},
        {&reg_rbx, rbx_color},
        {&reg_rbp, rbp_color},
        {&reg_r10, r10_color},
        {&reg_r11, r11_color},
        {&reg_r12, r12_color},
        {&reg_r13, r13_color},
        {&reg_r14, r14_color},
        {&reg_r15, r15_color}
    };

    std::unordered_map<Color, Item *> color2reg = {
        {rdi_color, &reg_rdi},
        {rsi_color, &reg_rsi},
        {rdx_color, &reg_rdx},
        {rcx_color, &reg_rcx},
        {r8_color, &reg_r8},
        {r9_color, &reg_r9},
        {rax_color, &reg_rax},
        {r10_color, &reg_r10},
        {r11_color, &reg_r11},
        {r12_color, &reg_r12},
        {r13_color, &reg_r13},
        {r14_color, &reg_r14},
        {r15_color, &reg_r15},
        {rbp_color, &reg_rbp},
        {rbx_color, &reg_rbp}
    };


    VarColorVisitor::VarColorVisitor(
        std::unordered_map<Item *, Color> & item2color,
        Function * F
    ) {
        this->F = F;
        this->item2color = item2color;
    }

    bool VarColorVisitor::contains_varToColor(
        Item ** itemAddr,
        std::vector<Item **> & placeToReplace
    ){  
        Item * item = *itemAddr;
        switch (item->itemtype)
        {
            case ItemType::item_registers :
            {
                return false;
            }

            case ItemType::item_constant :
            {
                return false;
            }

            case ItemType::item_memory :
            {
                ItemMemoryAccess * ItemAccess = (ItemMemoryAccess *) item;
                return contains_varToColor(&ItemAccess->reg, placeToReplace);
            }

            case ItemType::item_labels :
            {
                return false;
            }

            case ItemType::item_aop :
            {
                return false;
            }
            case ItemType::item_cmp :
            {
                ItemCmp * ItemAccess = (ItemCmp *) item;
                bool op1Used = contains_varToColor(&ItemAccess->op1, placeToReplace);
                bool op2Used = contains_varToColor(&ItemAccess->op2, placeToReplace);

                return op1Used || op2Used;
            }

            case ItemType::item_variable :
            {
                if (IN_MAP(this->item2color, item)) {
                    placeToReplace.push_back(itemAddr);
                    return true;
                }
                
                return false;
            }
            
            case ItemType::item_stack_arg :
            {
                return false;
            }

            default:
                std::cerr << "wrong ItemType in get_used_reg_var: " << item->itemtype << '\n';
                break;
        }

        return false;
    }

    void VarColorVisitor::visit(Instruction_ret * ret) {
        /**
         *  Nothing to do 
         * */
    }          
    void VarColorVisitor::visit(Instruction_label * label_inst)  {
        /**
         *  Nothing to do 
         * */
    }        
    void VarColorVisitor::visit(Instruction_call_runtime *runtime_call) {
        /**
         *  Nothing to do 
         * */
    }
    void VarColorVisitor::visit(Instruction_call_user *user_call) {
        std::vector<Item **> placeToReplace;
        bool var_used = this->contains_varToColor(&user_call->callee, placeToReplace);

        if (var_used) {
            for (Item ** itemAddr : placeToReplace) {
                Item* var = *itemAddr;
                *itemAddr = L2::color2reg[this->item2color[var]];
            }
        }
    }

    void VarColorVisitor::visit(Instruction_aop *aop) {
        std::vector<Item **> placeToReplace;
        bool op1Used = this->contains_varToColor(&aop->op1, placeToReplace);
        bool op2Used = this->contains_varToColor(&aop->op2, placeToReplace);

        if (op1Used ||  op2Used ) {
            for (Item ** itemAddr : placeToReplace) {
                Item* var = *itemAddr;
                *itemAddr = L2::color2reg[this->item2color[var]];
            }
        }
    }

    void VarColorVisitor::visit(Instruction_assignment *assignment) {
        std::vector<Item **> placeToReplace;
        bool dstUsed = this->contains_varToColor(&assignment->dst, placeToReplace);
        bool srcUsed = this->contains_varToColor(&assignment->src, placeToReplace);

        if (dstUsed ||  srcUsed ) {
            for (Item ** itemAddr : placeToReplace) {
                Item* var = *itemAddr;
                *itemAddr = L2::color2reg[this->item2color[var]];
            }
        }
    }

    void VarColorVisitor::visit(Instruction_sop *sop) {
        std::vector<Item **> placeToReplace;
        bool targetUsed = this->contains_varToColor(&sop->target, placeToReplace);
        bool offsetUsed = this->contains_varToColor(&sop->offset, placeToReplace);

        if (targetUsed ||  offsetUsed ) {
            for (Item ** itemAddr : placeToReplace) {
                Item* var = *itemAddr;
                *itemAddr = L2::color2reg[this->item2color[var]];
            }
        }
    }    


    void VarColorVisitor::visit(Instruction_lea *lea) {
        std::vector<Item **> placeToReplace;
        bool dstUsed = this->contains_varToColor(&lea->dst, placeToReplace);
        bool addrUsed = this->contains_varToColor(&lea->addr, placeToReplace);
        bool multrUsed = this->contains_varToColor(&lea->multr, placeToReplace);
        
        if (dstUsed ||  addrUsed || multrUsed) {
            for (Item ** itemAddr : placeToReplace) {
                Item* var = *itemAddr;
                *itemAddr = L2::color2reg[this->item2color[var]];
            }
        }
    }    

    void VarColorVisitor::visit(Instruction_goto *inst_goto) {
        /**
         *  Nothing to do 
         * */

    }            
    void VarColorVisitor::visit(Instruction_dec *dec) {
        std::vector<Item **> placeToReplace;
        bool varUsed = this->contains_varToColor(&dec->op, placeToReplace);
        
        if (varUsed) {
            for (Item ** itemAddr : placeToReplace) {
                Item* var = *itemAddr;
                *itemAddr = L2::color2reg[this->item2color[var]];
            }
        }

    }
    void VarColorVisitor::visit(Instruction_inc *inc) {
        std::vector<Item **> placeToReplace;
        bool varUsed = this->contains_varToColor(&inc->op, placeToReplace);
        
        if (varUsed) {
            for (Item ** itemAddr : placeToReplace) {
                Item* var = *itemAddr;
                *itemAddr = L2::color2reg[this->item2color[var]];
            }
        }
    }

    void VarColorVisitor::visit(Instruction_cjump *cjump) {
        std::vector<Item **> placeToReplace;
        bool varUsed = this->contains_varToColor(&cjump->condition, placeToReplace);
        
        if (varUsed) {
            for (Item ** itemAddr : placeToReplace) {
                Item* var = *itemAddr;
                *itemAddr = L2::color2reg[this->item2color[var]];
            }
        }
    }


    RegisterAllocator::RegisterAllocator(Function * F, Function ** F_addr) {
        this->F = F;
        this->F_addr = F_addr;
    }
    

    // void RegisterAllocator::allcoate() {
        
    //     Function * F_copy = this->F->copy();
    //     bool shouldSpillAll = false;
    //     /**
    //      * "%SPILL_VAR_SYMBOL_%d_%d", iteration, index of varsToSpill
    //      * */
    //     std::string SpillPrefixPre = "%SPILL_VAR_SYMBOL_";

    //     int32_t i = 0;
    //     bool AllAssigned = false;
        
    //     std::set<Item *> prevSpillReplace;
    //     do {

        
    //         /**
    //          *  run liveness analysis
    //          * */
    //         FunctionLivenessAnalyzer live_analyzer(this->F);
    //         live_analyzer.calculate_GENKILL();
    //         live_analyzer.calculate_INOUT();

    //         DEBUG_OUT << "Done: " << "liveness analysis!" << '\n';
    //         // live_analyzer.output_INOUT();

            
    //         /**
    //          *  bind liveness analysis with interference analysis
    //          * */
    //         FunctionInterferenceAnalyzer int_analyzer(
    //             this->F,
    //             live_analyzer.get_live_KILL(),
    //             live_analyzer.get_live_IN(),
    //             live_analyzer.get_live_OUT()
    //         );

    //         /**
    //          *  Run interference analysis and output
    //          * */
    //         int_analyzer.build_Inteference_graph();
            
    //         // int_analyzer.output_Inteference();
    //         DEBUG_OUT << "Done: " << "Interference Graph!" << '\n';
            

    //         InterferenceGraph intGraph = int_analyzer.getIntGraph();
    //         std::stack<Item *>  nodeStack;
    //         std::unordered_map<Item *, Color> item2color;
    //         std::set<Item *> NonColorItems;
    //         std::vector<Item *> varsToSpill;

 
    //         /**z
    //          *  color_num default to be 15, number of L2 GP register
    //          * */
    //         NodeSelector node_selector(
    //             intGraph,
    //             L2::COLOR_NUM
    //         );

    //         /**
    //          *  populate stack of variables/registers to color
    //          * */
    //         node_selector.populate_stack(nodeStack);

    //         DEBUG_OUT << "Done: " << "Node selection!" << '\n';

 
 
    //         ColorSelector color_selector(
    //             &intGraph,
    //             &nodeStack
    //         );

    //         AllAssigned = color_selector.assignColorForAll(
    //             item2color,
    //             NonColorItems
    //         );

    //         DEBUG_OUT << "Done: " << "Assign colors!" << '\n';



    //         this->color_variables(item2color);
    //         DEBUG_OUT << "Done: " << "Color variables!" << '\n';



    //         SpillVarSelector spill_selector(&NonColorItems, &prevSpillReplace);
    //         // spill_selector.
    //         spill_selector.selectVarsToSpill(varsToSpill);

    //         if (!NonColorItems.empty() && varsToSpill.empty()) {
    //             /***
    //              *  Cannot color all variables but cannot spill any variable
    //              *      break out of the loop
    //              * */
    //             shouldSpillAll = true;
    //             break;
    //         }

    //         for (uint32_t j = 0 ; j < varsToSpill.size(); j++) {

    //             /**
    //             * "%SPILL_VAR_SYMBOL_%d_%d", iteration, index of varsToSpill
    //             * */
    //             std::string prefix_str =    SpillPrefixPre 
    //                                     +   std::to_string(i) + "_"
    //                                     +   std::to_string(j) + "_";

    //             ItemVariable * prefix = new ItemVariable(
    //                 prefix_str
    //             );
                
    //             Spiller sp (
    //                 this->F,
    //                 (ItemVariable *) varsToSpill[j],
    //                 prefix
    //             );

    //             sp.spill_variables();
                
    //             std::vector<ItemVariable *> var_replacements = sp.get_var_replacement();

    //             /**
    //              * append variables created by spilling to the prevSpillReplace
    //              * */
    //             prevSpillReplace.insert(
    //                 var_replacements.begin(),
    //                 var_replacements.end()
    //             );
    //         }
            
    //         i++;

    //         DEBUG_OUT << "AllAssigned = " << AllAssigned << '\n';

    //         this->F->print();
    //     } while (!AllAssigned );


        

    // }
    
    bool RegisterAllocator::analysisAndColoring(
        Function * F,
        std::set<Item *> & NonColorItems
    ){
        /**
         *  run liveness analysis
         * */
        FunctionLivenessAnalyzer live_analyzer(F);
        live_analyzer.calculate_GENKILL();
        live_analyzer.calculate_INOUT();

        DEBUG_OUT << "Done: " << "liveness analysis!" << '\n';
        // live_analyzer.output_INOUT();



        /**
         *  bind liveness analysis with interference analysis
         * */
        FunctionInterferenceAnalyzer int_analyzer(
            F,
            live_analyzer.get_live_KILL(),
            live_analyzer.get_live_IN(),
            live_analyzer.get_live_OUT()
        );


        /**
         *  Run interference analysis and output
         * */
        int_analyzer.build_Inteference_graph();
        
        // int_analyzer.output_Inteference();
        DEBUG_OUT << "Done: " << "Interference Graph!" << '\n';


        InterferenceGraph intGraph = int_analyzer.getIntGraph();
        std::stack<Item *>  nodeStack;
        std::unordered_map<Item *, Color> item2color;
        


        /**
         *  color_num default to be 15, number of L2 GP register
         * */
        NodeSelector node_selector(
            intGraph,
            L2::COLOR_NUM
        );

        /**
         *  populate stack of variables/registers to color
         * */
        node_selector.populate_stack(nodeStack);

        DEBUG_OUT << "Done: " << "Node selection!" << '\n';

        ColorSelector color_selector(
            &intGraph,
            &nodeStack
        );

        bool AllAssigned = color_selector.assignColorForAll(
            item2color,
            NonColorItems
        );

        DEBUG_OUT << "Done: " << "Assign colors!" << '\n';


        this->color_variables(F, item2color);
        DEBUG_OUT << "Done: " << "Color variables!" << '\n';

        return AllAssigned;
    }

    void RegisterAllocator::allcoate() {
        
        Function * F_copy = this->F->copy();
        bool shouldSpillAll = false;
        /**
         * "%SPILL_VAR_SYMBOL_%d_%d", iteration, index of varsToSpill
         * */
        std::string SpillPrefixPre = "%SPILL_VAR_SYMBOL_";

        int32_t i = 0;
        bool AllAssigned = false;
        
        std::set<Item *> prevSpillReplace;
        do {


            std::set<Item *> NonColorItems;
            std::vector<Item *> varsToSpill;

            AllAssigned = this->analysisAndColoring(
                this->F,
                NonColorItems
            );


            SpillVarSelector spill_selector(&NonColorItems, &prevSpillReplace);
            // spill_selector.
            spill_selector.selectVarsToSpill(varsToSpill);

            if (!NonColorItems.empty() && varsToSpill.empty()) {
                /***
                 *  Cannot color all variables but cannot spill any variable
                 *      break out of the loop
                 * */
                DEBUG_OUT << "Cannot color all variables but cannot spill any variable!\n";
                DEBUG_OUT << "spill all variables!\n";
                shouldSpillAll = true;

                for (Item * it : prevSpillReplace) {
                    delete (ItemVariable *) it;
                }
                
                break;
            }

            for (uint32_t j = 0 ; j < varsToSpill.size(); j++) {

                /**
                * "%SPILL_VAR_SYMBOL_%d_%d", iteration, index of varsToSpill
                * */
                std::string prefix_str =    SpillPrefixPre 
                                        +   std::to_string(i) + "_"
                                        +   std::to_string(j) + "_";

                ItemVariable * prefix = new ItemVariable(
                    prefix_str
                );
                
                Spiller sp (
                    this->F,
                    (ItemVariable *) varsToSpill[j],
                    prefix
                );

                sp.spill_variables();
                
                std::vector<ItemVariable *> var_replacements = sp.get_var_replacement();

                /**
                 * append variables created by spilling to the prevSpillReplace
                 * */
                prevSpillReplace.insert(
                    var_replacements.begin(),
                    var_replacements.end()
                );
            }
            
            i++;

            DEBUG_OUT << "AllAssigned = " << AllAssigned << '\n';

        } while (!AllAssigned );


        
        if (shouldSpillAll) {
            
            /**
             *  Spill All
             * */
            F_copy->print();
            
            std::string SpillALLPrefixPre = "%SPILLALL_VAR_SYMBOL_";
            
            std::vector<Item * > allVars;
            for (auto & kv : F_copy->varName2ptr) {
                allVars.push_back(kv.second);
            }

            for (uint32_t it = 0; it < allVars.size(); it ++) {
                /**
                * "%SPILL_VAR_SYMBOL_%d_%d", iteration, index of varsToSpill
                * */
                std::string prefix_str =    SpillALLPrefixPre 
                                        +   std::to_string(it) + "_"; 

                ItemVariable * prefix = new ItemVariable(prefix_str);

                Spiller sp (
                    F_copy,
                    (ItemVariable *) allVars[it],
                    prefix
                );

                sp.spill_variables();

            } 

            F_copy->print();
            
            std::set<Item *> NonColorItems;
            std::vector<Item *> varsToSpill;

            this->analysisAndColoring(
                F_copy,
                NonColorItems
            );
            // F_copy->print();

            *this->F_addr = F_copy;
            return;
        }
        
        
    }


    void RegisterAllocator::color_variables(
        Function * F,
        std::unordered_map<Item *, Color> & item2color
    ) {
        VarColorVisitor var_color_visitor(
            item2color,
            F
        );

        /**
         *  color variables
         *      replace variables in item2color
         * */
        for (Instruction * inst : F->instructions) {
            inst->accept(var_color_visitor);
        }   

        /**
         *  clean items that replaced by registers
         * */

        // for (auto & kv : item2color) {
        //     Item * it = kv.first;
        //     if (it->itemtype == ItemType::item_variable) {
        //         ItemVariable * var = (ItemVariable *) it;
        //         delete var;
        //     }
        // }
    }

    


    NodeSelector::NodeSelector(
                InterferenceGraph & intGraph, 
                int32_t color_num
    ) {
        this->intGraph = intGraph;
        this->color_num = color_num;
    }

    void NodeSelector::populate_stack(std::stack<Item *> & nodeStack) {
        while (Item * it = this->select_next_node())
        {   
            if(it->itemtype == ItemType::item_variable) {
                DEBUG_OUT << "push on stack " << it->to_string() << '\n';
                nodeStack.push(it);
            }
            
        }
        
    }

    Item * NodeSelector::select_next_node() {

        /**
         *  edgeCnt2nodes contains the following information
         *      (number_of_edges, set of nodes tha have such number_of_edges)
         *   
         *  Note that map keeps an increasing on number_of_edges
         * */
        std::map<int32_t, std::set<Item * >> edgeCnt2nodes;
        this->intGraph.get_nodes_stats(edgeCnt2nodes);
        
        // DEBUG_OUT << "edgeCnt2nodes.size() = " << edgeCnt2nodes.size() << '\n';
        Item * toRemove = NULL;
        if (edgeCnt2nodes.size() == 0) return NULL;

        int32_t lowest_num_edges = edgeCnt2nodes.begin()->first;

        if (lowest_num_edges <= this->color_num) {
            /**
             *  toRemove = the one with most number of edges <= this->color_num
             * */
            for (auto & kv : edgeCnt2nodes) {
                int32_t number_of_edges = kv.first;
            
                if (number_of_edges > this->color_num) {
                    break;
                }

                toRemove = *kv.second.begin();
            }

        } else {
            /**
             *  toRemove = the one with most number of edges 
             * */
            toRemove = *(edgeCnt2nodes.rbegin()->second.begin());
        }

        this->intGraph.remove_node(toRemove);

        return toRemove;
   
    }


    ColorSelector::ColorSelector (
        InterferenceGraph * intGraph, 
        std::stack<Item *> *nodeStack
    ) {
        this->intGraph = intGraph;
        this->nodeStack = nodeStack;
    }

    /**
     *  Try to assign colors for all variables
     *      if failed return false; otherwise true
     *      @varsToSpill will contain the variables that cannot be colored
     *      @item2color will contain mapping from variable to its color
     * */
    bool ColorSelector::assignColorForAll(
        std::unordered_map<Item *, Color> & item2color,         /*output*/
        std::set<Item *> & NonColorItems                       /*output*/
    ) {
        
        /**
         *  Precolor registers
         * */
        this->pre_color_registers(item2color);
        
        
        while (!this->nodeStack->empty())
        {
            Item * var = this->nodeStack->top();
            this->nodeStack->pop();

            if (var->itemtype == ItemType::item_variable) {
                
                bool found_color = this->select_color(var, item2color);
                if (!found_color) {
                    NonColorItems.insert(var);
                    DEBUG_OUT << "Cannnot color " << var->to_string() << '\n';
                } else {
                    DEBUG_OUT << "Color " << var->to_string() << " as " << color2reg[item2color[var]]->to_string() << '\n';
                }

            } else if (var->itemtype == ItemType::item_registers) {
                /**
                 * If it's a register we already colored it
                 *      nothing to do
                 * */

            } else {
                std::cerr << "error type of var->itemtype = " << var->itemtype << " in  ColorSelector::assignColorForAll" << '\n';
            }
        }

        return NonColorItems.size() == 0;        
    }


    void ColorSelector::pre_color_registers(std::unordered_map<Item *, Color> & item2color) {
        item2color.insert(
            reg2color.begin(),
            reg2color.end()
        );
    }

    bool ColorSelector::select_color(Item * toColor, std::unordered_map<Item *, Color> & item2color) {
        /**
         *  • Sort the colors at design time starting from caller save registers
         *  • Use the lowest free color
         */
        std::set<Item *> neighbors = this->intGraph->get_neighbors(toColor);
        std::set<Color> neighbor_colors;

        for (Item * n : neighbors) {
            if (IN_MAP(item2color, n)) {
                neighbor_colors.insert(item2color[n]);
            }
        }


        for (uint32_t i = 0; i < L2::COLOR_NUM; i++) {
            Color c = sorted_color[i];
            
            if (!IN_SET(neighbor_colors, c)) {
                item2color[toColor] = c;
                return true;
            }
        }

        return false;
    }

    SpillVarSelector::SpillVarSelector(
        std::set<Item *> * NonColorItems,
        std::set<Item *> * prevSpillReplace
    ) {
        this->NonColorItems = NonColorItems;
        this->prevSpillReplace = prevSpillReplace;
    }

    /**
     *  spill all right now
     * */
    void SpillVarSelector::selectVarsToSpill(std::vector<Item *> &varsToSpill){
        std::set<Item*> target;
        set_diff(
            *this->NonColorItems,           /* srcA */
            *this->prevSpillReplace,        /* srcB */
            target                          /* target */
        );


        varsToSpill.insert(
            varsToSpill.end(),              /* position to insert*/
            target.begin(),                 /* first iterator */
            target.end()                    /* end iterator */
        );
    }

    void run_register_allocation(Program &p) {
        for (int32_t i = 0; i < p.functions.size(); i++) {
            RegisterAllocator reg_alloc(p.functions[i], &p.functions[i]);

            DEBUG_OUT << "Begin allocation for " << p.functions[i]->name << '\n';
            reg_alloc.allcoate();

            p.functions[i]->print();
        }
    }
}   