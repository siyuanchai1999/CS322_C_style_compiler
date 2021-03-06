/*
 * SUGGESTION FROM THE CC TEAM:
 * double check the order of actions that are fired.
 * You can do this in (at least) two ways:
 * 1) by using gdb adding breakpoints to actions
 * 2) by adding printing statements in each action
 *
 * For 2), we suggest writing the code to make it straightforward to enable/disable all of them 
 * (e.g., assuming shouldIPrint is a global variable
 *    if (shouldIPrint) std::cerr << "MY OUTPUT" << std::endl;
 * )
 */
#include <sched.h>
#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <set>
#include <iterator>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include <stdint.h>
#include <assert.h>
#include <iostream>
#include <sstream>

#include <tao/pegtl.hpp>
#include <tao/pegtl/analyze.hpp>
#include <tao/pegtl/contrib/raw_string.hpp>

#include <L2.h>
#include <parser.h>

#define IN_MAP(map, key) (map.find(key) != map.end())
#define IN_SET(set, key) (set.find(key) != set.end())

namespace pegtl = tao::TAO_PEGTL_NAMESPACE;

using namespace pegtl;
using namespace std;

namespace L2 {

    /* 
     * Data required to parse
     */ 
    std::vector<Item *> parsed_items;

    /* 
     * Grammar rules from now on.
     */
    struct name:
    pegtl::seq<
        pegtl::plus< 
        pegtl::sor<
            pegtl::alpha,
            pegtl::one< '_' >
        >
        >,
        pegtl::star<
        pegtl::sor<
            pegtl::alpha,
            pegtl::one< '_' >,
            pegtl::digit
        >
        >
    > {};

    /* 
     * Keywords.
     */
    struct str_return : TAOCPP_PEGTL_STRING( "return" ) {};
    struct str_call : TAOCPP_PEGTL_STRING( "call" ) {};
    struct str_cjump : TAOCPP_PEGTL_STRING( "cjump" ) {};
    struct str_mem : TAOCPP_PEGTL_STRING( "mem" ) {};
    struct str_goto : TAOCPP_PEGTL_STRING( "goto" ) {};
    struct str_print : TAOCPP_PEGTL_STRING( "print" ) {};
    struct str_input : TAOCPP_PEGTL_STRING( "input" ) {};
    struct str_allocate : TAOCPP_PEGTL_STRING( "allocate" ) {};
    struct str_tensor_error: TAOCPP_PEGTL_STRING( "tensor-error" ) {};
    struct str_stack_arg : TAOCPP_PEGTL_STRING("stack-arg"){};

    //increment operator
    struct str_increment : TAOCPP_PEGTL_STRING( "++" ) {};
    struct str_decrement : TAOCPP_PEGTL_STRING( "--" ) {};
    
    // Arithmetic Operator
    struct str_plus_eq : TAOCPP_PEGTL_STRING( "+=" ) {};
    struct str_minus_eq : TAOCPP_PEGTL_STRING( "-=" ) {};
    struct str_mult_eq : TAOCPP_PEGTL_STRING( "*=" ) {};
    struct str_bitand_eq : TAOCPP_PEGTL_STRING( "&=" ) {};

    // Shift operators
    struct str_shift_left : TAOCPP_PEGTL_STRING( "<<=" ) {};
    struct str_shift_right : TAOCPP_PEGTL_STRING( ">>=" ) {};

    // Compare operators
    struct str_less : TAOCPP_PEGTL_STRING( "<" ) {};
    struct str_less_eq : TAOCPP_PEGTL_STRING( "<=" ) {};
    struct str_eq: TAOCPP_PEGTL_STRING( "=" ) {};

    // @
    struct str_lea: TAOCPP_PEGTL_STRING( "@" ) {};

    // Register keywords
    struct str_rdi : TAOCPP_PEGTL_STRING( "rdi" ) {};
    struct str_rsi : TAOCPP_PEGTL_STRING( "rsi" ) {};
    struct str_rax : TAOCPP_PEGTL_STRING( "rax" ) {};
    struct str_rdx : TAOCPP_PEGTL_STRING( "rdx" ) {};
    struct str_rcx : TAOCPP_PEGTL_STRING( "rcx" ) {};
    struct str_r8 : TAOCPP_PEGTL_STRING( "r8" ) {};
    struct str_r9 : TAOCPP_PEGTL_STRING( "r9" ) {};
    struct str_rbx : TAOCPP_PEGTL_STRING( "rbx" ) {};
    struct str_rbp : TAOCPP_PEGTL_STRING( "rbp" ) {};
    struct str_r10 : TAOCPP_PEGTL_STRING( "r10" ) {};
    struct str_r11 : TAOCPP_PEGTL_STRING( "r11" ) {};
    struct str_r12 : TAOCPP_PEGTL_STRING( "r12" ) {};
    struct str_r13 : TAOCPP_PEGTL_STRING( "r13" ) {};
    struct str_r14 : TAOCPP_PEGTL_STRING( "r14" ) {};
    struct str_r15 : TAOCPP_PEGTL_STRING( "r15" ) {};

    // stack pointer
    struct str_rsp : TAOCPP_PEGTL_STRING( "rsp" ) {};



    struct label:
    pegtl::seq<
        pegtl::one<':'>,
        name
    > {};

    struct variable:
    pegtl::seq<
        pegtl::one<'%'>,
        name
    > {};

    struct register_rdi_rule:
        str_rdi {};

    struct register_rax_rule:
        str_rax {};
    
