#include <iostream>

#include "analysis.h"

// #include "L2.h"

#define IS_REG_VAR(item) (item->itemtype == ItemType::item_registers || item->itemtype == ItemType::item_variable)
#define NOT_RSP(item) (item != &L2::reg_rsp)

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define IN_MAP(map, key) (map.find(key) != map.end())
#define IN_SET(set, key) (set.find(key) != set.end())

namespace L2
{   
    void get_used_reg_var(Item * item, std::vector<Item*> & used ){
        switch (item->itemtype)
        {
            case ItemType::item_registers :
            {
                if (NOT_RSP(item)) used.push_back(item);
                break;
            }

            case ItemType::item_constant :
            {
                // do nothing
                break;
            }

            case ItemType::item_memory :
            {
                ItemMemoryAccess * ItemAccess = (ItemMemoryAccess *) item;
                get_used_reg_var(ItemAccess->reg, used);
                break;
            }

            case ItemType::item_labels :
            {
                // do nothing
                break;
            }

            case ItemType::item_aop :
            {
                // do nothing
                break;
            }
            case ItemType::item_cmp :
            {
                ItemCmp * ItemAccess = (ItemCmp *) item;
                get_used_reg_var(ItemAccess->op1, used);
                get_used_reg_var(ItemAccess->op2, used);
                break;
            }

            case ItemType::item_variable :
            {
                used.push_back(item);
                break;
            }
            
            case ItemType::item_stack_arg :
            {
                // do nothing
                break;
            }

            default:
                std::cerr << "wrong ItemType in get_used_reg_var: " << item->itemtype << '\n';
                break;
        }
    }

    bool calc_live_OUT2IN(
        Instruction *inst,
        LiveSet &GEN,
        LiveSet &KILL,
        LiveSet &IN,
        LiveSet &OUT)
    {
        std::set<Item *> tempIN;
        /**
             *  p = &x;
             *  IN[i] = GEN[i] U (OUT[i] – KILL[i])
             * */
        std::set<Item *> outMinusKill;
        set_diff(
            OUT[inst],   /* srcA */
            KILL[inst],  /* srcB */
            outMinusKill /* target */
        );

        set_union(
            outMinusKill, /* srcA */
            GEN[inst],    /* srcB */
            tempIN        /* target */
        );

        bool diff = tempIN != IN[inst];
        IN[inst] = tempIN;
        return diff;
    }

    FunctionLivenessAnalyzer::FunctionLivenessAnalyzer(Function * F){
        this->F = F;
        
    }

    void FunctionLivenessAnalyzer::calculate_GENKILL() {
        for (Instruction * inst : this->F->instructions){
            inst->accept(this->live_visitor);
        }
    }

    void FunctionLivenessAnalyzer::output_GENKILL() {
        std::cout << '(' << "GEN" << '\n';
        
        for (Instruction *inst : this->F->instructions){
            // std::cerr << "gen size = " << this->live_visitor.GEN[inst].size() << '\n';
            std::cout << '(';
            for(Item * item : this->live_visitor.GEN[inst]){
                // std::cerr << item->to_string();
                // item->accept(this->item_output_visitor);
                
                std::cout << item->to_string();
                std::cout << ' ' ;
            }   
            std::cout << ')' << '\n';
        } 

        std::cout << ')' << '\n';

        std::cout << '(' << "KILL" << '\n';
        
        for (Instruction *inst : this->F->instructions){
            // std::cerr << "gen size = " << this->live_visitor.GEN[inst].size() << '\n';
            std::cout << '(';
            for(Item * item : this->live_visitor.KILL[inst]){
                // std::cerr << item->to_string();
                // item->accept(this->item_output_visitor);
                
                std::cout << item->to_string();
                std::cout << ' ' ;
            }   
            std::cout << ')' << '\n';
        } 

        std::cout << ')' << '\n';

    }

    /**
     * 
(:myF
0
                        GEN
  %myVar1 <- 0         

  %myVar1 += rdi

  return
)

(GEN
()
(rdi %myVar1 )
(rax rbx rbp r12 r13 r14 r15 )
)

(KILL
(%myVar1 )
(%myVar1 )
()
)
IN[+=] = 
OUT[+=] = {rax rbx rbp r12 r13 r14 r15}

IN[return] = {rax rbx rbp r12 r13 r14 r15}
OUT[return] = {}


     * */

