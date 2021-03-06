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
#include <deque>

#include <tao/pegtl.hpp>
#include <tao/pegtl/analyze.hpp>
#include <tao/pegtl/contrib/raw_string.hpp>

#include "L3.h"
#include "parser.h"

#define IN_MAP(map, key) (map.find(key) != map.end())
#define IN_SET(set, key) (set.find(key) != set.end())

// #define PARSER_DEBUG

#ifdef PARSER_DEBUG
#define DEBUG_OUT (std::cerr << "DEBUG-L3-parser: ") // or any other ostream
#else
#define DEBUG_OUT 0 && std::cerr
#endif


namespace pegtl = tao::TAO_PEGTL_NAMESPACE;

using namespace pegtl;
using namespace std;

namespace L3 {

    /* 
     * Data required to parse
     */ 
    std::deque<Item *> parsed_items;

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

    struct str_arrow : TAOCPP_PEGTL_STRING( "<-" ) {};
        /**
         * useless str_right_arrow to appease crazy vscode highlighting
         * */
    struct str_right_arrow : TAOCPP_PEGTL_STRING( "->" ) {};

    
    struct str_return : TAOCPP_PEGTL_STRING( "return" ) {};
    struct str_call : TAOCPP_PEGTL_STRING( "call" ) {};
    // struct str_cjump : TAOCPP_PEGTL_STRING( "cjump" ) {};
    // struct str_mem : TAOCPP_PEGTL_STRING( "mem" ) {};
    // struct str_goto : TAOCPP_PEGTL_STRING( "goto" ) {};
    struct str_print : TAOCPP_PEGTL_STRING( "print" ) {};
    struct str_input : TAOCPP_PEGTL_STRING( "input" ) {};
    struct str_allocate : TAOCPP_PEGTL_STRING( "allocate" ) {};
    struct str_tensor_error: TAOCPP_PEGTL_STRING( "tensor-error" ) {};
    // struct str_stack_arg : TAOCPP_PEGTL_STRING("stack-arg"){};
    struct str_load : TAOCPP_PEGTL_STRING( "load" ) {};
    struct str_store: TAOCPP_PEGTL_STRING( "store" ) {};
    struct str_branch: TAOCPP_PEGTL_STRING( "br" ) {};
    struct str_define: TAOCPP_PEGTL_STRING( "define" ) {};
    
    // Arithmetic Operator
    struct str_plus : TAOCPP_PEGTL_STRING( "+" ) {};
    struct str_minus : TAOCPP_PEGTL_STRING( "-" ) {};
    struct str_mult : TAOCPP_PEGTL_STRING( "*" ) {};
    struct str_bitand : TAOCPP_PEGTL_STRING( "&" ) {};
    struct str_shift_left : TAOCPP_PEGTL_STRING( "<<" ) {};
    struct str_shift_right : TAOCPP_PEGTL_STRING( ">>" ) {};

    // Compare operators
    struct str_less : TAOCPP_PEGTL_STRING( "<" ) {};
    struct str_less_eq : TAOCPP_PEGTL_STRING( "<=" ) {};
    struct str_eq: TAOCPP_PEGTL_STRING( "=" ) {};
    struct str_great : TAOCPP_PEGTL_STRING( ">" ) {};
    struct str_geq: TAOCPP_PEGTL_STRING( ">=" ) {};

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

    struct comment: 
    pegtl::disable< 
        TAOCPP_PEGTL_STRING( "//" ), 
        pegtl::until< pegtl::eolf > 
    > {};

    struct seps: 
    pegtl::star< 
        pegtl::sor< 
        pegtl::ascii::space, 
        comment 
        > 
    > {};


    struct variable_rule:
        variable {};

    struct variable_func_rule:
        variable {};

    

    struct func_variables_rule:
    /**
     * match zero time for no argument
     * match one time for one or more arguments
     * */
    pegtl::opt<
        pegtl::seq<
            seps,
            variable_func_rule,
            seps,
            /**
             *  star matches zero or more times
             * */
            pegtl::star<
                pegtl::seq<
                    seps,
                    pegtl::one<','>,
                    seps,
                    variable_func_rule,
                    seps
                >
            >
        >   
    >
     {};
    
    

    struct runtime_rule:
    pegtl::sor<
        str_print,
        str_input,
        str_allocate,
        str_tensor_error
        >{};


