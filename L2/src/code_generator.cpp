#include <code_generator.h>
#include <assert.h>

#define CODE_GEN_DEBUG 1

#ifdef CODE_GEN_DEBUG
#define DEBUG_OUT (std::cerr << "DEBUG-Code-Generator: ") // or any other ostream
#else
#define DEBUG_OUT 0 && std::cerr
#endif

#define QUADSIZE 8
#define REG_ARGS_NUM 6
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

namespace L2 {
    L2ToL1_GeneratorVisitor::L2ToL1_GeneratorVisitor() {
        this->out = NULL;
    }
    
    L2ToL1_GeneratorVisitor::L2ToL1_GeneratorVisitor(std::ofstream *outputFile) {
        this->out = outputFile;
    }

    void L2ToL1_GeneratorVisitor::visit(Instruction_ret *ret)  {
        *this->out << ret->to_string();
    }

    void L2ToL1_GeneratorVisitor::visit(Instruction_label *label)  {
        *this->out << label->to_string() ;
        
    }
    
    void L2ToL1_GeneratorVisitor::visit(Instruction_call_runtime *runtime_call)  {
        *this->out << runtime_call->to_string() ;
    }
    
    void L2ToL1_GeneratorVisitor::visit(Instruction_call_user *user_call)  {
        *this->out << user_call->to_string() ;
    }
    
    void L2ToL1_GeneratorVisitor::visit(Instruction_aop *aop)  {
        *this->out << aop->to_string() ;
    }
    
    void L2ToL1_GeneratorVisitor::visit(Instruction_assignment *assignment)  {

        assert(this->out != NULL);

        std::string dst_str = assignment->dst->to_string();
        
        *this->out << dst_str;
        *this->out << " <- ";
        
        std::string src_str = "";

        if (assignment->src->itemtype == item_stack_arg) {
            ItemStackArg * stack_arg = (ItemStackArg *) assignment->src;
            
            int32_t offset = this->numlocals * QUADSIZE;
            offset += ((ItemConstant *) stack_arg->offset)->constVal;

            src_str += "mem rsp ";
            src_str += std::to_string(offset);
            
        } else {
            src_str = assignment->src->to_string();
        }
        
        *this->out << src_str << '\n';
    }
    
    void L2ToL1_GeneratorVisitor::visit(Instruction_sop *sop)  {
        *this->out << sop->to_string();
    }
    
    void L2ToL1_GeneratorVisitor::visit(Instruction_lea *lea)  {
        *this->out << lea->to_string() ;
    }

    void L2ToL1_GeneratorVisitor::visit(Instruction_goto *inst_goto)  {
        *this->out << inst_goto->to_string(); 
    }
    
    void L2ToL1_GeneratorVisitor::visit(Instruction_dec *dec)  {
         *this->out << dec->to_string(); 
    }

    void L2ToL1_GeneratorVisitor::visit(Instruction_inc *inc)  {
         *this->out << inc->to_string(); 
    }
    
    void L2ToL1_GeneratorVisitor::visit(Instruction_cjump *cjump)  {
         *this->out << cjump->to_string(); 
    }
    
    void L2ToL1_GeneratorVisitor::set_numlocals(int32_t numlocals) {
        this->numlocals = numlocals;
    }

    // CodeGenerator_L2ToL1::CodeGenerator_L2ToL1(std::string outFilename, Program * p){
    //     this->out = std::ofstream();
    //     this->out.open(outFilename);

    //     this->instGenerator = L2ToL1_GeneratorVisitor(&this->out);
    // }

    CodeGenerator_L2ToL1::CodeGenerator_L2ToL1(std::ofstream * out, Program * p){
        this->out = out;
        this->p = p;
        this->instGenerator = L2ToL1_GeneratorVisitor(this->out);
    }
    

    void CodeGenerator_L2ToL1::generate() {
        *this->out << "(";
        *this->out << this->p->entryPointLabel << '\n';
        
        for (Function *f : this->p->functions) {
            // DEBUG_OUT << "generating output for " <<  f->name << "\n";

            this->instGenerator.set_numlocals(f->locals);

            *this->out << '\t' << "(";
            *this->out <<  f->name << " " <<  f->arguments << " " << f->locals << '\n' ;
            
            for (Instruction * inst : f->instructions) {
                *this->out <<  '\t';
                inst->accept(this->instGenerator);    
            }

            *this->out << ")";
            *this->out << "\n";
        }

        *this->out << ")";
        *this->out << "\n";
    }
    
    void CodeGenerator_L2ToL1::close() {
        this->out->close();
    }

    void generate_code(Program & p){

        /* 
        * Open the output file.
        */ 
        std::ofstream outputFile;
        outputFile.open("prog.L1");

        CodeGenerator_L2ToL1 gen(
            &outputFile,
            &p
        );

        gen.generate();
        gen.close();
         
        /* 
        * Generate target code
        */ 
        //TODO
        // output_header(outputFile);
        // push_caller_save(outputFile);

        // call_entryPoint(outputFile, p);


        // pop_caller_save(outputFile);
        // output_ret(outputFile);

        // empty_lines(outputFile, 2);

        // for (auto f: p.functions) {
        //   output_function(outputFile, f);
        //   empty_lines(outputFile, 2);
        // }

        // /* 
        //  * Close the output file.
        //  */ 
        // outputFile.close();

    }


  
} 