    void FunctionLivenessAnalyzer::calculate_INOUT() {
        /**
         *  Find successors for each instruction
         * */
        this->succVisitor.find_successors(this->F);

        bool changed;

        do {
            changed = false;
            for (auto inst_it= F->instructions.rbegin(); inst_it != F->instructions.rend(); inst_it++ ){
                Instruction * inst = *inst_it;
                
                // std::cerr <<  "successor size "<< this->succVisitor.successor[inst].size() << '\n';
                this->OUT[inst].clear();
                for (Instruction * succ : this->succVisitor.successor[inst]){
                    this->OUT[inst].insert(
                        this->IN[succ].begin(),
                        this->IN[succ].end()
                    );
                }
                
                // output_INOUT();

                bool in_changed = calc_live_OUT2IN(
                                    inst,
                                    this->live_visitor.GEN,
                                    this->live_visitor.KILL,
                                    this->IN,
                                    this->OUT
                                );
                
                if (!changed) changed = in_changed;

                // output_INOUT();
            }

            // for (Instruction * inst : F->instructions){
            //     // Instruction * inst = *inst_it;
                
            //     bool in_changed = calc_live_OUT2IN(
            //                         inst,
            //                         this->live_visitor.GEN,
            //                         this->live_visitor.KILL,
            //                         this->IN,
            //                         this->OUT
            //                     );

            //     std::set<Item *> tempOut;            
            //     /**
            //      *  Calculate OUT of current instruction from IN of successors
            //      * */
            //     for (Instruction * succ : this->succVisitor.successor[inst]){
            //         this->OUT[inst].insert(
            //             this->IN[inst].begin(),
            //             this->IN[inst].end()
            //         );
            //     }
            //     bool out_changed = this->OUT[inst] != tempOut;

                
                
            //     changed = changed || in_changed || out_changed;

            // }

        } while(changed);
        
    }

    void FunctionLivenessAnalyzer::output_INOUT() {
        std::cout << '(' << '\n'; 
        std::cout << '(' << "in" << '\n';
        
        for (Instruction *inst : this->F->instructions){
            // std::cerr << "gen size = " << this->live_visitor.GEN[inst].size() << '\n';
            std::cout << '(';
            // for(Item * item : this->IN[inst]){
            //     // std::cerr << item->to_string();
            //     item->accept(this->item_output_visitor);
            //     std::cerr << ' ' ;
            // }   

            for (auto it = this->IN[inst].begin(); it != this->IN[inst].end(); it++){
                if (it != this->IN[inst].begin()) std::cout << ' ' ;

                std::cout << (*it)->to_string();
                // (*it)->accept(this->item_output_visitor);
            }
            std::cout << ')' << '\n';
        } 

        
        std::cout << ')' << '\n';
        std::cout << '\n';
        std::cout << '(' << "out" << '\n';
        
        for (Instruction *inst : this->F->instructions){
            // std::cerr << "gen size = " << this->live_visitor.GEN[inst].size() << '\n';
            std::cout << '(';

            // for(Item * item : this->OUT[inst]){
            //     // std::cerr << item->to_string();
            //     item->accept(this->item_output_visitor);
            //     std::cerr << ' ' ;
            // }   

            for (auto it = this->OUT[inst].begin(); it != this->OUT[inst].end(); it++){
                if (it != this->OUT[inst].begin()) std::cout << ' ' ;

                std::cout << (*it)->to_string();
                // (*it)->accept(this->item_output_visitor);
            }
            std::cout << ')' << '\n';
        } 

        std::cout << ')' << '\n';
        std::cout << '\n';
        std::cout << ')' << '\n';
    }

    LiveSet & FunctionLivenessAnalyzer::get_live_KILL()
    {
        return this->live_visitor.KILL;
    }

    LiveSet & FunctionLivenessAnalyzer::get_live_IN() 
    {
        return this->IN;
    }

    LiveSet & FunctionLivenessAnalyzer::get_live_OUT()
    {
        return this->OUT;
    }


