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
#include <stack>

#include <tao/pegtl.hpp>
#include <tao/pegtl/analyze.hpp>
#include <tao/pegtl/contrib/raw_string.hpp>

#include "parser.h"


// #define PARSER_DEBUG

#ifdef PARSER_DEBUG
#define DEBUG_OUT (std::cerr << "DEBUG-LB-parser: ") // or any other ostream
#else
#define DEBUG_OUT 0 && std::cerr
#endif


namespace pegtl = tao::TAO_PEGTL_NAMESPACE;

using namespace pegtl;
using namespace std;

namespace LB {

    /* 
     * Data required to parse
     */ 
    std::deque<Item *> parsed_items;
    // std::map<std::string, Function *> name2Fptr;
    std::map<std::string, ItemFName *> name2FNameItem;
    std::stack<Instruction_scope *> scopeStack;

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
    
    /**
     *  N in LB
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

    /* 
     * Keywords.
     */

    struct str_arrow : TAOCPP_PEGTL_STRING( "<-" ) {};
        /**
         * useless str_right_arrow to appease crazy vscode highlighting
         * */
    struct str_right_arrow : TAOCPP_PEGTL_STRING( "->" ) {};

    struct str_goto: TAOCPP_PEGTL_STRING( "goto" ) {};
    struct str_return : TAOCPP_PEGTL_STRING( "return" ) {};
    struct str_if: TAOCPP_PEGTL_STRING( "if" ) {};
    struct str_while: TAOCPP_PEGTL_STRING( "while" ) {};
    struct str_continue: TAOCPP_PEGTL_STRING( "continue" ) {};
    struct str_break: TAOCPP_PEGTL_STRING( "break" ) {};

    
    struct str_call : TAOCPP_PEGTL_STRING( "call" ) {};
    struct str_print : TAOCPP_PEGTL_STRING( "print" ) {};
    struct str_input : TAOCPP_PEGTL_STRING( "input" ) {};
    struct str_tensor_error: TAOCPP_PEGTL_STRING( "tensor-error" ) {};
    
    struct str_define: TAOCPP_PEGTL_STRING( "define" ) {};
    struct str_length: TAOCPP_PEGTL_STRING( "length" ) {};
    
    struct str_new: TAOCPP_PEGTL_STRING( "new" ) {};
    struct str_Tuple: TAOCPP_PEGTL_STRING( "Tuple" ) {};
    struct str_Array: TAOCPP_PEGTL_STRING( "Array" ) {};

    struct str_type_int64: TAOCPP_PEGTL_STRING( "int64" ) {};
    struct str_closed_brack: TAOCPP_PEGTL_STRING( "[]" ) {};
    struct str_type_tuple: TAOCPP_PEGTL_STRING( "tuple" ) {};
    struct str_type_code: TAOCPP_PEGTL_STRING( "code" ) {};
    struct str_type_void: TAOCPP_PEGTL_STRING( "void" ) {};

    struct str_left_brack: TAOCPP_PEGTL_STRING( "(" ) {};
    struct str_right_brack: TAOCPP_PEGTL_STRING( ")" ) {};

    struct str_left_curly_brack: TAOCPP_PEGTL_STRING( "{" ) {};
    struct str_right_curly_brack: TAOCPP_PEGTL_STRING( "}" ) {};

    //  Operators str
    struct str_plus : TAOCPP_PEGTL_STRING( "+" ) {};
    struct str_minus : TAOCPP_PEGTL_STRING( "-" ) {};
    struct str_mult : TAOCPP_PEGTL_STRING( "*" ) {};
    struct str_bitand : TAOCPP_PEGTL_STRING( "&" ) {};
    struct str_shift_left : TAOCPP_PEGTL_STRING( "<<" ) {};
    struct str_shift_right : TAOCPP_PEGTL_STRING( ">>" ) {};
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
        name {};
    // struct variable:
    // pegtl::seq<
    //     pegtl::one<'%'>,
    //     name
    // > {};

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

   

    struct type_int64_rule: str_type_int64 {};   /* <- fire to generate var type*/
    struct type_tuple_rule: str_type_tuple {};   /* <- fire to generate var type*/
    struct type_code_rule: str_type_code {};      /* <- fire to generate var type*/
    struct type_void_rule: str_type_void {};      /* <- fire to generate var type*/

     /* int64([])* */
    struct type_tensor_rule:   /* <- fire to generate var type*/
    pegtl::seq<
        str_type_int64,
        seps,
        pegtl::plus<
            str_closed_brack
        >
    > {};

    struct func_type_sig:
    pegtl::sor<
        pegtl::seq< pegtl::at<type_tensor_rule>       , type_tensor_rule    >,
        pegtl::seq< pegtl::at<type_int64_rule>        , type_int64_rule     >,
        pegtl::seq< pegtl::at<type_tuple_rule>        , type_tuple_rule     >, 
        pegtl::seq< pegtl::at<type_code_rule>         , type_code_rule      >,
        pegtl::seq< pegtl::at<type_void_rule>         , type_void_rule      >
    > {};

