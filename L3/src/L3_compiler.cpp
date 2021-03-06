#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <set>
#include <iterator>
#include <iostream>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include <stdint.h>
#include <unistd.h>
#include <iostream>

// #include <L2.h>
#include "L3.h"
#include "parser.h"
#include "analysis.h"
#include "transformer.h"
#include "inst_selection.h"
#include "code_generator.h"
// #include <spiller.h>
// #include <register_allocation.h>
#include <utils.h>

using namespace std;

void print_help (char *progName){
    std::cerr << "Usage: " << progName << " [-v] [-g 0|1] [-O 0|1|2] [-s] [-l] [-i] SOURCE" << std::endl;
    return ;
}

int main(
    int argc, 
    char **argv
    ){
    
    auto enable_code_generator = true;
    int32_t optLevel = 0;
    bool verbose  = false;

    /* 
    * Check the compiler arguments.
    */
    if( argc < 2 ) {
        print_help(argv[0]);
        return 1;
    }
    
    // int32_t opt;
    // while ((opt = getopt(argc, argv, "vg:O:")) != -1) {
    //     switch (opt){
    //     case 'O':
    //         optLevel = strtoul(optarg, NULL, 0);
    //         break ;

    //     case 'g':
    //         enable_code_generator = (strtoul(optarg, NULL, 0) == 0) ? false : true ;
    //         break ;

    //     case 'v':
    //         verbose = true;
    //         break ;

    //     default:
    //         print_help(argv[0]);
    //         return 1;
    //     }
    // }

    /*
    * Parse the input file.
    */
    auto p = L3::parse_file(argv[optind]);

    
    std::vector<std::vector<L3::InstSelectForest *>> codeGenerator;

    L3::transform_label(p);
    L3::select_insts(p, codeGenerator);
    L3::generateCode(p, codeGenerator);

    // /* 
    // * Print the source program.
    // */
    if (verbose){

        // for (auto f : p.functions){
        //     f->print();
        // //TODO
        // }
    }

    // /*
    // * Generate x86_64 assembly.
    // */
    // if (enable_code_generator){
    //     L3::generate_code(p);
    // }

    return 0;
    
}