    void LivenessVisitor::visit(Instruction_ret *ret) 
    {
        // std::cerr << "This is a ret." << '\n';
        this->GEN[ret].insert(& L2::reg_rax);

        // ret add all callee saved
        this->GEN[ret].insert(
            std::begin(callee_saved), 
            std::end(callee_saved)
        );

        // std::cerr << callee_saved[0]->rType << '\n';
    }

    void LivenessVisitor::visit(Instruction_label *label)  {
    
    }

    void LivenessVisitor::visit(Instruction_call_runtime *runtime_call)  {
        ItemConstant *constant = (ItemConstant *) runtime_call->num_args;

        int num_arg_used = MIN(constant->constVal, L2::ARG_NUM);

        this->GEN[runtime_call].insert(
            L2::arg_regs,
            L2::arg_regs + num_arg_used
        );

        this->KILL[runtime_call].insert(
            std::begin(L2::caller_saved), 
            std::end(L2::caller_saved)
        );


    }

    void LivenessVisitor::visit(Instruction_call_user *user_call)  {
        std::vector<Item *> used;
        get_used_reg_var(user_call->callee, used);
        this->GEN[user_call].insert(used.begin(), used.end());


        ItemConstant *constant = (ItemConstant *) user_call->num_args;
        int num_arg_used = MIN(constant->constVal, L2::ARG_NUM);

        this->GEN[user_call].insert(
            L2::arg_regs,
            L2::arg_regs + num_arg_used
        );


        this->KILL[user_call].insert(
            std::begin(L2::caller_saved), 
            std::end(L2::caller_saved)
        );
        
    }

    void LivenessVisitor::visit(Instruction_aop *aop)  {
        if (IS_REG_VAR(aop->op1)) {
            this->KILL[aop].insert(aop->op1);
        }

        std::vector<Item *> used;
        get_used_reg_var(aop->op1, used);
        get_used_reg_var(aop->op2, used);

        this->GEN[aop].insert(used.begin(), used.end());

    }

    void LivenessVisitor::visit(Instruction_assignment *assignment)  {
        // std::cerr << "This is a assignment." << '\n';
        
        // Define KILL
        if (IS_REG_VAR(assignment->dst)) {
            this->KILL[assignment].insert(assignment->dst);
        }
        
        std::vector<Item *> used;

        if (assignment->dst->itemtype == ItemType::item_memory){
            get_used_reg_var(assignment->dst, used);
        }

        // Define GET
               
        get_used_reg_var(assignment->src, used);

        this->GEN[assignment].insert(used.begin(), used.end());

        // std::cerr << "GEN size = " << this->GEN[assignment].size() << '\n';
        // std::cerr << "KILL size = " << this->KILL[assignment].size() << '\n';
    }

    void LivenessVisitor::visit(Instruction_sop *sop)  {
        if (IS_REG_VAR(sop->target)) {
            this->KILL[sop].insert(sop->target);
        } else {
            std::cerr << "wrong sop target type!\n";
        }

        std::vector<Item *> used;
        get_used_reg_var(sop->offset, used);
        get_used_reg_var(sop->target, used);

        this->GEN[sop].insert(used.begin(), used.end());
    }

    void LivenessVisitor::visit(Instruction_lea *lea)  {
        /**
         *  LEA destination/addr/multiplier must be a register (no rsp) or variable
         * */
        this->KILL[lea].insert(lea->dst);

        this->GEN[lea].insert(lea->addr);
        this->GEN[lea].insert(lea->multr);

    }
    void LivenessVisitor::visit(Instruction_goto *goto_inst)  {
        /**
         *  nothing to do
         * */
    }

    void LivenessVisitor::visit(Instruction_dec *dec)  {
        /**
         *  w-- GEN and KILL w at the same time
         * */
        this->GEN[dec].insert(dec->op);
        this->KILL[dec].insert(dec->op);
    }

    void LivenessVisitor::visit(Instruction_inc *inc)  {
        /**
         *  w++ GEN and KILL w at the same time
         * */
        this->GEN[inc].insert(inc->op);
        this->KILL[inc].insert(inc->op);
    }

    void LivenessVisitor::visit(Instruction_cjump *cjump)  {
        /**
         *  cjump only GEN
         * */
        std::vector<Item *> used;
        get_used_reg_var(cjump->condition, used);
         this->GEN[cjump].insert(used.begin(), used.end());
    }



