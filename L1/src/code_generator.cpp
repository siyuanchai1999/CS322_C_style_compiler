#include <string>
#include <iostream>
#include <fstream>

#include <code_generator.h>

#define QUADSIZE 8
#define REG_ARGS_NUM 6
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))


using namespace std;

namespace L1{
  Register_8b_type find_8b_equivalent(Register_type rtype) {
    switch(rtype) {
      case r10: return r10b;
      case r11: return r11b;
      case r13: return r13b;
      case r12: return r12b;
      case r14: return r14b;
      case r15: return r15b;
      case r8: return r8b;
      case r9: return r9b;
      case rax: return al;
      case rbp: return bpl;
      case rbx: return bl;
      case rcx: return cl;
      case rdi: return dil;
      case rdx: return dl;
      case rsi: return sil;
      default: 
        std::cerr << "Erorr register type in find_8b_equivalent:" << rtype << '\n';
        return r10b;
    }
    
  }
  void output_8bregister(std::ofstream & out, Register_8b_type rtype) {
    out << '%';
    switch(rtype) {
      case r10b : 
        out << "r10b";
        break;
      case r11b : 
        out << "r11b";
        break;
      case r12b:
        out << "r12b";
        break;
      case r13b : 
        out << "r13b";
        break;
      case r14b : 
        out << "r14b";
        break;
      case r15b:
        out << "r15b";
        break;
      case r8b : 
        out << "r8b";
        break;
      case r9b : 
        out << "r9b";
        break;
      case al:
        out << "al";
        break;
      case bpl : 
        out << "bpl";
        break;
      case bl : 
        out << "bl";
        break;
      case cl:
        out << "cl";
        break;
      case dil : 
        out << "dil";
        break;
      case dl : 
        out << "dl";
        break;
      case sil:
        out << "sil";
        break;
      default:
        std::cerr << "Erorr Register_8b_type in output_8bregister!\n";
        break;
    }
  }

  void output_register(std::ofstream & out, Register_type rtype) {
    out << '%';
    switch(rtype) {
      case rdi : 
        out << "rdi";
        break;

      case rax : 
        out << "rax";
        break; 

      case rsi : 
        out << "rsi";
        break;
      
      case rdx : 
        out << "rdx";
        break;
      
      case rcx : 
        out << "rcx";
        break;

      case r8 : 
        out << "r8";
        break;

      case r9 : 
        out << "r9";
        break;
      
      case rbx : 
        out << "rbx";
        break;
      
      case rbp : 
        out << "rbp";
        break;
      
      case r10 : 
        out << "r10";
        break;
      
      case r11 : 
        out << "r11";
        break;
      
      case r12 : 
        out << "r12";
        break;
      
      case r13 : 
        out << "r13";
        break;
      
      case r14 : 
        out << "r14";
        break;
      
      case r15 : 
        out << "r15";
        break;

      case rsp : 
        out << "rsp";
        break;
    }
  }

  void output_set_cmp(std::ofstream & out, CmpType cmptype){
    switch(cmptype) {
          case CmpType::less:
            out << "setl";
            break;

          case CmpType::leq:
            out << "setle";
            break;

          case CmpType::eq:
            out << "sete";
            break;

          case CmpType::great :
            out << "setg";
            break;

          case CmpType::geq :
            out << "setge";
            break;

          default:
            std::cerr << "Error cmp type!\n";
        }
  }

  void output_jmp_cmp(std::ofstream & out, CmpType cmptype){
    switch(cmptype) {
          case CmpType::less:
            out << "jl";
            break;

          case CmpType::leq:
            out << "jle";
            break;

          case CmpType::eq:
            out << "je";
            break;

          case CmpType::great :
            out << "jg";
            break;

          case CmpType::geq :
            out << "jge";
            break;

          default:
            std::cerr << "Error cmp type!\n";
        }
  }

  void output_constant(std::ofstream & out, int64_t val) {
    out << '$' << val;
  }

  void output_memmory_access(std::ofstream & out, Register_type rType, int64_t offset){
    out << offset;
    out << '(' ;
    output_register(out, rType);
    out << ')' ;
  }

  void output_item_labels(std::ofstream & out, std::string & labelName) {
    out << "$_";
    out << labelName.substr(1, labelName.length() - 1);
  }