    struct var_type_sig:
    pegtl::sor<
        pegtl::seq< pegtl::at<type_tensor_rule>       , type_tensor_rule    >,
        pegtl::seq< pegtl::at<type_int64_rule>        , type_int64_rule     >,
        pegtl::seq< pegtl::at<type_tuple_rule>        , type_tuple_rule     >, 
        pegtl::seq< pegtl::at<type_code_rule>         , type_code_rule      >
    > {};
    
    struct variable_rule:
        variable {};

    struct variable_func_rule:      /* Fire to generate arguments for function. Need to take prev typeSig*/
        variable {};

    struct variable_src_rule:
        variable {};

    struct func_variables_rule:
    /**
     * match zero time for no argument
     * match one time for one or more arguments
     * */
    pegtl::opt<
        pegtl::seq<
            seps,
            var_type_sig,
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
                    var_type_sig,
                    seps,
                    variable_func_rule,
                    seps
                >
            >
        >   
    >
     {};


    

    struct function_name:
    name {};

    struct constant_number:
    number {} ;
    

    /**
     *  t ::= var | N in LB
     * */
    struct variable_or_number_rule:
    pegtl::sor<
        variable_rule,
        constant_number
    > {};
    

    struct Label_rule:
    label {};

    /**
     *  u ::= var | label in LB
     * */
    struct variable_or_label_rule:
    pegtl::sor<
        variable_rule,
        Label_rule
    > {};

    /**
     *  s ::= t | label in LB
     *  source only
     *      e.g. name <- s
     *          name[idx] <- s 
     * */
    struct variable_or_label_or_number_rule:
    pegtl::sor<
        // variable_rule,
        variable_src_rule,
        Label_rule,
        constant_number
        
    > {};


    /**
     *  args ::= | t | t (, t)* in LB
     *  0 arg or more
     * */
    struct arguments_rule_optional:
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

    struct arguments_rule_atLeastOne:
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
    {};
    

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


    /**
     *  Instruction rule from now on
     * */
    
    struct  Instruction_label_rule:
    pegtl::seq<
        Label_rule
    > {};

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
    
    struct Instruction_goto_rule: 
    // conditionally jump to a label
    pegtl::seq<
        str_goto,
        seps,
        Label_rule
    > {};


    struct variable_declare_rule:
    variable_rule {};

    struct varDeclare_rule_atLeastOne:
    pegtl::seq<
        seps,
        variable_declare_rule,
        seps,
        /**
         *  star matches zero or more times
         * */
        pegtl::star<
            pegtl::seq<
                seps,
                pegtl::one<','>,
                seps,
                variable_declare_rule,
                seps
            >
        >
    >
    {};

    struct Instruction_declare_rule:
    pegtl::seq<
        var_type_sig,
        seps,
        varDeclare_rule_atLeastOne
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


    struct Instruction_if_rule: 
    // conditionally jump to a label
    pegtl::seq<
        str_if,
        seps,
        str_left_brack,
        seps,
        cmp_rule,
        seps,
        str_right_brack,
        seps,
        Label_rule,
        seps,
        Label_rule
    > {};

    struct Instruction_while_rule: 
    // conditionally jump to a label
    pegtl::seq<
        str_while,
        seps,
        str_left_brack,
        seps,
        cmp_rule,
        seps,
        str_right_brack,
        seps,
        Label_rule,
        seps,
        Label_rule
    > {};

    struct Instruction_continue_rule: 
    // conditionally jump to a label
    pegtl::seq<
        str_continue
    > {};

    struct Instruction_break_rule: 
    // conditionally jump to a label
    pegtl::seq<
        str_break
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


    struct array_access :
    pegtl::seq<
        variable_rule,
        seps,
        /**
         *  plus: match one or more times
         * */
        pegtl::plus <
            seps,
            pegtl::one <'['>,
            seps,
            variable_or_number_rule,
            seps,
            pegtl::one <']'>,
            seps
        >
    > {};

    /* var <- var([t])+ */
    struct var_from_array_access  :          /* fire to generate array access */
    pegtl::seq<
        variable_rule,
        seps,
        str_arrow,
        seps,
        array_access
    > {};

    /* var([t])+ <- s */
    struct array_access_from_allSrc :         /* fire to generate array access */
    pegtl::seq<
        array_access,
        seps,
        str_arrow,
        seps,
        variable_or_label_or_number_rule
    > {};
    
    /* var <- length var t */
    struct length_query :                   /* fire to generate length */
    pegtl::seq<
        str_length,
        seps,
        variable_rule,
        seps,
        variable_or_number_rule,
        seps
    > {};