    struct register_rsi_rule:
        str_rsi {};

    struct register_rdx_rule:
        str_rdx {};

    struct register_rcx_rule:
        str_rcx {};

    struct register_r8_rule:
        str_r8 {};
    
    struct register_r9_rule:
        str_r9 {};

    struct register_rbx_rule:
        str_rbx {};

    struct register_rbp_rule:
        str_rbp {};

    struct register_r10_rule:
        str_r10 {};
    
    struct register_r11_rule:
        str_r11 {};

    struct register_r12_rule:
        str_r12 {};

    struct register_r13_rule:
        str_r13 {};

    struct register_r14_rule:
        str_r14 {};
    
    struct register_r15_rule:
        str_r15 {};

    struct stack_ptr_rule:
        str_rsp {};

    struct variable_rule:
        variable {};

    struct var_toSpill:
        variable {};

    struct spill_replace:
        variable {};

    struct register_rule:
    pegtl::sor<
        register_rdi_rule,
        register_rsi_rule,
        register_rax_rule,
        register_rdx_rule,
        register_rcx_rule,
        register_r8_rule,
        register_r9_rule,
        register_rbx_rule,
        register_rbp_rule,
        register_r10_rule,
        register_r11_rule,
        register_r12_rule,
        register_r13_rule,
        register_r14_rule, 
        register_r15_rule
    > {};

    /**
     *  w in L2
     * */
    struct register_and_variable_rule:
    pegtl::sor<
        register_rule,
        variable_rule
    >{};

    struct all_register_rule:
    pegtl::sor<
        register_rule,
        stack_ptr_rule
    > {};
    
    /**
     *  x in L2
     * */
    struct all_register_and_variable_rule:
    pegtl::sor<
        all_register_rule,
        variable_rule
    >{};


    struct runtime_rule:
    pegtl::sor<
        str_print,
        str_input,
        str_allocate,
        str_tensor_error
        >{};


    /**
     *  N in L2
     * */
    struct number:
    pegtl::seq<
        pegtl::opt<
        pegtl::sor<
            pegtl::one< '-' >,
            pegtl::one< '+' >
        >
        >,
        pegtl::plus< 
        pegtl::digit
        >
    >{};

    
    

    struct function_name:
    label {};

    struct argument_number:
    number {};

    struct local_number:
    number {} ;

    struct constant_number:
    number {} ;

    struct cmp_constant_number:
    number {} ;

    struct lea_size_number:
    number {};

    struct str_arrow : TAOCPP_PEGTL_STRING( "<-" ) {};
    struct comment: 
    pegtl::disable< 
        TAOCPP_PEGTL_STRING( "//" ), 
        pegtl::until< pegtl::eolf > 
    > {};

    struct 
    seps: 
    pegtl::star< 
        pegtl::sor< 
        pegtl::ascii::space, 
        comment 
        > 
    > {};

    /**
     *  t ::= x | N in L2
     * */
    struct all_register_number_variable:
    pegtl::sor<
        // all_register_rule,
        all_register_and_variable_rule,
        constant_number
    > {};

    

    struct Label_rule:
    label {};

    struct memAccess_rule:
    pegtl::seq<
        str_mem,
        seps,
        all_register_and_variable_rule,
        seps,
        constant_number
    > {};

    struct stack_arg_rule:
    pegtl::seq<
        str_stack_arg,
        seps,
        constant_number
    > {};

    struct plus_eq_rule:
    pegtl::seq<
        pegtl::sor<register_and_variable_rule, memAccess_rule>,
        seps,
        str_plus_eq,
        seps,
        pegtl::sor<all_register_and_variable_rule, memAccess_rule, constant_number>
    > {};

    struct minus_eq_rule:
    pegtl::seq<
        pegtl::sor<register_and_variable_rule, memAccess_rule>,
        seps,
        str_minus_eq,
        seps,
        pegtl::sor<all_register_and_variable_rule, memAccess_rule, constant_number>
    > {};

    struct times_eq_rule:
    pegtl::seq<
        register_and_variable_rule,
        seps,
        str_mult_eq,
        seps,
        pegtl::sor<all_register_and_variable_rule, constant_number>
    > {};

    struct bitand_eq_rule:
    pegtl::seq<
        register_and_variable_rule,
        seps,
        str_bitand_eq,
        seps,
        pegtl::sor<all_register_and_variable_rule, constant_number>
    > {};

    struct cmp_less_rule:
    pegtl::seq<
        all_register_number_variable,
        seps,
        str_less,
        seps,
        all_register_number_variable
    > {};

    struct cmp_lesseq_rule:
    pegtl::seq<
        all_register_number_variable,
        // pegtl::sor<all_register_rule, constant_number>,
        seps,
        str_less_eq,
        seps,
        all_register_number_variable
        // pegtl::sor<all_register_rule, constant_number>
    > {};
    
    struct cmp_eq_rule:
    pegtl::seq<
        all_register_number_variable,
        seps,
        str_eq,
        seps,
        all_register_number_variable
    > {};

    struct shift_left_rule:
    pegtl::seq<
        register_and_variable_rule,
        seps, 
        str_shift_left,
        seps,
        pegtl::sor< 
            register_rcx_rule,
            constant_number, 
            variable_rule
        >
    > {};

    struct shift_right_rule:
    pegtl::seq<
        register_and_variable_rule,
        seps, 
        str_shift_right,
        seps,
        pegtl::sor< 
            register_rcx_rule,
            constant_number,
            variable_rule
        >
    > {};