  /**
   *  output jmp style label
   */
  void output_jmp_labels(std::ofstream & out, std::string & labelName) {
    out << "_";
    out << labelName.substr(1, labelName.length() - 1);
  }

  /**
   *  Wrapper of jmp instruction output
   * */
  void output_jmp_inst(std::ofstream & out, ItemLabel * itLabel) {
    out << "jmp ";
    output_jmp_labels(out, itLabel->labelName);
    out << '\n';
  }

  // void output_item_cmp(std::ofstream & out, ItemCmp * cmp) {
  //   output_item(cmp->op1)
  // }

  void empty_lines(std::ofstream & out, int count) {
    while(count-- > 0) {
      out << '\n';
    }
  }


  void output_header(std::ofstream & out){
    out << ".text\n";
    out << "  .globl go\n";
    out << "go:\n";
  }


  void push_caller_save(std::ofstream & out) {
    out << "pushq %rbx\n";
    out << "pushq %rbp\n";
    out << "pushq %r12\n";
    out << "pushq %r13\n";
    out << "pushq %r14\n";
    out << "pushq %r15\n";
  }


  void call_entryPoint(std::ofstream & out, Program & p) {
    out << "call _"; 
    out << p.entryPointLabel.substr(1, p.entryPointLabel.length() - 1);
    out << '\n';
  }

  void pop_caller_save(std::ofstream & out) {
    out << "popq %rbx\n";
    out << "popq %rbp\n";
    out << "popq %r12\n";
    out << "popq %r13\n";
    out << "popq %r14\n";
    out << "popq %r15\n";
  }

  void output_ret(std::ofstream & out) {
    out << "retq\n";
  }

  void output_label_def(std::ofstream & out, std::string & labelname){
    out << "_"; 
    out << labelname.substr(1, labelname.length() - 1);
    out << ':';
    out << '\n';
  }

  void output_item(std::ofstream & out, Item * it){
    switch (it->itemtype) {
      case ItemType::item_registers:
      {
        ItemRegister * ir = (ItemRegister *) it;
        output_register(out, ir->rType);
        break;
      }
      
      case ItemType::item_constant:
      {
        ItemConstant * ic = (ItemConstant *) it;
        // ItemConstant * ic =  static_cast<ItemConstant *>(&it);
        // std::cerr << "val = " << ic->constVal << '\n';
        output_constant(out, ic->constVal);
        break;
      }
      
      case ItemType::item_memory:
      {
        ItemMemoryAccess * im = (ItemMemoryAccess *) it;
        output_memmory_access(out, im->rType, im->offset);
        break;
      }
      
      case ItemType::item_labels:
      {
        ItemLabel * il = (ItemLabel *) it;
        output_item_labels(out, il->labelName);
        break;
      }

      case ItemType::item_cmp:
      {
        ItemCmp * il = (ItemCmp *) it;
        // output_item_labels(out, il->labelName);
        break;
      }

      default:
        break;
    }
  }

  int gen_cmp_result (int64_t val1, int64_t val2, CmpType cmptype) {
    switch (cmptype) {
      case CmpType::less :
        return val1 < val2;

      case CmpType::leq :
        return val1 <= val2;

      case CmpType::eq :
        return val1 == val2;
      
      case CmpType::great :
        return val1 > val2;

      case CmpType::geq :
        return val1 >= val2;

      default:
        std::cerr << "Error cmp type!\n";
        return 0;
    }
  }

  CmpType get_reverse_cmptype(CmpType cmptype){
    switch (cmptype) {
      case CmpType::less :
        return CmpType::great;

      case CmpType::leq :
        return CmpType::geq;

      case CmpType::eq :
        return CmpType::eq;
      
      case CmpType::great :
        return CmpType::less;

      case CmpType::geq :
        return CmpType::leq;

      default:
        std::cerr << "Error cmp type!\n";
        return CmpType::less;
    }
  }

  void output_movzbq(std::ofstream & out, Register_type rtype) {
    out << "movzbq ";
    
    output_8bregister(out, find_8b_equivalent(rtype));

    out << ',';
    out << ' ';

    output_register(out, rtype);
    out << '\n';
  }

