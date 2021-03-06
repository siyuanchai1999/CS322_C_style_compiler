#include "trace.h"

#ifdef CFG_DEBUG
#define DEBUG_OUT (std::cerr << "DEBUG-IR: ") // or any other ostream
#else
#define DEBUG_OUT 0 && std::cerr
#endif

namespace IR {

    BasicBlock * fetch_remove_UncondBrFirst(std::vector<BasicBlock *> & BBlist, bool entryTraced) {
        if (!entryTraced) {
            BasicBlock * ret =  BBlist[0];
            BBlist.erase(BBlist.begin());
            return ret;
        }


        auto it = BBlist.begin();
        for (; it != BBlist.end(); it++) {
            /**
             *  return BB with only one successor
             *      which has possibility of merging
             * */
            if((*it)->succs.size() == 1) {
                BasicBlock * ret = *it;
                BBlist.erase(it);
                return ret;
            }
        }

        BasicBlock * ret =  BBlist[0];
        BBlist.erase(BBlist.begin());
        return ret;
    }


    BasicBlock * findNextInTrace_UncondBrFirst(
        std::set<BasicBlock *> & successors,
        std::set<BasicBlock *> & tracedBB
    ) {
        std::set<BasicBlock *> succNotMarked;
        set_diff(
            successors,         /* src A*/
            tracedBB,           /* src A*/
            succNotMarked       /* dst*/
        );

        /**
         *  return BB with only one successor
         *      which has possibility of merging
         * */

        for (BasicBlock * nextBB : succNotMarked) {
            if(nextBB->succs.size() == 1) {
                return nextBB;
            }
        }

        if (succNotMarked.empty()) {
            return NULL;
        } else {
            return *succNotMarked.begin();  
        }
    }

    /* Note: this NOT pass by reference*/
    /**
     *  assume input BB put entry block at the first
     * */
    std::vector<Trace *> TraceGenerator::generateTrace(
        std::vector<BasicBlock *> BBs 

    ) {
        std::set<BasicBlock *> tracedBB;
        std::vector<Trace *> traces;
        bool entryTraced = false;
        int32_t it = 0;
        
        do
        {   
            Trace * tr = new Trace();
            BasicBlock * bb = this->fetch_remove(BBs, entryTraced);

            entryTraced = true;
            
            while (!IN_SET(tracedBB, bb))
            {
                tracedBB.insert(bb);        /* mark the BB*/
                tr->jointBBs.push_back(bb);

                BasicBlock * nextBB = this->findNextForTrace(
                    bb->succs,
                    tracedBB
                );  

                if (nextBB) {
                    bb = nextBB;
                }
            }
            
            if (tr->jointBBs.empty()) {
                delete tr;
            } else {
                traces.push_back(tr);
            }
            
            
        } while (!BBs.empty());
        
        return traces;
    }

    TraceGenerator::TraceGenerator(
            FetchRemove_Heuristic_F fetch_remove,
            FindNextForTrace_Heuristic_F findNextForTrace
    ) {
        this->fetch_remove = fetch_remove;
        this->findNextForTrace = findNextForTrace;
    }

    std::vector<Trace *> runGenerateTrace(Function * F) {
        TraceGenerator gen (
            fetch_remove_UncondBrFirst,
            findNextInTrace_UncondBrFirst
        );

        return gen.generateTrace(F->BasicBlocks);
    }

}