    /**
     *  N in L3
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

    struct constant_number:
    number {} ;


    
    

    /**
     *  t ::= var | N in L3
     * */
    struct variable_or_number_rule:
    pegtl::sor<
        variable_rule,
        constant_number
    > {};
    

    struct Label_rule:
    label {};

    /**
     *  u ::= var | label in L3
     * */
    struct variable_or_label_rule:
    pegtl::sor<
        variable_rule,
        Label_rule
    > {};

    /**
     *  s ::= t | label in L3
     * */
    struct variable_or_label_or_number_rule:
    pegtl::sor<
        variable_rule,
        Label_rule,
        constant_number
        
    > {};

    // struct variable_args_rule:
    //     variable {};
    
    // struct constant_number_args:
    // number {} ;

    // struct args_variable_or_number_rule:
    // pegtl::sor<
    //     variable_args_rule,
    //     constant_number_args
    // > {};


    /**
     *  args ::= | t | t (, t)* in L3
     * */
    struct arguments_rule:
    pegtl::opt<
        pegtl::seq<
            seps,
            variable_or_number_rule,
            seps,
            /**
             *  star matches zero or more times
             * */
            pegtl::star<
                pegtl::seq<
                    seps,
                    pegtl::one<','>,
                    seps,
                    variable_or_number_rule,
                    seps
                >
            >
        >
    > {};

    // struct memAccess_rule:
    // pegtl::seq<
    //     str_mem,
    //     seps,
    //     all_register_and_variable_rule,
    //     seps,
    //     constant_number
    // > {};

    // struct stack_arg_rule:
    // pegtl::seq<
    //     str_stack_arg,
    //     seps,
    //     constant_number
    // > {};

    struct plus_rule:
    pegtl::seq<
        variable_or_number_rule,
        seps,
        str_plus,
        seps,
        variable_or_number_rule
    > {};

    struct minus_rule:
    pegtl::seq<
        variable_or_number_rule,
        seps,
        str_minus,
        seps,
        variable_or_number_rule
    > {};

    struct times_rule:
    pegtl::seq<
        variable_or_number_rule,
        seps,
        str_mult,
        seps,
        variable_or_number_rule
    > {};

    struct bitand_rule:
    pegtl::seq<
        variable_or_number_rule,
        seps,
        str_bitand,
        seps,
       variable_or_number_rule
    > {};

    struct cmp_less_rule:
    pegtl::seq<
        variable_or_number_rule,
        seps,
        str_less,
        seps,
       variable_or_number_rule
    > {};

    struct cmp_lesseq_rule:
    pegtl::seq<
        variable_or_number_rule,
        // pegtl::sor<all_register_rule, constant_number>,
        seps,
        str_less_eq,
        seps,
        variable_or_number_rule
        // pegtl::sor<all_register_rule, constant_number>
    > {};
    
    struct cmp_great_rule:
    pegtl::seq<
        variable_or_number_rule,
        seps,
        str_great,
        seps,
        variable_or_number_rule
    > {};

    struct cmp_greateq_rule:
    pegtl::seq<
        variable_or_number_rule,
        // pegtl::sor<all_register_rule, constant_number>,
        seps,
        str_geq,
        seps,
        variable_or_number_rule
        // pegtl::sor<all_register_rule, constant_number>
    > {};

    struct cmp_eq_rule:
    pegtl::seq<
        variable_or_number_rule,
        seps,
        str_eq,
        seps,
        variable_or_number_rule
    > {};

    struct shift_left_rule:
    pegtl::seq<
        variable_or_number_rule,
        seps, 
        str_shift_left,
        seps,
        variable_or_number_rule
    > {};

    struct shift_right_rule:
    pegtl::seq<
        variable_or_number_rule,
        seps, 
        str_shift_right,
        seps,
        variable_or_number_rule
    > {};

    struct load_rule:
    pegtl::seq<
        str_load,
        seps,
        variable_rule
    > {};

    struct store_rule:
    pegtl::seq<
        str_store,
        seps, 
        variable_rule
    > {};

    /**
     *  Instruction rule from now on
     * */

    struct Instruction_return_void_rule:
    pegtl::seq<
        str_return
    > { };