  void output_inst_assign(std::ofstream & out, Instruction_assignment * assign){
    /**
     *  dest <- src  ==>> movq src dest
     **/
    if (assign->src->itemtype == item_cmp) {
      ItemCmp * cmp = (ItemCmp *) assign->src;
      
      if (cmp->op1->itemtype == ItemType::item_constant 
        && cmp->op2->itemtype == ItemType::item_constant
      ) {
        // std::cerr << "both constant!\n";
        // std::cerr << "dest type = " << assign->dst->itemtype << '\n';
        /**
         *     reg <- op1 {leq | eq | less} op2: op1, op2 both constant
         *  =>
         *    res = val1 {leq | eq | less} val2
         *     movq $res, reg
         */
        int64_t val1 = ((ItemConstant *) cmp->op1)->constVal;
        int64_t val2 = ((ItemConstant *) cmp->op2)->constVal;
        
        int res = gen_cmp_result(
            val1,
            val2,
            cmp->cmptype
        );
        
        out << "movq ";
        output_constant(out, res);
        out << ", ";

        output_item(out, assign->dst );

        out << "\n";
      } else{
        /**
         *  
         *    reg <- op1 {leq | eq | less} op2
         *  =>>
         *    cmpq op2, op1
         *    set{le|e|l} 8b_register_dest
         *    movzbq 8b_register_dest, dest
         * 
         *    reverse op1 and op2, and set flag when op1 is a constant
         * */

        out << "cmpq ";
        CmpType cmptype = cmp->cmptype;

        if(cmp->op1->itemtype == ItemType::item_constant) {
          output_item(out, cmp->op1);
          out << ", ";
          output_item(out, cmp->op2);
          out << "\n";
          cmptype = get_reverse_cmptype(cmptype);

        }else {
          output_item(out,cmp->op2);
          out << ", ";
          output_item(out,cmp->op1);
          out << "\n";
        }

        output_set_cmp(out, cmptype);

        out << " ";
        
        ItemRegister *reg = (ItemRegister *) assign->dst;
        output_8bregister(out, find_8b_equivalent(reg->rType));

        out << "\n";
        
        output_movzbq(out, reg->rType);
        
      }
      
      return;
    }


    out << "movq ";

    output_item(out, assign->src );
    
    out << ", ";

    output_item(out, assign->dst );

    out << "\n";
  }

  void stack_grow(std::ofstream & out, int bytes) {
    out << "subq ";
    out << '$';
    out << bytes;
    out << ", ";
    out << "%rsp\n";
  }

  void stack_shrink(std::ofstream & out, int bytes) {
    out << "addq ";
    out << '$';
    out << bytes;
    out << ", ";
    out << "%rsp\n";
  }


  void alloc_locals(std::ofstream & out, Function * function){
    int growedQuad = function->locals;
    if (growedQuad > 0) {
      stack_grow(out, growedQuad * QUADSIZE);
    }
  }

  void dealloc_locals_and_stack(std::ofstream & out, Function * function) {
    int numLocals = function->locals;
    int numArgsOnStack = MAX(function->arguments - REG_ARGS_NUM, 0);
    
    int growedQuad = numLocals + numArgsOnStack;
    if (growedQuad > 0) {
      stack_shrink(out, growedQuad * QUADSIZE);
    }
  }

  void output_runtimeCall(std::ofstream & out, Instruction_call_runtime * runtime_call) {
    out << "call ";
    if (runtime_call->callee == "tensor-error") {
      switch (runtime_call->arg_cnt)
      {
        case 1:
          out << "array_tensor_error_null";
          break;
        
        case 3:
          out << "array_error";
          break;

        case 4:
          out << "tensor_error";
          break;

        default:
          break;
      }
    } else {
      out << runtime_call->callee;
    }
    out << '\n';
  }

  void output_userCall(std::ofstream & out, Instruction_call_user * user_call) {
    int quadToGrow = 1;
    ItemConstant * c = (ItemConstant * ) user_call->num_args;
    quadToGrow += MAX(c->constVal - REG_ARGS_NUM, 0);
    
    stack_grow(out, quadToGrow * QUADSIZE);
    
    out << "jmp";
    out << ' ';

    if (user_call->callee->itemtype == ItemType::item_registers) {
      /**
       *  Indirect call
       *  jmp *%rdi
       * */
      out << '*';
      output_item(out, user_call->callee);

    } else {
      /**
       *  Direct call 
       *  jmp _callee
       * */
      ItemLabel * calleeLabel = (ItemLabel *) user_call->callee;

      output_jmp_labels(out, calleeLabel->labelName);
    }

    out << '\n';
  }