    // Instruction Rules
    struct Instruction_return_rule:
    pegtl::seq<
        str_return
    > { };

    struct Instruction_inc_rule:
    pegtl::seq<
        register_and_variable_rule,
        seps,
        str_increment
    > {};

    struct Instruction_dec_rule:
    pegtl::seq<
        register_and_variable_rule,
        seps,
        str_decrement
    > {};

    struct Instruction_sop_rule:
    pegtl::sor< 
        pegtl::seq< pegtl::at<shift_left_rule>         , shift_left_rule             >,
        pegtl::seq< pegtl::at<shift_right_rule>        , shift_right_rule            >
    > {};
    
    struct Instruction_aop_rule: 
    pegtl::sor<
    pegtl::seq< pegtl::at<plus_eq_rule>         , plus_eq_rule             >,
    pegtl::seq< pegtl::at<minus_eq_rule>        , minus_eq_rule             >,
    pegtl::seq< pegtl::at<times_eq_rule>        , times_eq_rule             >,
    pegtl::seq< pegtl::at<bitand_eq_rule>       , bitand_eq_rule             >
    // plus_eq_rule,
    // minus_eq_rule,
    // times_eq_rule,
    // bitand_eq_rule
    > {};

    struct Instruction_goto_rule: 
    pegtl::seq<
        str_goto,
        seps,
        Label_rule
    > {};

    /* reg/var <- all_reg/var*/
    struct reg_from_allReg :
    pegtl::seq<
        register_and_variable_rule,
        seps,
        str_arrow,
        seps,
        all_register_and_variable_rule
    > {};
    
    /* reg/var <- N*/
    struct reg_from_number :
    pegtl::seq<
        register_and_variable_rule,
        seps,
        str_arrow,
        seps,
        constant_number
    > {};

    /* reg/var <- mem*/
    struct reg_from_mem :
    pegtl::seq<
        register_and_variable_rule,
        seps,
        str_arrow,
        seps,
        memAccess_rule
    > {};

    /* mem <- all_reg/var*/
    struct mem_from_allReg :
    pegtl::seq<
        memAccess_rule,
        seps,
        str_arrow,
        seps,
        all_register_and_variable_rule
    > {};

    /* mem <- constant*/
    struct mem_from_constant :
    pegtl::seq<
        memAccess_rule,
        seps,
        str_arrow,
        seps,
        constant_number
    > {};

    // mem <- :label
    struct mem_from_label :
    pegtl::seq<
        memAccess_rule,
        seps,
        str_arrow,
        seps,
        Label_rule
    > {};

    // reg/var <- :label
    struct reg_from_label :
    pegtl::seq<
        register_and_variable_rule,
        seps,
        str_arrow,
        seps,
        Label_rule
    > {};

    /* reg/var <- stack argument*/
    struct reg_from_stack : 
    pegtl::seq< 
        register_and_variable_rule,
        seps,
        str_arrow,
        seps,
        stack_arg_rule
    > {};

    struct Instruction_assignment_rule:
    pegtl::sor<
        pegtl::seq< pegtl::at<reg_from_allReg>           , reg_from_allReg             >,
        pegtl::seq< pegtl::at<reg_from_number>           , reg_from_number             >,
        pegtl::seq< pegtl::at<reg_from_mem>              , reg_from_mem                >,
        pegtl::seq< pegtl::at<mem_from_allReg>           , mem_from_allReg             >,
        pegtl::seq< pegtl::at<mem_from_constant>         , mem_from_constant           >,
        pegtl::seq< pegtl::at<reg_from_label>            , reg_from_label              >,
        pegtl::seq< pegtl::at<mem_from_label>            , mem_from_label              >,
        pegtl::seq< pegtl::at<reg_from_stack>            , reg_from_stack              >
    > {};

    struct Instruction_assignment_cmp_rule: 
    /* everyReg except rsp <- cmp*/
    pegtl::seq<
        register_and_variable_rule,
        seps,
        str_arrow,
        seps,
        pegtl::sor<
            pegtl::seq< pegtl::at<cmp_less_rule>          , cmp_less_rule             >,
            pegtl::seq< pegtl::at<cmp_lesseq_rule>        , cmp_lesseq_rule             >,
            pegtl::seq< pegtl::at<cmp_eq_rule>            , cmp_eq_rule             >
        >
    > {};

    struct Instruction_cjump_rule: 
    // conditionally jump to a label. Cannot jump to registers. 
    pegtl::seq<
        str_cjump,
        seps,
        pegtl::sor<
        pegtl::seq< pegtl::at<cmp_less_rule>          , cmp_less_rule             >,
        pegtl::seq< pegtl::at<cmp_lesseq_rule>        , cmp_lesseq_rule           >,
        pegtl::seq< pegtl::at<cmp_eq_rule>            , cmp_eq_rule               >
        >,
        seps,
        Label_rule
    > {};

    struct Instruction_lea_rule:
    pegtl::seq<
        register_and_variable_rule, 
        seps,
        str_lea,
        seps,
        register_and_variable_rule,
        seps,
        register_and_variable_rule,
        seps,
        constant_number
    > {};
    
    struct  Instruction_call_runtime_rule:
    pegtl::seq<
        str_call,
        seps,
        runtime_rule,
        seps,
        constant_number
    > {}; 

    struct  Instruction_call_user_rule:
    pegtl::seq<
        str_call,
        seps,
        pegtl::sor<Label_rule, register_and_variable_rule>,
        seps,
        constant_number
    > {}; 

    
    // struct  Instruction_label_rule:
    // Label_rule {};
    struct  Instruction_label_rule:
    pegtl::seq<
        Label_rule
    > {};