    void ItemOutputVisitor::visit(ItemRegister * reg) {
        // std::cerr << "outputing register=" << reg->itemtype<< "\n";
        switch(reg->rType) {
            case rdi : 
                std::cout << "rdi";
                break;

            case rax : 
                std::cout << "rax";
                break; 

            case rsi : 
                std::cout << "rsi";
                break;
            
            case rdx : 
                std::cout << "rdx";
                break;
            
            case rcx : 
                std::cout << "rcx";
                break;

            case r8 : 
                std::cout << "r8";
                break;

            case r9 : 
                std::cout << "r9";
                break;
            
            case rbx : 
                std::cout << "rbx";
                break;
            
            case rbp : 
                std::cout << "rbp";
                break;
            
            case r10 : 
                std::cout << "r10";
                break;
            
            case r11 : 
                std::cout << "r11";
                break;
            
            case r12 : 
                std::cout << "r12";
                break;
            
            case r13 : 
                std::cout << "r13";
                break;
            
            case r14 : 
                std::cout << "r14";
                break;
            
            case r15 : 
                std::cout << "r15";
                break;

            case rsp : 
                std::cout << "rsp";
                break;
            
            default :
                std::cout << "wrong register type in ItemRegister::to_string\n";
        }
    
    }

    void ItemOutputVisitor::visit(ItemConstant *constant)
    {
    }

    void ItemOutputVisitor::visit(ItemLabel *label)
    {
    }

    void ItemOutputVisitor::visit(ItemMemoryAccess *mem)
    {
    }

    void ItemOutputVisitor::visit(ItemAop *aop)
    {
    }

    void ItemOutputVisitor::visit(ItemCmp *cmp)
    {
    }

    void ItemOutputVisitor::visit(ItemVariable *var)
    {
        std::cout << var->name;
    }

    void ItemOutputVisitor::visit(ItemStackArg *stack_arg)
    {
    }

    // SuccessorVisitor: finds successors
    void SuccessorVisitor::visit(Instruction_ret *ret)
    {
        /**
         *  return instruction has no successor
         * */
        this->successor[ret].clear();
    }
    void SuccessorVisitor::visit(Instruction_label *label)
    {
        /**
         *  do nothing, handled by general case
         *      next instruction is just the next instruction in instructin list
         * */
        return;
    }
    void SuccessorVisitor::visit(Instruction_call_runtime *runtime_call)
    {  
        /**
         *  if it's a tensor error call, it has no successor
         * */
        if (runtime_call->runtime_callee == & L2::tensor_label) {
            this->successor[runtime_call].clear();
        }
        
        return;
    }
    void SuccessorVisitor::visit(Instruction_call_user *user_call)
    {
        /**
         *  do nothing, handled by general case
         *      next instruction is just the next instruction in instructin list
         * */
        return;

    }
    void SuccessorVisitor::visit(Instruction_aop *aop)
    {
        /**
         *  do nothing, handled by general case
         *      next instruction is just the next instruction in instructin list
         * */
        return;
    }
    void SuccessorVisitor::visit(Instruction_assignment *assignment)
    {
        /**
         *  do nothing, handled by general case
         *      next instruction is just the next instruction in instructin list
         * */
        return;
    }
    void SuccessorVisitor::visit(Instruction_sop *sop)
    {
        /**
         *  do nothing, handled by general case
         *      next instruction is just the next instruction in instructin list
         * */
        return;
    }
    void SuccessorVisitor::visit(Instruction_lea *lea)
    {
        /**
         *  do nothing, handled by general case
         *      next instruction is just the next instruction in instructin list
         * */
        return;
    }
    void SuccessorVisitor::visit(Instruction_goto *inst_goto)
    {
        // successor is the associated label
        this->successor[inst_goto].clear();
        this->successor[inst_goto].push_back(this->label2Inst[inst_goto->gotoLabel]);

    }
    void SuccessorVisitor::visit(Instruction_dec *dec)
    {
        /**
         *  do nothing, handled by general case
         *      next instruction is just the next instruction in instructin list
         * */
        return;
    }
    void SuccessorVisitor::visit(Instruction_inc *inc)
    {
        /**
         *  do nothing, handled by general case
         *      next instruction is just the next instruction in instructin list
         * */
        return;
    }
    void SuccessorVisitor::visit(Instruction_cjump *cjump)
    {
        // Special case: add conditional jump destination as a successor
        this->successor[cjump].push_back(this->label2Inst[cjump->dst]);
    }