    struct Instruction_return_value_rule:
    pegtl::seq<
        str_return,
        seps,
        variable_or_number_rule
    > {};

    struct Instruction_return_rule:
    pegtl::sor<
        pegtl::seq< pegtl::at<Instruction_return_value_rule>        , Instruction_return_value_rule            >,
        pegtl::seq< pegtl::at<Instruction_return_void_rule>         , Instruction_return_void_rule             >
    > {};
    
    struct aop_rule: 
    pegtl::sor<
        pegtl::seq< pegtl::at<plus_rule>         ,   plus_rule              >,
        pegtl::seq< pegtl::at<minus_rule>        ,   minus_rule             >,
        pegtl::seq< pegtl::at<times_rule>        ,   times_rule             >,
        pegtl::seq< pegtl::at<bitand_rule>       ,   bitand_rule            >,
        pegtl::seq< pegtl::at<shift_left_rule>   ,   shift_left_rule           >,
        pegtl::seq< pegtl::at<shift_right_rule>  ,   shift_right_rule          >
    > {};

    struct cmp_rule: 
    pegtl::sor<
        pegtl::seq< pegtl::at<cmp_less_rule>          , cmp_less_rule           >,
        pegtl::seq< pegtl::at<cmp_lesseq_rule>        , cmp_lesseq_rule         >,
        pegtl::seq< pegtl::at<cmp_eq_rule>            , cmp_eq_rule             >, 
        pegtl::seq< pegtl::at<cmp_great_rule>         , cmp_great_rule          >,
        pegtl::seq< pegtl::at<cmp_greateq_rule>       , cmp_greateq_rule        >
    > {};

    /* var <- var/label/N*/
    struct var_from_allSrc :
    pegtl::seq<
        variable_rule,
        seps,
        str_arrow,
        seps,
        variable_or_label_or_number_rule
    > {};

    /* var <- t aop t*/
    struct var_from_aop :
    pegtl::seq<
        variable_rule,
        seps,
        str_arrow,
        seps,
        aop_rule
    > {};
    

    /* var <- t cmp t*/
    struct var_from_cmp :
    pegtl::seq<
        variable_rule,
        seps,
        str_arrow,
        seps,
        cmp_rule
    > {};

    /* var <- load var*/
    struct var_from_load :
    pegtl::seq<
        variable_rule,
        seps,
        str_arrow,
        seps,
        load_rule
    > {};

    /* store var <- s*/
    struct store_from_allSrc :
    pegtl::seq<
        store_rule,
        seps,
        str_arrow,
        seps,
        variable_or_label_or_number_rule
    > {}; 

    struct Instruction_assignment_rule:
    pegtl::sor<
        pegtl::seq< pegtl::at<var_from_aop>            , var_from_aop             >,
        pegtl::seq< pegtl::at<var_from_cmp>            , var_from_cmp             >,
        pegtl::seq< pegtl::at<var_from_load>           , var_from_load            >,
        pegtl::seq< pegtl::at<store_from_allSrc>       , store_from_allSrc        >,
        pegtl::seq< pegtl::at<var_from_allSrc>         , var_from_allSrc          >
    > {};
    
    

    struct Instruction_conditional_branch_rule: 
    // conditionally jump to a label
    pegtl::seq<
        str_branch,
        seps,
        variable_or_number_rule,
        seps,
        Label_rule
    > {};

    struct Instruction_unconditional_branch_rule: 
    // conditionally jump to a label
    pegtl::seq<
        str_branch,
        seps,
        Label_rule
    > {};

    
    struct  call_runtime_rule:
    pegtl::seq<
        str_call, 
        seps,
        runtime_rule,  // <- fire runtime_rule push label onto the stack
        seps,
        pegtl::one< '(' >,
        seps,
        arguments_rule, // <- fire var/number push var/number onto the stack
        seps,
        pegtl::one< ')' >,
        seps   
    > {};            

    struct  call_user_rule:
    pegtl::seq<
        str_call,
        seps,
        variable_or_label_rule,
        seps,
        pegtl::one< '(' >,
        seps,
        arguments_rule,
        seps,
        pegtl::one< ')' >,
        seps
    > {}; 

    struct  Instruction_call_void_rule:  // <- fire! create new instructions and call wrapper
    pegtl::seq<
        pegtl::sor<call_runtime_rule, call_user_rule>
    > {}; 

