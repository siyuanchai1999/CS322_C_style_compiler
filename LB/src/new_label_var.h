#pragma once

#include "LB.h"
#include "utils.h"
 
namespace LB { 

    void new_var_label_init(Program & p);


    ItemVariable * get_new_var();
    ItemLabel * get_new_label();

    void clean_vars();
    void clean_lables();

    class LabelVarGen
    {
        private:
            std::string varPrefix = "";
            int32_t varIdx = 0;
            std::string labelPrefix = "";
            int32_t labelIdx = 0;

            std::vector<ItemVariable *> vars;
            std::vector<ItemLabel *> labels;

        public:
            ItemVariable * get_new_var(VarType vtype);
            ItemLabel * get_new_label();

            void clean();
        
            LabelVarGen(Program &p);
            LabelVarGen();
    };
    

    extern LabelVarGen * GENLV;
    // extern LabelVarGen GENLV;
}