    void SuccessorVisitor::associate_label(Function *F) {

        for (Instruction *instruction: F->instructions) {
            if (instruction->type == InstType::inst_label){
                Instruction_label * inst_label = (Instruction_label *) instruction;
                this->label2Inst[inst_label->item_label] = instruction;
            }   
        }
    }

    void SuccessorVisitor::find_successors(L2::Function *F)
    {
        /**
         * build map from item label to label instruction
         * */

        this->associate_label(F);

        /**
         *  general case: populate successor[] with the next instruction
         * */
        for (int i = 0; i < F->instructions.size() - 1; i++){
            this->successor[F->instructions[i]].push_back( F->instructions[i + 1]);
        }

        /**
         *  Special case call visit
         * */
        for (Instruction *instruction: F->instructions)
        {
            // this->visit(instruction);
            instruction->accept(*this);
        }

    }

    InterferenceGraph::InterferenceGraph() {
        this->nodes = std::set<Item *>();
        this->edges =  std::map<Item *, std::set<Item *>>();
    }

    InterferenceGraph::InterferenceGraph(
        std::set<Item *> & nodes,
        std::map<Item *, std::set<Item *>> & edges
    ) {
        this->nodes = nodes;
        this->edges = edges;
    }

    bool InterferenceGraph::add_edge(Item *v1, Item *v2) {
        this->nodes.insert(v1);
        this->nodes.insert(v2);

        this->edges[v1].insert(v2);
        this->edges[v2].insert(v1);

        return true;
    }

    bool InterferenceGraph::remove_node(Item * toRemove) {
        if (!IN_SET(this->nodes, toRemove)) {
            std::cerr << "cannot find node " << toRemove->to_string() << '\n';
            return false;
        }

    
        /** 
         *  toRemove -- v
         *  delete (v, toRemove) 
         * */
        for (Item * v : this->edges[toRemove]) {
            this->edges[v].erase(toRemove);
        }

        /**
         *  Delete (toRemove, v)
         * */
        this->edges.erase(toRemove);

        this->nodes.erase(toRemove);

        return true;
    }
    
    void InterferenceGraph::get_nodes_stats(std::map<int32_t, std::set<Item * >> & edgeCnt2nodes) {
        for (Item * n : this->nodes) {
            edgeCnt2nodes[this->edges[n].size()].insert(n);
        }
    }
    
    std::set<Item *> & InterferenceGraph::get_neighbors(Item * node) {
        if (!IN_MAP(this->edges, node)){

            std::cerr << "cannot find node " << node->to_string() << '\n';
        }
        return this->edges[node];
    }


    FunctionInterferenceAnalyzer::FunctionInterferenceAnalyzer(
        Function *F,
        LiveSet &KILL,
        LiveSet &IN,
        LiveSet &OUT)
    {
        this->F = F;
        this->KILL = KILL;
        this->IN = IN;
        this->OUT = OUT;

        nodes = std::set<Item *>();
        edges = std::map<Item *, std::set<Item *>>();
    }

    /**
      *  connect everything in varsA with everything in VarsB
      * */
         
    void FunctionInterferenceAnalyzer::connect_two_sets(std::set<Item *> &varsA, std::set<Item *> &varsB) 
    {
        for (Item * v1: varsA) {
            this->nodes.insert(v1);
            this->edges[v1].insert(varsB.begin(), varsB.end());
            this->edges[v1].erase(v1);
        }

        for (Item * v2: varsB) {
            this->nodes.insert(v2);
            this->edges[v2].insert(varsA.begin(), varsA.end());
            this->edges[v2].erase(v2);
        }

    }

    /**
     *  fully connect everything in vars with every other variables.
     * */
    void FunctionInterferenceAnalyzer::full_connect(std::set<Item *> &vars)
    {
        for (Item *var : vars) {
            this->nodes.insert(var);
        }

        for (Item *a : vars) {
            for (Item *b : vars) {
                if (a != b) {
                    this->edges[a].insert(b);
                    this->edges[b].insert(a);
                }
            }
        }
    }