  void output_call_inst(std::ofstream & out, Instruction_call * call) {

    if (call->isRuntimeCall) {

      Instruction_call_runtime * runtime_call = (Instruction_call_runtime *) call;
      output_runtimeCall(out, runtime_call);
    
    } else {
      Instruction_call_user * user_call = (Instruction_call_user *) call;
      output_userCall(out, user_call);
      
    }
  }

  void output_aop_inst(std::ofstream & out, Instruction_aop * aop) {
    /**
     *  op1 += op2  ==>> addq op2 op1
     * */

    switch (aop->aopType)
    {
      case AopType::plus_eq :
        out << "addq";
        break;

      case AopType::minus_eq :
        out << "subq";
        break;

      case AopType::times_eq :
        out << "imulq";
        break;
      
      case AopType::bitand_eq :
        out << "andq";
        break;

      default:
        break;
    }

    out << ' ';
    output_item(out, aop->op2);
    out << ", ";

    output_item(out, aop->op1);
    out << '\n';
  }
  
  void output_sop_inst(std::ofstream & out, Instruction_sop * sop) {
    /**
     *  op1 >>= op2  ==>> sarq op2 op1
     * */

    switch (sop->direction)
    {
      case ShiftType::left :
        out << "salq";
        break;

      case ShiftType::right :
        out << "sarq";
        break;

      default:
        std::cerr << "Error! wrong ShiftType in output_sop_inst!\n";
        break;
    }

    out << ' ';
    if (sop->offset->itemtype == ItemType::item_registers ) {
      /**
       *  must be rcx
       * */
      ItemRegister * reg = (ItemRegister *) sop->offset;
      output_8bregister(out, find_8b_equivalent(reg->rType));

    } else {
      output_item(out, sop->offset);
      
    }
    
    out << ", ";
    output_item(out, sop->target);
    out << '\n';
  }

  void output_lea_inst(std::ofstream & out, Instruction_lea * lea) {
    /**
     *  rax @ rdi rsi 4  ==>> lea (%rdi, %rsi, 4), %rax
     * */
    
    out << "lea";
    out << ' ';
    
    out << '(';

      output_item(out, lea->addr);
      out << ", ";

      output_item(out, lea->multr);
      out << ", ";

      ItemConstant * itC = (ItemConstant *) lea->const_multr;
      out << itC->constVal;

    out << ')';
    out << ',';
    out << ' ';
    
    output_item(out, lea->dst);
    out << '\n';
  }
  
  void output_goto_inst(std::ofstream & out, Instruction_goto * goto_inst) {
    /**
     *  goto :label  ==>> jmp _label
     * */
    
    out << "jmp ";
    ItemLabel * gotoLable = (ItemLabel *) goto_inst->gotoLabel;
    output_jmp_labels(out, gotoLable->labelName);
    out << '\n';
  }

  void output_inc_inst(std::ofstream & out, Instruction_inc * inc) {
    /**
     *  rdi++ => inc rdi
     * */
    out << "inc ";
    ItemRegister * itR = (ItemRegister *) inc->op;
    output_register(out, itR->rType);
    out << '\n';
  }
  
  void output_dec_inst(std::ofstream & out, Instruction_dec * dec) {
    /**
     *  rdi-- => dec rdi
     * */
    out << "dec ";
    ItemRegister * itR = (ItemRegister *) dec->op;
    output_register(out, itR->rType);
    out << '\n';
  }