    struct  Instruction_call_withret_rule: // <- fire! create new instructions and call wrapper
    pegtl::seq<
        variable_rule,
        seps,
        str_arrow,
        seps,
        pegtl::sor<call_runtime_rule, call_user_rule>
    > {}; 
    
    // struct  Instruction_label_rule:
    // Label_rule {};
    struct  Instruction_label_rule:
    pegtl::seq<
        Label_rule
    > {};


    struct Instruction_rule:
    pegtl::sor<
        pegtl::seq< pegtl::at<Instruction_return_rule>                  , Instruction_return_rule               >,
        pegtl::seq< pegtl::at<Instruction_assignment_rule>              , Instruction_assignment_rule           >,
        pegtl::seq< pegtl::at<Instruction_call_void_rule>               , Instruction_call_void_rule            >,
        pegtl::seq< pegtl::at<Instruction_call_withret_rule>            , Instruction_call_withret_rule         >,
        pegtl::seq< pegtl::at<Instruction_label_rule>                   , Instruction_label_rule                >,
        pegtl::seq< pegtl::at<Instruction_conditional_branch_rule>      , Instruction_conditional_branch_rule   >,    
        pegtl::seq< pegtl::at<Instruction_unconditional_branch_rule>    , Instruction_unconditional_branch_rule > 
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
        str_define,
        seps,
        function_name,
        seps,
        pegtl::one< '(' >,
        seps,
        func_variables_rule,
        seps,
        pegtl::one< ')' >,
        seps,
        pegtl::one< '{' >,
        seps,
        Instructions_rule,
        seps,
        pegtl::one< '}' >
    > {};

    struct Functions_rule:
    pegtl::plus<
        seps,
        Function_rule,
        seps
    > {};

    // struct entry_point_rule:
    // pegtl::seq<
    //     seps,
    //     pegtl::one< '(' >,
    //     seps,
    //     label,
    //     seps,
    //     Functions_rule,
    //     seps,
    //     pegtl::one< ')' >,
    //     seps
    // > { };


    // struct function_grammar : 
    // pegtl::must< 
    //     Function_rule
    // > {};

    struct grammar : 
    pegtl::must< 
        Functions_rule
    > {};

    /* 
     * Actions attached to grammar rules.
     */
    template< typename Rule >
    struct action : pegtl::nothing< Rule > {};

    // template<> struct action < label > {
    // template< typename Input >
    // static void apply( const Input & in, Program & p){
    //     if (p.entryPointLabel.empty()){
    //     p.entryPointLabel = in.string();
    //     } else {
    //     abort();
    //     }
    // }
    // };

    

    template<> struct action < function_name > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        auto newF = new Function();
        newF->name = new ItemLabel(in.string());
        p.functions.push_back(newF);

        if (in.string() == ":main") {
            p.mainF = newF;
        }

        // newF->labelName2ptr[in.string()] = newF->name;
        // newF->labels.insert(newF->name);
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
        // currentF->labels.insert(l);
        
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

        if (in.string() == L3::print_str) {
            
            parsed_items.push_back(& L3::print_label);

        } else if (in.string() == L3::input_str) {

            parsed_items.push_back(& L3::input_label);

        } else if (in.string() == L3::allocate_str) {

            parsed_items.push_back(& L3::allocate_label);

        } else if (in.string() == L3::tensor_str) {

            parsed_items.push_back(& L3::tensor_label);
        
        } else {
            // std::cerr << "wrong runtime function in actions for runtime function call!\n";
        }
        
        // std::cerr << "runtime str parsed_items.size() = " << parsed_items.size() << '\n';
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

            currentF->vars.insert(v);

