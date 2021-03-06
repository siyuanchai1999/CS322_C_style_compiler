#include <iostream>

#include "analysis.h"

// #include "L2.h"

#define IS_REG_VAR(item) (item->itemtype == ItemType::item_registers || item->itemtype == ItemType::item_variable)
#define NOT_RSP(item) (item != &L2::reg_rsp)

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define IN_MAP(map, key) (map.find(key) != map.end())
#define IN_SET(set, key) (set.find(key) != set.end())

namespace L3
{   
    void get_used_var(Item * item, std::vector<Item*> & used ){
        if (item  == NULL) {
            std::cerr << "null reference passed in get_used_var!\n";
        }

        switch (item->itemtype)
        {   
            case ItemType::item_labels:
            {
                // do nothing
                break;
            }

            case ItemType::item_constant :
            {
                // do nothing
                break;
            }

            case ItemType::item_variable :
            {
                used.push_back(item);
                break;
            }

            case ItemType::item_aop :
            {
                ItemAop * itemAop = (ItemAop *) item;
                get_used_var(itemAop->op1, used);
                get_used_var(itemAop->op2, used);
                break;
            }
            case ItemType::item_cmp :
            {
                ItemCmp * itemCmp = (ItemCmp *) item;
                get_used_var(itemCmp->op1, used);
                get_used_var(itemCmp->op2, used);
                break;
            }

            case ItemType::item_load :
            {
                ItemLoad * load = (ItemLoad *) item;
                get_used_var(load->varToLoad, used);
                break;
            }
            
            case ItemType::item_store :
            {
                ItemStore * store = (ItemStore *) item;
                get_used_var(store->dst, used);
                break;
            }

            case ItemType::item_call :
            {
                ItemCall * call = (ItemCall *) item;
                get_used_var(call->callee, used);

                for (Item * arg : call->args) {
                    get_used_var(arg, used);
                }

                break;
            }

            default:
                std::cerr << "wrong ItemType in get_used_var: " << item->itemtype << '\n';
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

        this->GEN = this->live_visitor.GEN;
        this->KILL = this->live_visitor.KILL;
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
            std::cout << inst->to_string();
            
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
            std::cout << inst->to_string();
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


    std::set<Item *> FunctionLivenessAnalyzer::get_used(Instruction * inst) {
        return this->GEN[inst];
    }

    std::set<Item *> FunctionLivenessAnalyzer::get_defs(Instruction * inst) {
        return this->KILL[inst];
    }

    std::set<Item *> FunctionLivenessAnalyzer::get_live_after(Instruction * inst) {
        return this->OUT[inst];
    }

    std::set<Item *> FunctionLivenessAnalyzer::get_live_since(Instruction * inst) {
        return this->IN[inst];
    }

    void LivenessVisitor::visit(Instruction_ret *ret) 
    {
        /**
         *  Nothing to do
         * */
    }

    void LivenessVisitor::visit(Instruction_ret_var *ret) 
    {
        // std::cerr << "This is a ret." << '\n';
        std::vector<Item *> used;
        get_used_var(ret->valueToReturn, used);
        
        this->GEN[ret].insert(
            used.begin(),
            used.end()
        );

    }

    void LivenessVisitor::visit(Instruction_label *label)  {
    
    }

    void LivenessVisitor::visit(Instruction_call * call)  {
        /**
         * call callee ( args ) | var <- call callee ( args )
         *      used : callee,  args
         *      def : var (if available)
         * */
        std::vector<Item *> used;

        if (call->ret) {
            
            get_used_var(call->ret, used);

            this->KILL[call].insert(
                used.begin(),
                used.end()
            );
        }


        used.clear();
        get_used_var(call->call_wrap, used);

        this->GEN[call].insert(
            used.begin(),
            used.end()
        );

    }


    void LivenessVisitor::visit(Instruction_assignment *assignment)  {
        
        /**
         *  assignment kills dst if it's a variable
         * */
        if (assignment->dst->itemtype == ItemType::item_variable) {
            this->KILL[assignment].insert(assignment->dst);
        }
        
        /**
         *  assignment gens store var if it's the dst = store var 
         * */

        std::vector<Item *> used;

        if (assignment->dst->itemtype == ItemType::item_store){
            get_used_var(assignment->dst, used);
        }
        
        /**
         *  assignment always gen vars in src
         * */
        get_used_var(assignment->src, used);

        this->GEN[assignment].insert(
            used.begin(),
            used.end()
        );

    }

    void LivenessVisitor::visit(Instruction_branch * br)  {
        

        if (br->condition) {
            /**
            *  Only used variable if condition is valid
            * */

            std::vector<Item *> used;
            get_used_var(br->condition, used);

            this->GEN[br].insert(
                used.begin(),
                used.end()
            );
        }

        
    }


    // SuccessorVisitor: finds successors
    void SuccessorVisitor::visit(Instruction_ret *ret)
    {
        /**
         *  return instruction has no successor
         * */
        this->successor[ret].clear();
    }

    void SuccessorVisitor::visit(Instruction_ret_var *ret)
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
    void SuccessorVisitor::visit(Instruction_call * call)
    {  
        /**
         *  if it's a tensor error call, it has no successor
         * */
        ItemLabel * callee_lb = (ItemLabel *) ((ItemCall *) call->call_wrap)->callee;
        if (callee_lb == & L3::tensor_label) {
            this->successor[call].clear();
        }
        
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
    
    void SuccessorVisitor::visit(Instruction_branch * br)
    {
        /**
         *   Special case: add conditional jump destination as a successor
         * */

        if (!br->condition) {
            /**
             *  unconditional branch clear original successor 
             * */
            this->successor[br].clear();
        }


        this->successor[br].push_back(
            this->label2Inst[br->dst]
        );
    }

    void SuccessorVisitor::associate_label(Function *F) {

        for (Instruction *instruction: F->instructions) {
            if (instruction->type == InstType::inst_label){
                Instruction_label * inst_label = (Instruction_label *) instruction;
                this->label2Inst[inst_label->item_label] = instruction;
            }   
        }
    }

    void SuccessorVisitor::find_successors(L3::Function *F)
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


    


} // namespace L2