    struct var_from_length:         
    pegtl::seq<
        variable_rule,
        seps,
        str_arrow,
        seps,
        length_query
    > {};

    struct new_array :
    pegtl::seq<
        str_new,
        seps,
        str_Array,
        seps,
        pegtl::one <'('>,
        seps,
        arguments_rule_atLeastOne,
        seps,
        pegtl::one <')'>,
        seps
    > {};

    struct new_tuple :
    pegtl::seq<
        str_new,
        seps,
        str_Tuple,
        seps,
        pegtl::one <'('>,
        seps,
        variable_or_number_rule,
        seps,
        pegtl::one <')'>,
        seps
    > {};

    /* var <- new Array(args) */
    struct var_from_new_array:          /* fire to generate newArray item */  
    pegtl::seq<
        variable_rule,
        seps,
        str_arrow,
        seps,
        new_array
    > {};

    /* var <- new Tuple(t) */
    struct var_from_new_tuple:          /* fire to generate newTuple item */  
    pegtl::seq<
        variable_rule,
        seps,
        str_arrow,
        seps,
        new_tuple
    > {};

    struct Instruction_assignment_rule:
    pegtl::sor<
        pegtl::seq< pegtl::at<var_from_aop>                 , var_from_aop                  >,
        pegtl::seq< pegtl::at<var_from_cmp>                 , var_from_cmp                  >,
        pegtl::seq< pegtl::at<var_from_array_access>        , var_from_array_access         >,
        pegtl::seq< pegtl::at<array_access_from_allSrc>     , array_access_from_allSrc      >,
        pegtl::seq< pegtl::at<var_from_length>              , var_from_length               >,
        pegtl::seq< pegtl::at<var_from_new_array>           , var_from_new_array            >,
        pegtl::seq< pegtl::at<var_from_new_tuple>           , var_from_new_tuple            >,
        pegtl::seq< pegtl::at<var_from_allSrc>              , var_from_allSrc               >
    > {};
    
          

    struct callee_rule :        /* fire to identify callee is a var or FName */
        name {};

    struct  call_wrap_rule:
    pegtl::seq< 
        seps,
        callee_rule,
        seps,
        pegtl::one< '(' >,
        seps,
        arguments_rule_optional, // <- fire var/number push var/number onto the stack
        seps,
        pegtl::one< ')' >,
        seps   
    > {};  

    struct  Instruction_call_void_rule:  // <- fire! create new instructions and call wrapper
    pegtl::seq<
        call_wrap_rule
    > {}; 

    struct  Instruction_call_withret_rule: // <- fire! create new instructions and call wrapper
    pegtl::seq<
        variable_rule,
        seps,
        str_arrow,
        seps,
        call_wrap_rule
    > {}; 

    struct Instruction_scope_rule;

    struct Instruction_rule:
    pegtl::sor<
        pegtl::seq< pegtl::at<Instruction_label_rule>                   , Instruction_label_rule                >,
        
        pegtl::seq< pegtl::at<Instruction_call_void_rule>               , Instruction_call_void_rule            >,
        pegtl::seq< pegtl::at<Instruction_call_withret_rule>            , Instruction_call_withret_rule         >,
        pegtl::seq< pegtl::at<Instruction_assignment_rule>              , Instruction_assignment_rule           >,
        pegtl::seq< pegtl::at<Instruction_declare_rule>                 , Instruction_declare_rule              >,

        pegtl::seq< pegtl::at<Instruction_return_rule>                  , Instruction_return_rule               >,
        pegtl::seq< pegtl::at<Instruction_while_rule>                   , Instruction_while_rule   >,    
        pegtl::seq< pegtl::at<Instruction_if_rule>                      , Instruction_if_rule >,
        pegtl::seq< pegtl::at<Instruction_continue_rule>                , Instruction_continue_rule >,
        pegtl::seq< pegtl::at<Instruction_break_rule>                   , Instruction_break_rule >,
        pegtl::seq< pegtl::at<Instruction_goto_rule>                    , Instruction_goto_rule >,
        pegtl::seq< pegtl::at<Instruction_scope_rule>                   , Instruction_scope_rule >
    > { };

    struct Instructions_rule:
    pegtl::star<
        pegtl::seq<
        seps,
        Instruction_rule,
        seps
        >
    > { };


    struct Instruction_scope_rule :
    pegtl::seq<
        seps,
        str_left_curly_brack,
        seps,
        Instructions_rule,
        seps,
        str_right_curly_brack
    > { };


    struct Function_rule:
    pegtl::seq<
        seps,
        func_type_sig,
        seps,
        function_name,
        seps,
        pegtl::one< '(' >,
        seps,
        func_variables_rule,
        seps,
        pegtl::one< ')' >,
        seps,
        Instruction_scope_rule,
        seps
    > {};

    struct Functions_rule:
    pegtl::plus<
        seps,
        Function_rule,
        seps
    > {};

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