    /**
     *  Add connections within GP registers
     * */
    void FunctionInterferenceAnalyzer::add_GPRegisters_edges() {
        std::set<Item *> GP_reg_set(std::begin(GP_regs), std::end(GP_regs));

        this->full_connect(GP_reg_set);
    }

    /**
     *  Connect two variable if they are in the same IN/OUT set
     * */
    void FunctionInterferenceAnalyzer::add_INOUT_edges() {

        for (Instruction * inst : this->F->instructions) {
            this->full_connect(this->IN[inst]);
            this->full_connect(this->OUT[inst]);
        }
    }
    

    /**
     *  Connect every vars KILL[i] to OUT[i]
     * */
    void FunctionInterferenceAnalyzer::add_KILLOUT_edges() {

        for (Instruction * inst : this->F->instructions) {
            this->connect_two_sets(
                this->KILL[inst],
                this->OUT[inst]
            );
        }
    }

    void FunctionInterferenceAnalyzer::add_shift_edges() {
        std::set<Item *> GP_reg_no_rcx(std::begin(GP_regs), std::end(GP_regs));
        GP_reg_no_rcx.erase(& L2::reg_rcx);

        for(Instruction * inst : this->F->instructions) {
            if (inst->type == InstType::inst_sop) {
                Instruction_sop * sop = (Instruction_sop *) inst;

                if (IS_REG_VAR(sop->offset)) {

                    std::set<Item *> single_wrapper = {sop->offset};
                    this->connect_two_sets(GP_reg_no_rcx, single_wrapper);

                }        

            }
        }
    }

    void FunctionInterferenceAnalyzer::build_Inteference_graph() {
        this->add_GPRegisters_edges();
        this->add_INOUT_edges();
        this->add_KILLOUT_edges();
        this->add_shift_edges();
    }

    /**
     *  TODO: Optimize for future performance 
     * */
    InterferenceGraph FunctionInterferenceAnalyzer::getIntGraph() {
        InterferenceGraph intG (
            this->nodes,
            this->edges
        );
        return intG;
    }

    void FunctionInterferenceAnalyzer::output_Inteference() {
        // (*it)->accept(this->item_output_visitor);

        for (Item * node : this->nodes) {

            node->accept(this->item_output_visitor);
            std::cout << ' ';
            
            for (Item * v: this->edges[node]) {
                std::cout << v->to_string();
                // v->accept(this->item_output_visitor);
                
                std::cout << ' ';
            }
            std::cout << '\n';
        }
    }

    void run_liveAnalysis(Program &p)
    {
        // std::cerr << "Run run live analysis!\n";
        // std::cout << " cout run live analysis!\n";

        for (Function * f : p.functions){
            FunctionLivenessAnalyzer analyzer(f);
            
            // std::cerr << "done analyzer build!\n";
            // std::cout << " cout done analyzer build!\n"<< std::flush;

            analyzer.calculate_GENKILL();
            // std::cerr << "done GENKILL calc!\n";
            // std::cout << " cout done GENKILL calc!\n"<< std::flush;

            // analyzer.output_GENKILL();
            // std::cerr << "done GENKILL!\n";
            
            analyzer.calculate_INOUT();
            // std::cerr << "done INOUT!\n";
            
            analyzer.output_INOUT();
        }
       
    }

    void run_Interference(Program &p) {

        for (Function * f : p.functions){
            
            /**
             *  run liveness analysis
             * */
            FunctionLivenessAnalyzer live_analyzer(f);
            live_analyzer.calculate_GENKILL();
            live_analyzer.calculate_INOUT();

            // live_analyzer.output_INOUT();
            /**
             *  bind liveness analysis with interference analysis
             * */
            FunctionInterferenceAnalyzer int_analyzer(
                f,
                live_analyzer.get_live_KILL(),
                live_analyzer.get_live_IN(),
                live_analyzer.get_live_OUT()
            );

            /**
             *  Run interference analysis and output
             * */
            int_analyzer.build_Inteference_graph();
            int_analyzer.output_Inteference();
        }
    }

    


} // namespace L2