    struct Instruction_rule:
    pegtl::sor<
        pegtl::seq< pegtl::at<Instruction_return_rule>            , Instruction_return_rule             >,
        pegtl::seq< pegtl::at<Instruction_assignment_cmp_rule>    , Instruction_assignment_cmp_rule     >,
        pegtl::seq< pegtl::at<Instruction_cjump_rule>             , Instruction_cjump_rule     >,
        pegtl::seq< pegtl::at<Instruction_assignment_rule>        , Instruction_assignment_rule         >,
        pegtl::seq< pegtl::at<Instruction_call_runtime_rule>      , Instruction_call_runtime_rule       >,
        pegtl::seq< pegtl::at<Instruction_call_user_rule>         , Instruction_call_user_rule       >,
        pegtl::seq< pegtl::at<Instruction_label_rule>             , Instruction_label_rule              >,
        pegtl::seq< pegtl::at<Instruction_aop_rule>               , Instruction_aop_rule                >,
        pegtl::seq< pegtl::at<Instruction_sop_rule>               , Instruction_sop_rule                >,
        pegtl::seq< pegtl::at<Instruction_lea_rule>               , Instruction_lea_rule                >,
        pegtl::seq< pegtl::at<Instruction_goto_rule>              , Instruction_goto_rule               >,
        pegtl::seq< pegtl::at<Instruction_inc_rule>               , Instruction_inc_rule                >,
        pegtl::seq< pegtl::at<Instruction_dec_rule>               , Instruction_dec_rule                >
    > { };

    struct Instructions_rule:
    pegtl::plus<
        pegtl::seq<
        seps,
        Instruction_rule,
        seps
        >
    > { };

    struct Function_rule:
    pegtl::seq<
        seps,
        pegtl::one< '(' >,
        seps,
        function_name,
        seps,
        argument_number,
        seps,
        Instructions_rule,
        seps,
        pegtl::one< ')' >
    > {};

    struct Functions_rule:
    pegtl::plus<
        seps,
        Function_rule,
        seps
    > {};

    struct entry_point_rule:
    pegtl::seq<
        seps,
        pegtl::one< '(' >,
        seps,
        label,
        seps,
        Functions_rule,
        seps,
        pegtl::one< ')' >,
        seps
    > { };

    struct spill_file_rule :
    pegtl::seq<
        seps,
        Function_rule,
        seps,
        var_toSpill,
        seps,
        spill_replace,
        seps
    > {};
    
    struct spill_file_grammar : 
    pegtl::must< 
        spill_file_rule
    > {};

    struct function_grammar : 
    pegtl::must< 
        Function_rule
    > {};

    struct grammar : 
    pegtl::must< 
        entry_point_rule
    > {};

    /* 
     * Actions attached to grammar rules.
     */
    template< typename Rule >
    struct action : pegtl::nothing< Rule > {};

