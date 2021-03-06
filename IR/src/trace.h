#pragma once

#include "IR.h"
#include <functional>

namespace IR {

    typedef std::function<
            BasicBlock * 
            (std::vector<BasicBlock *> & BBlist, bool entryTraced)
        > 
    FetchRemove_Heuristic_F;

    typedef std::function<
            BasicBlock * 
            (std::set<BasicBlock *> & successors, std::set<BasicBlock *> & tracedBB)
        >
    FindNextForTrace_Heuristic_F;

    class Trace {
        public:
        std::vector<BasicBlock *> jointBBs; 
    };

    

    class TraceGenerator {
        public:
            std::vector<Trace *> generateTrace(
                std::vector<BasicBlock *> BBs 
            );
            
            TraceGenerator(
                FetchRemove_Heuristic_F fetch_remove,
                FindNextForTrace_Heuristic_F findNextForTrace
            );

        private:
            FetchRemove_Heuristic_F fetch_remove;
            
            FindNextForTrace_Heuristic_F findNextForTrace;

    };

    
    std::vector<Trace *> runGenerateTrace(Function * F);

}