        std::string inStr = in.string();

        auto newF = new Function();
        p.functions.push_back(newF);

        if (inStr == "main") {
            p.mainF = newF;
        }

        ItemFName * fname;
        if (IN_MAP(name2FNameItem, inStr)) {
            /* get the fname */
            fname = name2FNameItem[inStr];
        }
        else {
            /* create new fname and record it in the dictrionary */
            fname = new ItemFName(inStr);
            name2FNameItem[inStr] = fname;
        }

        newF->name = fname;
        fname->fptr = newF;

        Item * typeSig = parsed_items.back();
        parsed_items.pop_back();
        assert(typeSig->itemtype == ItemType::item_type_sig);

        newF->retType = (ItemTypeSig *) typeSig;

        assert(scopeStack.empty());


    }
    };

    template<> struct action < str_left_curly_brack > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        

        Instruction_scope * newScope;
        if (scopeStack.empty()) {
            /**
             *  This must be the beginning of a function
             * */ 

            Function * currF = p.functions.back();
            newScope = new Instruction_scope(NULL);
            currF->scope = newScope;

        } else {
            Instruction_scope * oldScope = scopeStack.top();
            newScope = new Instruction_scope(oldScope);
            oldScope->appendInst(newScope);
            
        }
        
        scopeStack.push(newScope);
        
    }

    };

    template<> struct action < str_right_curly_brack > {
    template< typename Input >
    static void apply( const Input & in, Program & p){

        assert(!scopeStack.empty());
        scopeStack.pop();

    }

    };


    template<> struct action < constant_number > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        // std::cerr << "pushing " << in.string () << '\n';

        // auto currentF = p.functions.back();
        ItemConstant *itConst = new ItemConstant(
            std::stoll(in.string())
        );
        // std::cerr << "   val = " << itConst->constVal << '\n';
        
        parsed_items.push_back(itConst);
    
        // Function * currF = p.functions.back();
        // currF->constToEncode.insert(itConst);
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
        currentF->labelName2ptr[in.string()] = l;
        // currentF->labels.insert(l);
        
        parsed_items.push_back(l);

        // std::cerr << "after push parsed_items.size() = " << parsed_items.size() << '\n';
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

            /**
             *  this is not the variable rule for declaration
             *  must be in map
             * */

            assert (IN_MAP(currentF->varName2ptr, in.string()));
            
            Item * v = currentF->varName2ptr[in.string()];
            parsed_items.push_back(v);

        }
    };

    template <>
    struct action<variable_src_rule>
    {
        template <typename Input>
        static void apply(const Input &in, Program &p)
        {
            // std::cerr << "firing " << in.string() << '\n';
            Function *currentF = p.functions.back();

            /**
             *  This is a name from in src, so might also be a function name
             * */
            std::string inStr = in.string();
            if(IN_MAP(currentF->varName2ptr, inStr)){
                Item * v = currentF->varName2ptr[inStr];
                parsed_items.push_back(v);
            } else {
                ItemFName * fname;
                if (IN_MAP(name2FNameItem, inStr)) {
                    /* get the fname */
                    fname = name2FNameItem[inStr];
                }
                else {
                    /* create new fname and record it in the dictrionary */
                    fname = new ItemFName(inStr);
                    name2FNameItem[inStr] = fname;
                }
                parsed_items.push_back(fname);
            } 

        }
    };


    template <>
    struct action<variable_func_rule>
    {
        template <typename Input>
        static void apply(const Input &in, Program &p)
        {   
            /**
             *  Expect one typeSig in the parsed_items
             * */   
            assert(parsed_items.size() == 1);
            // std::cerr << "firing " << in.string() << '\n';
            Function * currentF = p.functions.back();

            /**
             *  declaration of var so should NOT be in map
             * */
            assert(!IN_MAP(currentF->varName2ptr, in.string()));

            Item * typeSig = parsed_items.back();
            parsed_items.pop_back();
            ItemVariable * v = new ItemVariable(
                in.string(),
                typeSig    
            );
            currentF->varName2ptr[in.string()] = v;
            currentF->vars.insert(v);

            currentF->arg_list.push_back(v);
        }
    };

    template <>
    struct action<variable_declare_rule>
    {
        template <typename Input>
        static void apply(const Input &in, Program &p)
        {   
            /**
             *  Expect one typeSig in the parsed_items
             * */   
            assert(parsed_items.size() >= 1);
            // std::cerr << "firing " << in.string() << '\n';
            Function * currentF = p.functions.back();
            std::string inStr = in.string();

            /**
             *  declaration of var but
             *      but can be in the map because of scopes
             * */
            // assert(!IN_MAP(currentF->varName2ptr, in.string()));
            Item * typeSig = parsed_items.front();


            if(IN_MAP(currentF->varName2ptr, inStr)){
                Item * v = currentF->varName2ptr[inStr];
                

                /**
                 *  right now we assume that typesig of variable never changes ??
                 * */
                assert(v->itemtype == ItemType::item_variable);
                ItemVariable * var = (ItemVariable *) v;
                assert(typeSig == var->typeSig);
                
                parsed_items.push_back(v);



            } else {
                ItemVariable * v = new ItemVariable(
                    in.string(),
                    typeSig    
                );
                
                currentF->varName2ptr[inStr] = v;
                currentF->vars.insert(v);

                parsed_items.push_back(v);
            }
            
        }
    };

    template <>
    struct action<callee_rule>
    {
        template <typename Input>
        static void apply(const Input &in, Program &p)
        {   
             
            // std::cerr << "firing " << in.string() << '\n';
            Function * currentF = p.functions.back();

            std::string inStr = in.string();
            
            if (IN_MAP(currentF->varName2ptr, inStr)) {
                /**
                 * This is a var
                 */
                Item * var = currentF->varName2ptr[inStr];
                parsed_items.push_back(var);
            }
            else {

                if (inStr == LB::print_str) {
            
                    parsed_items.push_back(& LB::print_FName);

                } else if (inStr == LB::input_str) {

                    parsed_items.push_back(& LB::input_FName);

                } else {
                    // std::cerr << "wrong runtime function in actions for runtime function call!\n";
               

                    /* This is a function name */

                    if (IN_MAP(name2FNameItem, inStr)) {
                        /* get the fname */
                        Item * fname = name2FNameItem[inStr];
                        parsed_items.push_back(fname);
                    }
                    else {
                        /* create new fname and record it in the dictrionary */
                        ItemFName * fname = new ItemFName(inStr);

                        name2FNameItem[inStr] = fname;
                        parsed_items.push_back(fname);
                    }

                }   
            }

        }
    };

    
    template<> struct action < type_tensor_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        // DEBUG_OUT << "firing type_tensor_rule\n";
        
        /**
         *  expect int64[][][]
         *      ndims = [][]
         * */
        int32_t minlen = 7;
        int32_t int64_str_len = 5;
        assert(in.string().length() >= minlen);

        int32_t ndims = (in.string().length() - int64_str_len) / 2;

        ItemTypeSig * typesig = new ItemTypeSig(
            VarType::tensor,
            ndims
        );
        
        parsed_items.push_back(typesig);
    }
    };

    template<> struct action < type_int64_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        /**
         *  expect int64
         * */
        
        assert(in.string() == "int64");
        
        parsed_items.push_back( &LB::int64Sig);
    }
    };

    template<> struct action < type_code_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        /**
         *  expect code
         * */
        
        assert(in.string() == "code");
        parsed_items.push_back( &LB::codeSig);
    }
    };

    template<> struct action < type_tuple_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
         DEBUG_OUT << "firing type_tuple_rule\n";
        /**
         *  expect tuple
         * */
        
        assert(in.string() == "tuple");
        parsed_items.push_back( &LB::tupleSig);
    }
    };


    template<> struct action < type_void_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        /**
         *  expect void
         * */
        
        assert(in.string() == "void");
        ItemTypeSig * typesig = new ItemTypeSig(
            VarType::void_type
        );
        
        parsed_items.push_back(typesig);
    }
    };



    template<> struct action < plus_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        

        Item * op2 = parsed_items.back();
        parsed_items.pop_back();

        Item *op1 = parsed_items.back();
        parsed_items.pop_back();

        ItemOp * op = new ItemOp (
            op1,
            op2,
            OpType::plus
        );

        parsed_items.push_back(op);
    }
    };

    template<> struct action < minus_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        // DEBUG_OUT << "parsed_items.size = " << parsed_items.size() << '\n';
        Item * op2 = parsed_items.back();
        parsed_items.pop_back();

        Item *op1 = parsed_items.back();
        parsed_items.pop_back();

        ItemOp * op = new ItemOp (
            op1,
            op2,
            OpType::minus
        );

        parsed_items.push_back(op);

        // DEBUG_OUT << "parsed_items.size = " << parsed_items.size() << '\n';
        // DEBUG_OUT << "aop->op1 " << aop->op1->to_string() << "\n";
        // DEBUG_OUT << "aop->op2 " << aop->op2->to_string() << "\n";
        // DEBUG_OUT << "aop = " << aop->to_string() << "\n";
    }
    };

    template<> struct action < times_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        Item * op2 = parsed_items.back();
        parsed_items.pop_back();

        Item *op1 = parsed_items.back();
        parsed_items.pop_back();

        ItemOp * op = new ItemOp (
            op1,
            op2,
            OpType::times
        );

        parsed_items.push_back(op);
    }
    };

    template<> struct action < bitand_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        Item * op2 = parsed_items.back();
        parsed_items.pop_back();

        Item *op1 = parsed_items.back();
        parsed_items.pop_back();

        ItemOp * op = new ItemOp (
            op1,
            op2,
            OpType::bit_and
        );

        parsed_items.push_back(op);
    }
    };

    template<> struct action < shift_left_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        // std::cerr << in.string()  << '\n';

        Item * op2 = parsed_items.back();
        parsed_items.pop_back();

        Item *op1 = parsed_items.back();
        parsed_items.pop_back();

        ItemOp * op = new ItemOp (
            op1,
            op2,
            OpType::shift_left
        );

        parsed_items.push_back(op);
    }
    };

    template<> struct action < shift_right_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
         // std::cerr << in.string()  << '\n';

        Item * op2 = parsed_items.back();
        parsed_items.pop_back();

        Item *op1 = parsed_items.back();
        parsed_items.pop_back();

        ItemOp * op = new ItemOp (
            op1,
            op2,
            OpType::shift_right
        );

        parsed_items.push_back(op);
    }
    };
    

    template<> struct action < cmp_lesseq_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        // std::cerr << "firing" << in.string()  << '\n';

        Item * op2 = parsed_items.back();
        parsed_items.pop_back();

        Item *op1 = parsed_items.back();
        parsed_items.pop_back();

        ItemOp * op = new ItemOp (
            op1,
            op2,
            OpType::leq
        );

        parsed_items.push_back(op);
    }
    };

    template<> struct action < cmp_less_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        // std::cerr << in.string()  << '\n';

        Item * op2 = parsed_items.back();
        parsed_items.pop_back();

        Item *op1 = parsed_items.back();
        parsed_items.pop_back();

        ItemOp * op = new ItemOp (
            op1,
            op2,
            OpType::less
        );

        parsed_items.push_back(op);
    }
    };

    template<> struct action < cmp_greateq_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        // std::cerr << "firing" << in.string()  << '\n';

        Item * op2 = parsed_items.back();
        parsed_items.pop_back();

        Item *op1 = parsed_items.back();
        parsed_items.pop_back();

        ItemOp * op = new ItemOp (
            op1,
            op2,
            OpType::geq
        );

        parsed_items.push_back(op);
    }
    };

    template<> struct action < cmp_great_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        // std::cerr << in.string()  << '\n';

        Item * op2 = parsed_items.back();
        parsed_items.pop_back();

        Item *op1 = parsed_items.back();
        parsed_items.pop_back();

        ItemOp * op = new ItemOp (
            op1,
            op2,
            OpType::great
        );

        parsed_items.push_back(op);
    }
    };
    template<> struct action < cmp_eq_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        // std::cerr << in.string()  << " in cmp_eq_rule" <<  '\n';

        Item * op2 = parsed_items.back();
        parsed_items.pop_back();

        Item *op1 = parsed_items.back();
        parsed_items.pop_back();

        ItemOp * op = new ItemOp (
            op1,
            op2,
            OpType::eq
        );

        parsed_items.push_back(op);
    }
    };

    template<> struct action < var_from_array_access > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        
        /* 
         * build array access element for Instruction assignment
         *  var <- var([t])+
         *  
         *  evreything but the first two vars are ([t])+ 
         */ 
        Function * currF = p.functions.back();

        std::vector<Item *> offsets;

        while (parsed_items.size() > 2)
        { 
            Item * offset = parsed_items.back();
            offsets.push_back(offset);
            parsed_items.pop_back();

            /**
            *  If offset is a constant, we don't want it in our set of constants to encode
            * */
            // if (offset->itemtype == ItemType::item_constant) {
            //     currF->constToEncode.erase((ItemConstant *) offset);
            // }
        }
        
        std::reverse(offsets.begin(), offsets.end());

        Item * addr = parsed_items.back();
        parsed_items.pop_back();


        int64_t lineN = in.position().line;
        ItemArrAccess * i = new ItemArrAccess (
            addr, 
            offsets,
            lineN
        );
        /* 
         * Add the just-created instruction to the current function.
         */ 
        parsed_items.push_back(i);
    }
    };

    template<> struct 
    action < array_access_from_allSrc > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        
        /* 
         * build array access element for Instruction assignment
         *  var([t])+ <- s
         *  
         */ 
        Function * currF = p.functions.back();
        
        std::vector<Item *> offsets;
        Item * addr;

        /**
         * starts with the first one
         * */
        addr = parsed_items.front();
        parsed_items.pop_front();

        /**
         *  all elements except last are offsets
         * */
        // offsets = std::vector<Item *> (parsed_items.begin(), parsed_items.end() - 1)
        while(parsed_items.size() > 1) {
            Item *offset = parsed_items.front();
            offsets.push_back(offset);
            parsed_items.pop_front();

            /**
            *  If offset is a constant, we don't want it in our set of constants to encode
            * */
            // if (offset->itemtype == ItemType::item_constant) {
            //     currF->constToEncode.erase((ItemConstant *) offset);
            // }
        }
        
        int64_t lineN = in.position().line;
        ItemArrAccess * i = new ItemArrAccess (
            addr, 
            offsets,
            lineN
        );
        
        parsed_items.push_front(i);
    }
    };

    template<> struct action < var_from_new_array > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        
        /* 
         * build array access element for Instruction assignment
         *  var <- new Array(args)
         *  
         */ 
        
        std::vector<Item *> args;

        /**
         *  all elements are args except the first one
         * */

        while(parsed_items.size() > 1) {
            Item * arg = parsed_items.back();
            parsed_items.pop_back();

            args.push_back(arg);
        }
        
        std::reverse(args.begin(), args.end());

        ItemNewArray * arr = new ItemNewArray (
            args
        );
        
        parsed_items.push_back(arr);
    }
    };

    template<> struct action < var_from_new_tuple > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        
        /* 
         * build newTuple element for Instruction assignment
         *  var <- new Tuple(t)
         *  
         */ 
        
        Item * len = parsed_items.back();
        parsed_items.pop_back();

        ItemNewTuple * arr = new ItemNewTuple (
            len
        );
        
        parsed_items.push_back(arr);
    }
    };


    template<> struct action < length_query > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        
        /* 
         * build array access element for Instruction assignment
         *  length var t
         *  
         */ 
        Function * currF = p.functions.back();

        Item * dim = parsed_items.back();

        /**
         *  If dim is a constant, we don't want it in our set of constants to encode
         * */
        // if (dim->itemtype == ItemType::item_constant) {
        //     currF->constToEncode.erase((ItemConstant *) dim);
        // }

        parsed_items.pop_back();
        Item * addr = parsed_items.back();
        parsed_items.pop_back();
        
        ItemLength * len = new ItemLength (
            addr, 
            dim
        );
        
        parsed_items.push_back(len);
    }
    };

    template<> struct action < Instruction_return_void_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        // std::cerr << in.string()  << '\n';
        /* 
         *  call callee ( args )
         */ 
        assert(!scopeStack.empty());
        Instruction_scope * curScope = scopeStack.top();

        Instruction_ret * ret = new Instruction_ret(curScope);

        
        curScope->appendInst(ret);

    }
    };

    template<> struct action < Instruction_return_value_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        // std::cerr << in.string()  << '\n';
        assert(!scopeStack.empty());
        Instruction_scope * curScope = scopeStack.top();
        
        Item * valueToRet = parsed_items.back();
        parsed_items.pop_back();
        Instruction_ret_var * ret = new Instruction_ret_var(
            valueToRet,
            curScope
        );

        curScope->appendInst(ret);

    }
    };

    template <>
    struct action<Instruction_goto_rule>
    {
    template <typename Input>
    static void apply(const Input &in, Program &p)
    {
        // DEBUG_OUT << "firing " << in.string()  << " in Instruction_unconditional_branch_rule" << '\n';
        /* 
         * Fetch the current function.
         */
        assert(!scopeStack.empty());
        Instruction_scope * curScope = scopeStack.top();

        Item *dst = parsed_items.back();
        parsed_items.pop_back();
    
        Instruction_goto * gt = new Instruction_goto (dst, curScope);
    
        curScope->appendInst(gt);
    }
    };

    template <>
    struct action<Instruction_if_rule>
    {
    template <typename Input>
    static void apply(const Input &in, Program &p)
    {
        // std::cerr << in.string()  << "in Instruction_assignment_rule" << '\n';
        /* 
         * Fetch the current function.
         */
        assert(!scopeStack.empty());
        Instruction_scope * curScope = scopeStack.top();

        Item *dst2 = parsed_items.back();
        parsed_items.pop_back();
        
        Item *dst1 = parsed_items.back();
        parsed_items.pop_back();
        
        Item *cond = parsed_items.back();
        parsed_items.pop_back();

        Instruction_if * ifInst = new Instruction_if (
            dst1,       
            dst2,
            cond, 
            curScope
        );

        curScope->appendInst(ifInst);
    }
    };

     template <>
    struct action<Instruction_while_rule>
    {
    template <typename Input>
    static void apply(const Input &in, Program &p)
    {
        // std::cerr << in.string()  << "in Instruction_assignment_rule" << '\n';
        /* 
         * Fetch the current function.
         */
        assert(!scopeStack.empty());
        Instruction_scope * curScope = scopeStack.top();

        Item *dst2 = parsed_items.back();
        parsed_items.pop_back();
        
        Item *dst1 = parsed_items.back();
        parsed_items.pop_back();
        
        Item *cond = parsed_items.back();
        parsed_items.pop_back();

        Instruction_while * whileInst = new Instruction_while (
            dst1,       
            dst2,
            cond, 
            curScope
        );

        curScope->appendInst(whileInst);
    }
    };


    template<> struct action < Instruction_continue_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        // std::cerr << in.string()  << '\n';
        /* 
         *  call callee ( args )
         */ 
        assert(!scopeStack.empty());
        Instruction_scope * curScope = scopeStack.top();

        Instruction_continue * cont = new Instruction_continue(curScope);

        
        curScope->appendInst(cont);

    }
    };

    template<> struct action < Instruction_break_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        // std::cerr << in.string()  << '\n';
        /* 
         *  call callee ( args )
         */ 
        assert(!scopeStack.empty());
        Instruction_scope * curScope = scopeStack.top();

        Instruction_break * brk = new Instruction_break(curScope);

        
        curScope->appendInst(brk);

    }
    };


    template<> struct action < Instruction_declare_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        // std::cerr << in.string()  << '\n';
        /* 
         *  call callee ( args )
         */ 
        assert(!scopeStack.empty());
        Instruction_scope * curScope = scopeStack.top();
        

        Item * typesig = parsed_items.front();
        parsed_items.pop_front();
        
        // Item * var = parsed_items.back();
        // parsed_items.pop_back();
        
        std::vector<Item *> vars (
            parsed_items.begin(),
            parsed_items.end()
        );
        
        parsed_items.clear();

        // assert(var->itemtype == ItemType::item_variable);
        assert(typesig->itemtype == ItemType::item_type_sig);

        // DEBUG_OUT << typesig->to_string() << "\n";
        // DEBUG_OUT << var->to_string() << "\n";

        Instruction_declare * declare = new Instruction_declare (
            typesig,
            vars,
            curScope
        );

        curScope->appendInst(declare);
    }
    };

    template<> struct action < Instruction_call_void_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        // std::cerr << in.string()  << '\n';
        /* 
         *  call callee ( args )
         */ 
        assert(!scopeStack.empty());
        Instruction_scope * curScope = scopeStack.top();

        Item * callee = parsed_items.front();
        parsed_items.pop_front();
        
        bool isRuntime = LB::isRuntimeFName(callee);

        std::vector<Item *> args (
            parsed_items.begin(),
            parsed_items.end()
        );

        parsed_items.clear();

        ItemCall * call_wrap = new ItemCall(
            isRuntime,
            callee->itemtype == ItemType::item_fname,        /* calleeIsFName */
            callee,
            args
        );


        Instruction_call * call = new Instruction_call (
            call_wrap,
            curScope
        );

        curScope->appendInst(call);
    
    }
    };

    template<> struct action < Instruction_call_withret_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        // std::cerr << in.string()  << '\n';
        /* 
         *  var <- call callee ( args )
         */ 
        assert(!scopeStack.empty());
        Instruction_scope * curScope = scopeStack.top();

        Item * ret = parsed_items.front();
        parsed_items.pop_front();

        Item * callee = parsed_items.front();
        parsed_items.pop_front();
        
        bool isRuntime = LB::isRuntimeFName(callee);

        std::vector<Item *> args (
            parsed_items.begin(),
            parsed_items.end()
        );

        parsed_items.clear();

        ItemCall * call_wrap = new ItemCall(
            isRuntime,
            callee->itemtype == ItemType::item_fname,        /* calleeIsFName */
            callee,
            args
        );


        Instruction_assignment * call_assign = new Instruction_assignment (
            call_wrap,
            ret,
            curScope
        );

        curScope->appendInst(call_assign);

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
        assert(!scopeStack.empty());
        Instruction_scope * curScope = scopeStack.top();
        Function * currentF = p.functions.back();
        /* 
         * Create the instruction and add it to function
         */

        Item * label_item =  parsed_items.back();
        parsed_items.pop_back();
        assert(label_item->itemtype == ItemType::item_labels);

        Instruction_label * il = new Instruction_label(label_item, curScope);
        curScope->appendInst(il);

        /* 
         * have function keeps track of its labels
         */ 
        currentF->Instlabels.insert(il->item_label);
    }
    };


    
    

    template<> struct action < Instruction_assignment_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        // DEBUG_OUT << "firing " << in.string()  << " in Instruction_assignment_rule" << '\n';
        
        /* 
         * Fetch the current function.
         */ 
        assert(!scopeStack.empty());
        Instruction_scope * curScope = scopeStack.top();

        /* 
         * Create the instruction.
         */ 

        Item * src = parsed_items.back();
        parsed_items.pop_back();

        Item * dst = parsed_items.back();
        parsed_items.pop_back();

        // DEBUG_OUT << src->to_string() << "\n";
        // DEBUG_OUT << dst->to_string() << "\n";

        Instruction_assignment * i = new Instruction_assignment(
            src,
            dst,
            curScope
        );
        /* 
         * Add the just-created instruction to the current function.
         */ 
        curScope->appendInst(i);
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