    template<> struct action < label > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        if (p.entryPointLabel.empty()){
        p.entryPointLabel = in.string();
        } else {
        abort();
        }
    }
    };

    template<> struct action < var_toSpill > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        /**
         *  variable to spill
         *  NOTE! require file only have one function
         * */
        Function *currentF = p.functions.back();
        if (IN_MAP(currentF->varName2ptr, in.string()))
        {
            p.varToSpill = (ItemVariable * ) currentF->varName2ptr[in.string()];
            // std::cerr << "after push parsed_items.size() = " << parsed_items.size() << '\n';
        } else {
            p.varToSpill = new ItemVariable(in.string());
            // std::cerr << "CANNOT find variable to spill: " << in.string() << '\n';
        }

    }
    };

    template<> struct action < spill_replace > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        p.prefix =  new ItemVariable(in.string());

    }
    };
    

    template<> struct action < function_name > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        auto newF = new Function();
        newF->name = in.string();
        p.functions.push_back(newF);
    }
    };

    template<> struct action < argument_number > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        auto currentF = p.functions.back();
        currentF->arguments = std::stoll(in.string());
    }
    };

    template<> struct action < local_number > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        auto currentF = p.functions.back();
        currentF->locals = std::stoll(in.string());
    }
    };

    template<> struct action < constant_number > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        // std::cerr << "pushing " << in.string () << '\n';

        // auto currentF = p.functions.back();
        ItemConstant *itConst = new ItemConstant;
        itConst->itemtype = item_constant;
        itConst->constVal = std::stoll(in.string());
        // std::cerr << "   val = " << itConst->constVal << '\n';
        parsed_items.push_back(itConst);
    }
    };


    template<> struct action < str_return > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        // std::cerr << "firing return!\n";
        auto currentF = p.functions.back();
        auto i = new Instruction_ret();
        i->type = inst_ret;
        currentF->instructions.push_back(i);
    }
    };

    template<> struct action < Label_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        // std::cerr << "firing label rule " << in.string() << '\n';
        Function *currentF = p.functions.back();
        if (IN_MAP(currentF->labelName2ptr, in.string()))
        {

            parsed_items.push_back(currentF->labelName2ptr[in.string()]);
            // std::cerr << "after push parsed_items.size() = " << parsed_items.size() << '\n';
            return;
        }

        ItemLabel *l = new ItemLabel(in.string());
        l->itemtype = item_labels;
        currentF->labelName2ptr[in.string()] = l;

        parsed_items.push_back(l);

        // std::cerr << "after push parsed_items.size() = " << parsed_items.size() << '\n';
    }
    };

    template<> struct action < runtime_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        // std::cerr << "pushing " << in.string () << '\n';
        // ItemLabel *l = new ItemLabel;
        // l->itemtype = item_labels;
        // l->labelName = in.string();
        // Item i;
        // i.isARegister = false;
        // i.labelName = in.string();
        // std::cerr << "parsed_items size = " << parsed_items.size() << '\n';
        // std::cerr << "l.labelName =  " << l.labelName << "at " << &l.labelName << '\n';
        
        // std::cerr << "runtime str parsed_items.size() before push= " << parsed_items.size() << '\n';

        if (in.string() == L2::print_str) {
            
            parsed_items.push_back(& L2::print_label);

        } else if (in.string() == L2::input_str) {

            parsed_items.push_back(& L2::input_label);

        } else if (in.string() == L2::allocate_str) {

            parsed_items.push_back(& L2::allocate_label);

        } else if (in.string() == L2::tensor_str) {

            parsed_items.push_back(& L2::tensor_label);
        
        } else {
            // std::cerr << "wrong runtime function in actions for runtime function call!\n";
        }
        
        // std::cerr << "runtime str parsed_items.size() = " << parsed_items.size() << '\n';
    }
    };

    template<> struct action < str_plus_eq > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
    ItemAop *aop = new ItemAop;
    aop->itemtype = item_aop;
    aop->aopType = plus_eq;
    parsed_items.push_back(aop);
    }
    };

    template<> struct action < str_minus_eq > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
    ItemAop *aop = new ItemAop;
    aop->itemtype = item_aop;
    aop->aopType = minus_eq;
    parsed_items.push_back(aop);
    }
    };

    template<> struct action < str_mult_eq > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
    ItemAop *aop = new ItemAop;
    aop->itemtype = item_aop;
    aop->aopType = times_eq;
    parsed_items.push_back(aop);
    }
    };

    template<> struct action < str_bitand_eq > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
    ItemAop *aop = new ItemAop;
    aop->itemtype = item_aop;
    aop->aopType = bitand_eq;
    parsed_items.push_back(aop);
    }
    };

    template<> struct action < register_rdi_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        parsed_items.push_back(&L2::reg_rdi);
    }
    };
 
    template<> struct action < register_rax_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        // ItemRegister r;
        // r.itemtype = item_registers;
        // r.rType = rax;
        // ItemRegister * r = new ItemRegister;
        // r->itemtype = item_registers;
        // r->rType = rax;

        parsed_items.push_back(&L2::reg_rax);
    }
    };

    template<> struct action < register_rsi_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        // ItemRegister r;
        // r.itemtype = item_registers;
        // r.rType = rsi;

        // ItemRegister * r = new ItemRegister;
        // r->itemtype = item_registers;
        // r->rType = rsi;
        
        parsed_items.push_back(&L2::reg_rsi);
    }
    };

    template<> struct action < register_rdx_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        // ItemRegister r;
        // r.itemtype = item_registers;
        // r.rType = rdx;

        // ItemRegister * r = new ItemRegister;
        // r->itemtype = item_registers;
        // r->rType = rdx;

        parsed_items.push_back(&reg_rdx);
    }
    };

    template<> struct action < register_rcx_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        // ItemRegister r;
        // r.itemtype = item_registers;
        // r.rType = rcx;

        // ItemRegister * r = new ItemRegister;
        // r->itemtype = item_registers;
        // r->rType = rcx;

        parsed_items.push_back(&reg_rcx);
    }
    };

    template<> struct action < register_r8_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        // ItemRegister r;
        // r.itemtype = item_registers;
        // r.rType = rcx;

        // ItemRegister * r = new ItemRegister;
        // r->itemtype = item_registers;
        // r->rType = r8;

        parsed_items.push_back(&reg_r8);
    }
    };

    template<> struct action < register_r9_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        // ItemRegister r;
        // r.itemtype = item_registers;
        // r.rType = rcx;

        // ItemRegister * r = new ItemRegister;
        // r->itemtype = item_registers;
        // r->rType = r9;

        parsed_items.push_back(&reg_r9);
    }
    };

    template<> struct action < register_rbx_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        // ItemRegister r;
        // r.itemtype = item_registers;
        // r.rType = rbx;

        // ItemRegister * r = new ItemRegister;
        // r->itemtype = item_registers;
        // r->rType = rbx;
        parsed_items.push_back(&reg_rbx);
    }
    };

    template<> struct action < register_rbp_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        // ItemRegister r;
        // r.itemtype = item_registers;
        // r.rType = rbp;
        // ItemRegister * r = new ItemRegister;
        // r->itemtype = item_registers;
        // r->rType = rbp;
        parsed_items.push_back(&reg_rbp);
    }
    };

    template<> struct action < register_r10_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        // ItemRegister r;
        // r.itemtype = item_registers;
        // r.rType = r10;

        // ItemRegister * r = new ItemRegister;
        // r->itemtype = item_registers;
        // r->rType = r10;
        parsed_items.push_back(&reg_r10);
    }
    };

    template<> struct action < register_r11_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        // ItemRegister r;
        // r.itemtype = item_registers;
        // r.rType = r11;

        // ItemRegister * r = new ItemRegister;
        // r->itemtype = item_registers;
        // r->rType = r11;

        parsed_items.push_back(&reg_r11);
    }
    };

    template<> struct action < register_r12_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        // ItemRegister r;
        // r.itemtype = item_registers;
        // r.rType = r12;

        // ItemRegister * r = new ItemRegister;
        // r->itemtype = item_registers;
        // r->rType = r12;

        parsed_items.push_back(&reg_r12);
    }
    };

    template<> struct action < register_r13_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        // ItemRegister r;
        // r.itemtype = item_registers;
        // r.rType = r13;

        // ItemRegister * r = new ItemRegister;
        // r->itemtype = item_registers;
        // r->rType = r13;

        parsed_items.push_back(&reg_r13);
    }
    };

    template<> struct action < register_r14_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        // ItemRegister r;
        // r.itemtype = item_registers;
        // r.rType = r14;

        // ItemRegister * r = new ItemRegister;
        // r->itemtype = item_registers;
        // r->rType = r14;

        parsed_items.push_back(&reg_r14);
    }
    };

    template<> struct action < register_r15_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        // ItemRegister r;
        // r.itemtype = item_registers;
        // r.rType = r15;

        // ItemRegister * r = new ItemRegister;
        // r->itemtype = item_registers;
        // r->rType = r15;

        parsed_items.push_back(&reg_r15);
    }
    };
    

    template<> struct action < stack_ptr_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        // ItemRegister r;
        // r.itemtype = item_registers;
        // r.rType = r15;

        // ItemRegister * r = new ItemRegister;
        // r->itemtype = item_registers;
        // r->rType = rsp;

        parsed_items.push_back(&reg_rsp);
    }
    };

    template <>
    struct action<variable_rule>
    {
        template <typename Input>
        static void apply(const Input &in, Program &p)
        {
            // std::cerr << "firing " << in.string() << '\n';
            Function *currentF = p.functions.back();
            if (IN_MAP(currentF->varName2ptr, in.string()))
            {

                parsed_items.push_back(currentF->varName2ptr[in.string()]);
                // std::cerr << "after push parsed_items.size() = " << parsed_items.size() << '\n';
                return;
            }

            ItemVariable *v = new ItemVariable(in.string());
            v->itemtype = item_variable;
            v->name = in.string();
            currentF->varName2ptr[in.string()] = v;

            parsed_items.push_back(v);
            // std::cerr << "after push parsed_items.size() = " << parsed_items.size() << '\n';
        }
    };

    template<> struct action < stack_arg_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        // ItemRegister r;
        // r.itemtype = item_registers;
        // r.rType = r15;

        ItemStackArg *stack_arg = new ItemStackArg;
        
        stack_arg->itemtype = item_stack_arg;
        stack_arg->offset = parsed_items.back();

        parsed_items.pop_back();

        parsed_items.push_back(stack_arg);
    }
    };

    template<> struct action < memAccess_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        // ItemRegister r;
        // r.itemtype = item_registers;
        // r.rType = r15;

        ItemMemoryAccess *m = new ItemMemoryAccess;
        
        m->itemtype = item_memory;

        m->offset = parsed_items.back();
        parsed_items.pop_back();

        m->reg = parsed_items.back();
        parsed_items.pop_back();

        

        // ItemConstant *c = (ItemConstant * ) parsed_items.back();
        // m->offset = c->constVal;
        // delete c;
        // parsed_items.pop_back();


        // ItemRegister *r = (ItemRegister * ) parsed_items.back();
        // m->rType = r->rType;
        // delete r;
        // parsed_items.pop_back();

        parsed_items.push_back(m);
    }
    };

     template<> struct action < shift_left_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        // std::cerr << in.string()  << '\n';
        /* 
         * Fetch the current function.
         */ 
        auto currentF = p.functions.back();

        /* 
         * Create the instruction.
         */ 
        auto i = new Instruction_sop();
        i->type = inst_sop;

        i->direction = ShiftType::left;

        i->offset = parsed_items.back();
        parsed_items.pop_back();
        
        i->target = parsed_items.back();
        parsed_items.pop_back();

        /* 
         * Add the just-created instruction to the current function.
         */ 
        currentF->instructions.push_back(i);
    }
    };

     template<> struct action < shift_right_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        // std::cerr << in.string()  << '\n';
        /* 
         * Fetch the current function.
         */ 
        auto currentF = p.functions.back();

        /* 
         * Create the instruction.
         */ 
        auto i = new Instruction_sop();
        i->type = inst_sop;
        i->direction = ShiftType::right;

        i->offset = parsed_items.back();
        parsed_items.pop_back();
        
        i->target = parsed_items.back();
        parsed_items.pop_back();

        /* 
         * Add the just-created instruction to the current function.
         */ 
        currentF->instructions.push_back(i);
    }
    };

    template<> struct action < Instruction_lea_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        // std::cerr << in.string()  << '\n';
        /* 
         * Fetch the current function.
         */ 
        auto currentF = p.functions.back();

        /* 
         * Create the instruction.
         */ 
        auto i = new Instruction_lea();
        i->type = inst_lea;
        
        i->const_multr = parsed_items.back();
        parsed_items.pop_back();

        i->multr = parsed_items.back();
        parsed_items.pop_back();
        
        i->addr = parsed_items.back();
        parsed_items.pop_back();

        i->dst = parsed_items.back();
        parsed_items.pop_back();

        /* 
         * Add the just-created instruction to the current function.
         */ 
        currentF->instructions.push_back(i);
    }
    };


    template<> struct action < cmp_lesseq_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        // std::cerr << "firing" << in.string()  << '\n';
        /* 
         * Fetch the current function.
         */ 
        auto currentF = p.functions.back();

        // std::cerr << "before pop parsed_items.size() = " << parsed_items.size() << '\n';
        /* 
         * Create the instruction.
         */ 
        ItemCmp * itCmp = new ItemCmp();
        itCmp->itemtype = item_cmp;
        itCmp->cmptype = CmpType::leq;

        itCmp->op2 = parsed_items.back();
        parsed_items.pop_back();

        itCmp->op1 = parsed_items.back();
        parsed_items.pop_back();
        
        parsed_items.push_back(itCmp);

        // std::cerr << "after push parsed_items.size() = " << parsed_items.size() << '\n';
    }
    };

    template<> struct action < cmp_less_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        // std::cerr << in.string()  << '\n';
        /* 
         * Fetch the current function.
         */ 
        auto currentF = p.functions.back();

        /* 
         * Create the instruction.
         */ 
        ItemCmp * itCmp = new ItemCmp();
        itCmp->itemtype = item_cmp;
        itCmp->cmptype = CmpType::less;
        
        itCmp->op2 = parsed_items.back();
        parsed_items.pop_back();
        
        itCmp->op1 = parsed_items.back();
        parsed_items.pop_back();
        

        /* 
         * Add the just-created instruction to the current function.
         */ 
        parsed_items.push_back(itCmp);
    }
    };

    template<> struct action < cmp_eq_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        // std::cerr << in.string()  << " in cmp_eq_rule" <<  '\n';
        /* 
         * Fetch the current function.
         */ 
        auto currentF = p.functions.back();

        /* 
         * Create the instruction.
         */ 
        ItemCmp * itCmp = new ItemCmp();
        itCmp->itemtype = item_cmp;
        itCmp->cmptype = CmpType::eq;
        
        itCmp->op2 = parsed_items.back();
        parsed_items.pop_back();
        
        itCmp->op1 = parsed_items.back();
        parsed_items.pop_back();
        

        /* 
         * Add the just-created instruction to the current function.
         */ 
        parsed_items.push_back(itCmp);
    }
    };


    template<> struct action < Instruction_aop_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        // std::cerr << in.string()  << '\n';
        /* 
         * Fetch the current function.
         */ 
        auto currentF = p.functions.back();

        /* 
         * Create the instruction.
         */ 
        auto i = new Instruction_aop();
        i->type = inst_aop;

        i->op2 = parsed_items.back();
        parsed_items.pop_back();
        
        ItemAop *c = (ItemAop *) parsed_items.back();
        i->aopType = c->aopType;
        parsed_items.pop_back();

        i->op1 = parsed_items.back();
        parsed_items.pop_back();

        /* 
         * Add the just-created instruction to the current function.
         */ 
        currentF->instructions.push_back(i);
    }
    };
    
     template<> struct action < Instruction_label_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        // std::cerr << "firing instruction label rule"<< in.string()  << '\n';
        // std::cerr << "before pull parsed_items.size() = " << parsed_items.size() << '\n';
        /* 
         * Fetch the current function.
         */ 
        auto currentF = p.functions.back();

        /* 
         * Create the instruction.
         */ 
        Instruction_label * il = new Instruction_label();
        il->type = InstType::inst_label;
        il->item_label = parsed_items.back();
        parsed_items.pop_back();
        /* 
         * Add the just-created instruction to the current function.
         */ 
        currentF->instructions.push_back(il);
    }
    };

    template<> struct action < Instruction_call_user_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        // std::cerr << in.string() << '\n';
        /* 
         * Fetch the current function.
         */ 
        auto currentF = p.functions.back();

        
        /* 
         * Create the instruction.
         */ 
        Instruction_call_user * usercall = new Instruction_call_user();
        usercall->type = inst_call;
        usercall->isRuntimeCall = false;
        
        /**
         * Parse the input with iss 
         * */

        /**
         * callee and num_locals
         *  callee can be either register or label
         * */
        usercall->num_args =  parsed_items.back();
        parsed_items.pop_back();
        
        usercall->callee = parsed_items.back();
        parsed_items.pop_back();
        
        /* 
         * Add the just-created instruction to the current function.
         */ 
        currentF->instructions.push_back(usercall);
        // std::cerr << "done parsing\n" ;
    }
    };
    

    template<> struct action < Instruction_call_runtime_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        // std::cerr << "fire runtime call "<< in.string() << '\n';
        /* 
         * Fetch the current function.
         */ 
        auto currentF = p.functions.back();

        
        /* 
         * Create the instruction.
         */ 
        Instruction_call_runtime * libcall = new Instruction_call_runtime();
        libcall->type = inst_call;
        libcall->isRuntimeCall = true;

        /**
         * Parse the input parsed_items 
         * */

        // std::cerr << "runtime call parsed_items.size() = " << parsed_items.size() << '\n';
        // ItemConstant *c = (ItemConstant * ) parsed_items.back();
        // libcall->arg_cnt = c->constVal;
        // delete c;
        // parsed_items.pop_back();
        
        // // std::cerr << "parsed_items size = " << parsed_items.size() << '\n';

        // ItemLabel * l = (ItemLabel * ) parsed_items.back();
        // // std::cerr << "type = " << l->itemtype << '\n';
        // // std::cerr << " at " << &l->labelName << "name = " << l->labelName  << '\n';
        // libcall->callee = l->labelName;
        // delete l;
        // parsed_items.pop_back();

        libcall->num_args =  parsed_items.back();
        parsed_items.pop_back();
        libcall->runtime_callee =  parsed_items.back();
        parsed_items.pop_back();


        // std::cerr << "runtime call parsed_items.size() = " << parsed_items.size() << '\n';
        /* 
         * Add the just-created instruction to the current function.
         */ 
        currentF->instructions.push_back(libcall);
        // std::cerr << "done parsing\n" ;
    }
    };

    template<> struct action < Instruction_assignment_cmp_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        // std::cerr << in.string()  << " in Instruction_assignment_cmp_rule" <<  '\n';
        /* 
         * Fetch the current function.
         */ 
        auto currentF = p.functions.back();

        /* 
         * Create the instruction.
         */ 
        auto i = new Instruction_assignment();
        i->type = inst_assign;

        // std::cerr << "parsed_items.size = " << parsed_items.size() << '\n';
        i->src = parsed_items.back();
        // std::cerr << "src.type = " << i->src->itemtype << '\n';

        parsed_items.pop_back();

        // if (i->src.itemtype == item_constant){
        //   ItemConstant * ic = (ItemConstant *) &i->src;
        //   std::cerr << "val = " << ic->constVal << '\n';
        // }
        // std::cerr << "parsed_items.size = " << parsed_items.size() << '\n';
        i->dst = parsed_items.back();

        // std::cerr << "dest.type = " << i->dst->itemtype << '\n';
        parsed_items.pop_back();

        /* 
         * Add the just-created instruction to the current function.
         */ 
        currentF->instructions.push_back(i);
    }
    };

    template<> struct action < Instruction_cjump_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        // std::cerr << "firing cjump " << in.string()  << '\n';
        /* 
         * Fetch the current function.
         */ 
        auto currentF = p.functions.back();

        // std::cerr << "cjump str parsed_items.size() = " << parsed_items.size() << '\n';
        /* 
         * Create the instruction.
         */ 
        auto i = new Instruction_cjump();
        i->type = inst_cjump;

        i->dst = parsed_items.back();
        parsed_items.pop_back();

        i->condition = parsed_items.back();
        parsed_items.pop_back();

        /* 
         * Add the just-created instruction to the current function.
         */ 
        currentF->instructions.push_back(i);
    }
    };

    

    template<> struct action < Instruction_assignment_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        // std::cerr << "firing " << in.string()  << "in Instruction_assignment_rule" << '\n';
        /* 
         * Fetch the current function.
         */ 
        auto currentF = p.functions.back();

        /* 
         * Create the instruction.
         */ 
        auto i = new Instruction_assignment();
        i->type = inst_assign;

        i->src = parsed_items.back();
        parsed_items.pop_back();

        // if (i->src.itemtype == item_constant){
        //   ItemConstant * ic = (ItemConstant *) &i->src;
        //   std::cerr << "val = " << ic->constVal << '\n';
        // }

        i->dst = parsed_items.back();
        parsed_items.pop_back();

        /* 
         * Add the just-created instruction to the current function.
         */ 
        currentF->instructions.push_back(i);
    }
    };

    template <>
    struct action<Instruction_goto_rule>
    {
    template <typename Input>
    static void apply(const Input &in, Program &p)
    {
        // std::cerr << in.string()  << "in Instruction_assignment_rule" << '\n';
        /* 
         * Fetch the current function.
         */
        auto currentF = p.functions.back();

        /* 
         * Create the instruction.
         */ 
        auto i = new Instruction_goto();
        i->type = InstType::inst_goto;
        i->gotoLabel = parsed_items.back();
        parsed_items.pop_back();

        /* 
         * Add the just-created instruction to the current function.
         */ 
        currentF->instructions.push_back(i);
    }
    };

    template<> struct action < Instruction_inc_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        // std::cerr << in.string()  << "in Instruction_assignment_rule" << '\n';
        /* 
         * Fetch the current function.
         */ 
        auto currentF = p.functions.back();

        /* 
         * Create the instruction.
         */ 
        auto i = new Instruction_inc();
        i->type = inst_inc;

        i->op = parsed_items.back();
        parsed_items.pop_back();

        /* 
         * Add the just-created instruction to the current function.
         */ 
        currentF->instructions.push_back(i);
    }
    };

    template<> struct action < Instruction_dec_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        // std::cerr << in.string()  << "in Instruction_assignment_rule" << '\n';
        /* 
         * Fetch the current function.
         */ 
        auto currentF = p.functions.back();

        /* 
         * Create the instruction.
         */ 
        auto i = new Instruction_dec();
        i->type = inst_dec;
        i->op = parsed_items.back();
        parsed_items.pop_back();

        /* 
         * Add the just-created instruction to the current function.
         */ 
        currentF->instructions.push_back(i);
    }
    };

    Program parse_file (char *fileName){

    /* 
     * Check the grammar for some possible issues.
     */
    pegtl::analyze< grammar >();

    /*
     * Parse.
     */   
    file_input< > fileInput(fileName);
    Program p;
    parse< grammar, action >(fileInput, p);

    return p;
    }

    Program parse_function_file (char *fileName){
        // std::cerr << "parse function file" << fileName << '\n';
        /* 
        * Check the grammar for some possible issues.
        */
        pegtl::analyze< function_grammar >();

        /*
        * Parse.
        */   
        file_input< > fileInput(fileName);
        Program p;
        parse< function_grammar, action >(fileInput, p);

        return p;
    }

    Program parse_spill_file(char *fileName) {
        pegtl::analyze< spill_file_grammar >();

        /*
        * Parse.
        */   
        file_input< > fileInput(fileName);
        Program p;
        parse< spill_file_grammar, action >(fileInput, p);

        return p;

    }

}