            parsed_items.push_back(v);
            // std::cerr << "after push parsed_items.size() = " << parsed_items.size() << '\n';
        }
    };

    template <>
    struct action<variable_func_rule>
    {
        template <typename Input>
        static void apply(const Input &in, Program &p)
        {
            // std::cerr << "firing " << in.string() << '\n';
            Function * currentF = p.functions.back();
            if (IN_MAP(currentF->varName2ptr, in.string()))
            {

                currentF->arg_list.push_back(currentF->varName2ptr[in.string()]);
                // std::cerr << "after push parsed_items.size() = " << parsed_items.size() << '\n';
                return;
            }

            ItemVariable *v = new ItemVariable(in.string());
            v->itemtype = item_variable;
            v->name = in.string();
            currentF->varName2ptr[in.string()] = v;

            currentF->arg_list.push_back(v);
            // std::cerr << "after push parsed_items.size() = " << parsed_items.size() << '\n';
        }
    };

    template<> struct action < plus_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        ItemAop *aop = new ItemAop;

        aop->itemtype = item_aop;
        aop->aopType = AopType::plus;

        aop->op2 = parsed_items.back();
        parsed_items.pop_back();

        aop->op1 = parsed_items.back();
        parsed_items.pop_back();

        parsed_items.push_back(aop);
    }
    };

    template<> struct action < minus_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        // DEBUG_OUT << "parsed_items.size = " << parsed_items.size() << '\n';
        ItemAop *aop = new ItemAop;

        aop->itemtype = item_aop;
        aop->aopType = AopType::minus;

        aop->op2 = parsed_items.back();
        parsed_items.pop_back();

        aop->op1 = parsed_items.back();
        parsed_items.pop_back();

        parsed_items.push_back(aop);

        // DEBUG_OUT << "parsed_items.size = " << parsed_items.size() << '\n';
        // DEBUG_OUT << "aop->op1 " << aop->op1->to_string() << "\n";
        // DEBUG_OUT << "aop->op2 " << aop->op2->to_string() << "\n";
        // DEBUG_OUT << "aop = " << aop->to_string() << "\n";
    }
    };

    template<> struct action < times_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        ItemAop *aop = new ItemAop;

        aop->itemtype = item_aop;
        aop->aopType = AopType::times;

        aop->op2 = parsed_items.back();
        parsed_items.pop_back();

        aop->op1 = parsed_items.back();
        parsed_items.pop_back();

        parsed_items.push_back(aop);
    }
    };

    template<> struct action < bitand_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        ItemAop *aop = new ItemAop;

        aop->itemtype = item_aop;
        aop->aopType = AopType::bit_and;

        aop->op2 = parsed_items.back();
        parsed_items.pop_back();

        aop->op1 = parsed_items.back();
        parsed_items.pop_back();

        parsed_items.push_back(aop);
    }
    };

    template<> struct action < shift_left_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        // std::cerr << in.string()  << '\n';

        /* 
         * Create the aop.
         */ 
        auto i = new ItemAop;
        i->itemtype = item_aop;

        i->aopType = AopType::shift_left;

        i->op2 = parsed_items.back();
        parsed_items.pop_back();
        
        i->op1 = parsed_items.back();
        parsed_items.pop_back();

        /* 
         * put the new item in the parsed_items 
         */ 
        parsed_items.push_back(i);
    }
    };

    template<> struct action < shift_right_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
         // std::cerr << in.string()  << '\n';

        /* 
         * Create the aop.
         */ 
        auto i = new ItemAop;
        i->itemtype = item_aop;

        i->aopType = AopType::shift_right;

        i->op2 = parsed_items.back();
        parsed_items.pop_back();
        
        i->op1 = parsed_items.back();
        parsed_items.pop_back();

        /* 
         * put the new item in the parsed_items 
         */ 
        parsed_items.push_back(i);
    }
    };
    

    template<> struct action < cmp_lesseq_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        // std::cerr << "firing" << in.string()  << '\n';

        // std::cerr << "before pop parsed_items.size() = " << parsed_items.size() << '\n';
        /* 
         * Create the cmp item.
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
         * Create the cmp item.
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

    template<> struct action < cmp_greateq_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        // std::cerr << "firing" << in.string()  << '\n';

        // std::cerr << "before pop parsed_items.size() = " << parsed_items.size() << '\n';
        /* 
         * Create the cmp item.
         */ 
        ItemCmp * itCmp = new ItemCmp();
        itCmp->itemtype = item_cmp;
        itCmp->cmptype = CmpType::geq;

        itCmp->op2 = parsed_items.back();
        parsed_items.pop_back();

        itCmp->op1 = parsed_items.back();
        parsed_items.pop_back();
        
        parsed_items.push_back(itCmp);

        // std::cerr << "after push parsed_items.size() = " << parsed_items.size() << '\n';
    }
    };

    template<> struct action < cmp_great_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        // std::cerr << in.string()  << '\n';

       /* 
         * Create the cmp item.
         */ 
        ItemCmp * itCmp = new ItemCmp();
        itCmp->itemtype = item_cmp;
        itCmp->cmptype = CmpType::great;
        
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
         * Create the cmp item.
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

    template<> struct action < load_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        // std::cerr << in.string()  << " in cmp_eq_rule" <<  '\n';

        ItemLoad *load = new ItemLoad(parsed_items.back());
        parsed_items.pop_back();

        parsed_items.push_back(load);
    }
    };
    
    template<> struct action < store_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p) {
        ItemStore * store = new ItemStore(
            parsed_items.back()
        );

        parsed_items.pop_back();
        parsed_items.push_back(store);
    }};


    template<> struct action < Instruction_return_void_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        // std::cerr << in.string()  << '\n';
        /* 
         *  call callee ( args )
         */ 
        auto currentF = p.functions.back();

        Instruction_ret * ret = new Instruction_ret;
        ret->type = InstType::inst_ret;
        currentF->instructions.push_back(ret);

    }
    };

    template<> struct action < Instruction_return_value_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        // std::cerr << in.string()  << '\n';
        /* 
         *  call callee ( args )
         */ 
        auto currentF = p.functions.back();

        Instruction_ret_var * ret = new Instruction_ret_var;
        ret->type = InstType::inst_ret_var;
        ret->valueToReturn = parsed_items.back();
        parsed_items.pop_back();

        currentF->instructions.push_back(ret);

    }
    };


    template<> struct action < Instruction_call_void_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        // std::cerr << in.string()  << '\n';
        /* 
         *  call callee ( args )
         */ 
        auto currentF = p.functions.back();

        Instruction_call * call = new Instruction_call;
        call->type = InstType::inst_call;

        Item * callee = parsed_items.front();
        parsed_items.pop_front();
        
        bool isRuntime = L3::isRuntimeLabel(callee);

        std::vector<Item *> args (
            parsed_items.begin(),
            parsed_items.end()
        );

        parsed_items.clear();

        call->call_wrap = new ItemCall(
            isRuntime,
            callee,
            args
        );

        call->ret = NULL;

        currentF->instructions.push_back(call);

    }
    };

    template<> struct action < Instruction_call_withret_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        // std::cerr << in.string()  << '\n';
        /* 
         *  var <- call callee ( args )
         */ 
        auto currentF = p.functions.back();

        Instruction_call * call = new Instruction_call;
        call->type = InstType::inst_call;

        call->ret = parsed_items.front();
        parsed_items.pop_front();
        
        Item * callee = parsed_items.front();
        parsed_items.pop_front();
        
        bool isRuntime = L3::isRuntimeLabel(callee);

        std::vector<Item *> args (
            parsed_items.begin(),
            parsed_items.end()
        );

        parsed_items.clear();

        call->call_wrap = new ItemCall(
            isRuntime,
            callee,
            args
        );

        

        currentF->instructions.push_back(call);

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

        currentF->Instlabels.insert(il->item_label);
        currentF->instructions.push_back(il);
    }
    };


    

    template<> struct action < Instruction_assignment_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        DEBUG_OUT << "firing " << in.string()  << " in Instruction_assignment_rule" << '\n';
        
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
    struct action<Instruction_unconditional_branch_rule>
    {
    template <typename Input>
    static void apply(const Input &in, Program &p)
    {
        DEBUG_OUT << "firing " << in.string()  << " in Instruction_unconditional_branch_rule" << '\n';
        /* 
         * Fetch the current function.
         */
        auto currentF = p.functions.back();

        /* 
         * Create the instruction.
         */ 
        Item *dst = parsed_items.back();
        parsed_items.pop_back();

    
        auto i = new Instruction_branch(dst);
    

        /* 
         * Add the just-created instruction to the current function.
         */ 
        currentF->instructions.push_back(i);
    }
    };

    template <>
    struct action<Instruction_conditional_branch_rule>
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
        Item *dst = parsed_items.back();
        parsed_items.pop_back();

        Item *condition = parsed_items.back();
        parsed_items.pop_back();

        auto i = new Instruction_branch(dst, condition);

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

        if (p.mainF == NULL){
            std::cerr << "missing main function" << '\n';
            assert(0);
        }


        return p;
    }


}