  void output_cjump_inst(std::ofstream & out, Instruction_cjump * cjump) {
    /**
     *  cjump rax <= rdi :yes 
     * ==>>
     *  cmpq %rdi, %rax jle _yes
     **/
    
    ItemCmp * cmp = (ItemCmp *) cjump->condition;
    
    if (cmp->op1->itemtype == ItemType::item_constant 
      && cmp->op2->itemtype == ItemType::item_constant
    ) {
      // std::cerr << "both constant!\n";
      // std::cerr << "dest type = " << cmp->dst->itemtype << '\n';
      /**
       *     reg <- op1 {leq | eq | less} op2: op1, op2 both constant
       *  =>
       *     j{le|eq|l} _label
       */
      int64_t val1 = ((ItemConstant *) cmp->op1)->constVal;
      int64_t val2 = ((ItemConstant *) cmp->op2)->constVal;
      
      int res = gen_cmp_result(
          val1,
          val2,
          cmp->cmptype
      );
      
      if (res) {
        ItemLabel * itlabel = (ItemLabel *) cjump->dst;
        output_jmp_inst(out, itlabel);
      }
    } else{
      /**
       *  
       *    cjump op1 {leq | eq | less} op2 :label
       *  =>>
       *    cmpq op2, op1
       *    set{le|e|l} 8b_register_dest
       *    movzbq 8b_register_dest, dest
       * 
       *    reverse op1 and op2, and set flag when op1 is a constant
       * */

      out << "cmpq ";
      CmpType cmptype = cmp->cmptype;

      if(cmp->op1->itemtype == ItemType::item_constant) {
        output_item(out, cmp->op1);
        out << ", ";
        output_item(out, cmp->op2);
        out << "\n";
        cmptype = get_reverse_cmptype(cmptype);

      }else {
        output_item(out,cmp->op2);
        out << ", ";
        output_item(out,cmp->op1);
        out << "\n";
      }

      output_jmp_cmp(out, cmptype);

      out << " ";
      ItemLabel * itlabel = (ItemLabel *) cjump->dst;
      output_jmp_labels(out, itlabel->labelName);

      out << "\n";
            
    }
    
    
  }



  void output_inst(std::ofstream & out, Function * function, Instruction * inst) {
    switch(inst->type) {
      case InstType::inst_ret :
      {
        /**
         *  stack shrink
         * */
        dealloc_locals_and_stack(out, function);
        output_ret(out);
        break;
      }

      case InstType::inst_assign :
      { 
        Instruction_assignment * assign = (Instruction_assignment *) inst;
        output_inst_assign(out, assign );
        break; 
      }

      case InstType::inst_call : 
      { 
        Instruction_call * call = (Instruction_call *) inst;
        output_call_inst(out, call );
        break; 
      }

      case InstType::inst_aop :
      {
        Instruction_aop * aop = (Instruction_aop *) inst;
        output_aop_inst(out, aop);
        break;
      }

      case InstType::inst_sop :
      {
        Instruction_sop * sop = (Instruction_sop *) inst;
        output_sop_inst(out, sop);
        break;
      }

      case InstType::inst_lea :
      {
        Instruction_lea * lea = (Instruction_lea *) inst;
        output_lea_inst(out, lea);
        break;
      }

      case InstType::inst_goto : 
      { 
        Instruction_goto * gt = (Instruction_goto *) inst;
        output_goto_inst(out, gt);
        break; 
      } 

      case InstType::inst_inc : 
      { 
        Instruction_inc * inc = (Instruction_inc *) inst;
        output_inc_inst(out, inc);
        break; 
      } 

      case InstType::inst_dec : 
      { 
        Instruction_dec * dec = (Instruction_dec *) inst;
        output_dec_inst(out, dec);
        break; 
      } 
      
      case InstType::inst_cjump : 
      { 
        Instruction_cjump * cjump = (Instruction_cjump *) inst;
        output_cjump_inst(out, cjump);
        break; 
      } 
      

      case InstType::inst_label : 
      { 
        Instruction_label * il = (Instruction_label *) inst;
        output_label_def(out, il->labelName);
        break; 
      } 

    }
  }

  

  void output_function(std::ofstream & out, Function * function) {
    /**
     *  output function label
     * */
    output_label_def(out, function->name);
  
    /**
     *  stack grow
     * */
    alloc_locals(out, function);

    for (auto inst: function->instructions) {
      output_inst(out, function, inst);
    }


    
  }

  void generate_code(Program p){

    /* 
     * Open the output file.
     */ 
    std::ofstream outputFile;
    outputFile.open("prog.S");
   
    /* 
     * Generate target code
     */ 
    //TODO
    output_header(outputFile);
    push_caller_save(outputFile);

    call_entryPoint(outputFile, p);


    pop_caller_save(outputFile);
    output_ret(outputFile);

    empty_lines(outputFile, 2);

    for (auto f: p.functions) {
      output_function(outputFile, f);
      empty_lines(outputFile, 2);
    }
  
    /* 
     * Close the output file.
     */ 
    outputFile.close();
   
    return ;
  }


  
} 
